/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
*/

#include "NativeWebRTCTransport.h"

#include "NativeWebRTCProtocol.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <exception>
#include <map>
#include <mutex>
#include <set>
#include <thread>
#include <utility>
#include <variant>
#include <vector>

#include <curl/curl.h>
#include <rtc/rtc.hpp>

extern int __argc;
extern char **__argv;

// GeneralsX @feature Copilot 30/08/2026 Implement native macOS/Linux WebRTC
// signaling and unordered bounded-retransmit DataChannels using libdatachannel.
namespace GeneralsX::NativeWebRTC
{
namespace
{

constexpr std::size_t MAX_CONTROL_EVENTS = 4096;
constexpr std::size_t MAX_HTTP_RESPONSE_BYTES = 64 * 1024;
constexpr auto WEBSOCKET_CONNECT_TIMEOUT = std::chrono::seconds(10);
constexpr auto WEBSOCKET_PING_INTERVAL = std::chrono::seconds(20);

std::once_flag g_curlInitFlag;

struct HttpResponseBuffer
{
	std::string body;
	bool exceededLimit = false;
};

size_t CurlWriteCallback(char *data, size_t size, size_t count, void *userData)
{
	const std::size_t bytes = size * count;
	auto *buffer = static_cast<HttpResponseBuffer *>(userData);
	if (bytes > MAX_HTTP_RESPONSE_BYTES - std::min(buffer->body.size(), MAX_HTTP_RESPONSE_BYTES))
	{
		buffer->exceededLimit = true;
		return 0;
	}
	buffer->body.append(data, bytes);
	return bytes;
}

std::unordered_map<std::string, std::string> ReadProcessEnvironment()
{
	static constexpr const char *NAMES[] = {
		"GENERALSX_WEBRTC",
		"GENERALSX_WEBRTC_SIGNALING_URL",
		"GENERALSX_WEBRTC_ROOM",
		"GENERALSX_WEBRTC_PLAYER_NAME",
		"GENERALSX_WEBRTC_CAPACITY",
		"GENERALSX_WEBRTC_DISABLE_TURN",
		"GENERALSX_WEBRTC_FORCE_RELAY",
		"GENERALSX_WEBRTC_ICE_SERVERS_JSON",
	};

	std::unordered_map<std::string, std::string> environment;
	for (const char *name : NAMES)
	{
		if (const char *value = std::getenv(name); value != nullptr)
		{
			environment.emplace(name, value);
		}
	}
	return environment;
}

std::vector<std::string> ReadProcessArguments()
{
	std::vector<std::string> arguments;
	if (__argc <= 0 || __argv == nullptr)
	{
		return arguments;
	}
	arguments.reserve(static_cast<std::size_t>(__argc));
	for (int index = 0; index < __argc; ++index)
	{
		arguments.emplace_back(__argv[index] != nullptr ? __argv[index] : "");
	}
	return arguments;
}

void Log(const char *message)
{
	std::fprintf(stderr, "[NativeWebRTC] %s\n", message);
	std::fflush(stderr);
}

void LogError(const std::string &message)
{
	std::fprintf(stderr, "[NativeWebRTC] %s\n", message.c_str());
	std::fflush(stderr);
}

std::vector<IceServerSpec> FetchIceServerSpecs(const RuntimeConfig &config)
{
	std::string parseError;
	if (!config.iceServersJson.empty())
	{
		const auto parsed = ParseIceServersResponse(config.iceServersJson, false, &parseError);
		if (!parsed)
		{
			LogError("ignored malformed GENERALSX_WEBRTC_ICE_SERVERS_JSON: " + parseError);
			return {};
		}
		return *parsed;
	}
	if (!config.fetchTurnCredentials)
	{
		return {};
	}

	std::call_once(g_curlInitFlag, []() {
		curl_global_init(CURL_GLOBAL_DEFAULT);
	});
	CURL *curl = curl_easy_init();
	if (curl == nullptr)
	{
		Log("TURN unavailable because libcurl initialization failed; using direct ICE");
		return {};
	}

	HttpResponseBuffer response;
	const std::string url = BuildTurnCredentialsUrl(config);
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "GeneralsX/native-webrtc");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 8L);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

	const CURLcode result = curl_easy_perform(curl);
	long status = 0;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
	curl_easy_cleanup(curl);

	if (result != CURLE_OK || status < 200 || status >= 300 || response.exceededLimit)
	{
		Log("TURN credentials unavailable; using direct ICE");
		return {};
	}
	const auto parsed = ParseIceServersResponse(response.body, true, &parseError);
	if (!parsed)
	{
		LogError("TURN response was malformed; using direct ICE: " + parseError);
		return {};
	}
	return *parsed;
}

std::vector<rtc::IceServer> BuildIceServers(const std::vector<IceServerSpec> &specs)
{
	std::vector<rtc::IceServer> result;
	for (const IceServerSpec &spec : specs)
	{
		for (const std::string &url : spec.urls)
		{
			try
			{
				rtc::IceServer server(url);
				if (server.type == rtc::IceServer::Type::Turn)
				{
					server.username = spec.username;
					server.password = spec.credential;
				}
				result.push_back(std::move(server));
			}
			catch (const std::exception &)
			{
				Log("ignored an unsupported ICE server URL");
			}
		}
	}
	return result;
}

} // namespace

class NativeWebRTCTransport::Impl : public std::enable_shared_from_this<NativeWebRTCTransport::Impl>
{
public:
	Impl() = default;

	~Impl()
	{
		Stop();
	}

	void ConfigureFromProcess(CompatibilityProfile compatibility)
	{
		std::lock_guard lock(m_stateMutex);
		if (m_configured)
		{
			return;
		}
		m_configured = true;
		m_config = ParseRuntimeConfig(ReadProcessArguments(), ReadProcessEnvironment(), compatibility);
		const std::uint64_t generation = m_lifecycle.Start(m_config.enabled);
		if (!m_config.enabled)
		{
			return;
		}

		std::string error;
		if (!ValidateRuntimeConfig(m_config, &error))
		{
			m_error = error;
			m_lifecycle.Transition(generation, LifecycleState::Failed);
			m_stateChanged.notify_all();
			LogError("configuration rejected: " + error);
			return;
		}

		m_worker = std::thread([this, generation]() {
			Run(generation);
		});
	}

	bool IsEnabled() const
	{
		std::lock_guard lock(m_stateMutex);
		return m_configured && m_config.enabled;
	}

	bool HasFailed() const
	{
		std::lock_guard lock(m_stateMutex);
		return m_lifecycle.State() == LifecycleState::Failed;
	}

	std::string LastError() const
	{
		std::lock_guard lock(m_stateMutex);
		return m_error;
	}

	std::uint16_t Bind(std::uint16_t port)
	{
		std::lock_guard lock(m_inboxMutex);
		return m_inbox.Bind(port);
	}

	void Unbind(std::uint16_t port)
	{
		std::lock_guard lock(m_inboxMutex);
		m_inbox.Unbind(port);
	}

	bool Send(
		std::uint32_t destinationAddress,
		std::uint16_t destinationPort,
		std::uint16_t sourcePort,
		const std::uint8_t *payload,
		std::size_t payloadSize)
	{
		const std::vector<std::uint8_t> frame =
			EncodeUdpFrame(sourcePort, destinationPort, payload, payloadSize);
		std::vector<std::shared_ptr<rtc::DataChannel>> channels;
		{
			std::lock_guard lock(m_peersMutex);
			if (destinationAddress == BROADCAST_ADDRESS)
			{
				for (const auto &[slot, peer] : m_peers)
				{
					(void)slot;
					if (peer.channel != nullptr)
					{
						channels.push_back(peer.channel);
					}
				}
			}
			else if (const auto slot = SyntheticAddressToSlot(destinationAddress))
			{
				const auto found = m_peers.find(*slot);
				if (found != m_peers.end() && found->second.channel != nullptr)
				{
					channels.push_back(found->second.channel);
				}
			}
		}

		bool accepted = false;
		for (const std::shared_ptr<rtc::DataChannel> &channel : channels)
		{
			if (!channel->isOpen() || frame.size() > MAX_BUFFERED_BYTES ||
				channel->bufferedAmount() > MAX_BUFFERED_BYTES - frame.size())
			{
				continue;
			}
			try
			{
				channel->send(
					reinterpret_cast<const rtc::byte *>(frame.data()),
					frame.size());
				accepted = true;
			}
			catch (const std::exception &)
			{
				// Datagram semantics intentionally drop failed sends.
			}
		}
		return accepted;
	}

	int Receive(
		std::uint16_t destinationPort,
		std::uint8_t *payload,
		std::size_t payloadCapacity,
		ReceivedDatagram *received)
	{
		std::optional<Datagram> datagram;
		{
			std::lock_guard lock(m_inboxMutex);
			datagram = m_inbox.Pop(destinationPort);
		}
		if (datagram)
		{
			const std::size_t copied = std::min(payloadCapacity, datagram->payload.size());
			if (copied > 0)
			{
				std::memcpy(payload, datagram->payload.data(), copied);
			}
			if (received != nullptr)
			{
				received->sourceAddress = datagram->sourceAddress;
				received->sourcePort = datagram->sourcePort;
				received->size = copied;
			}
			return static_cast<int>(copied);
		}
		return HasFailed() ? -1 : 0;
	}

	std::uint32_t LocalAddress() const
	{
		std::lock_guard lock(m_stateMutex);
		return m_localAddress;
	}

	std::uint32_t WaitForLocalAddress(unsigned int timeoutMilliseconds) const
	{
		std::unique_lock lock(m_stateMutex);
		m_stateChanged.wait_for(lock, std::chrono::milliseconds(timeoutMilliseconds), [this]() {
			return m_localAddress != 0 || m_lifecycle.IsTerminal();
		});
		return m_localAddress;
	}

private:
	enum class EventType
	{
		WebSocketOpen,
		WebSocketMessage,
		WebSocketClosed,
		WebSocketError,
		LocalDescription,
		LocalCandidate,
		IncomingDataChannel,
		DataChannelOpen,
		DataChannelClosed,
		PeerFailed,
		Stop,
	};

	struct Event
	{
		EventType type = EventType::Stop;
		std::uint64_t generation = 0;
		int slot = -1;
		std::string first;
		std::string second;
		std::shared_ptr<rtc::DataChannel> channel;
	};

	struct Peer
	{
		std::shared_ptr<rtc::PeerConnection> connection;
		std::shared_ptr<rtc::DataChannel> channel;
		bool ignoreOffer = false;
	};

	void Run(std::uint64_t generation)
	{
		try
		{
			{
				std::lock_guard lock(m_stateMutex);
				m_lifecycle.Transition(generation, LifecycleState::FetchingTurn);
			}
			const std::vector<rtc::IceServer> iceServers = BuildIceServers(FetchIceServerSpecs(m_config));
			OpenWebSocket(generation, iceServers);

			for (;;)
			{
				Event event;
				{
					std::unique_lock lock(m_eventMutex);
					m_eventChanged.wait(lock, [this]() {
						return !m_events.empty();
					});
					event = std::move(m_events.front());
					m_events.pop_front();
				}
				if (event.generation != generation)
				{
					continue;
				}
				if (event.type == EventType::Stop)
				{
					if (m_webSocket != nullptr && m_webSocket->isOpen())
					{
						m_webSocket->send(BuildLeaveMessage());
					}
					break;
				}
				HandleEvent(event, generation, iceServers);
				if (HasFailed())
				{
					break;
				}
			}
		}
		catch (const std::exception &error)
		{
			Fail(generation, error.what());
		}

		CloseAllPeers();
		CloseWebSocket();
		{
			std::lock_guard lock(m_stateMutex);
			if (m_lifecycle.State() == LifecycleState::Stopping)
			{
				m_lifecycle.Transition(generation, LifecycleState::Stopped);
			}
			m_stateChanged.notify_all();
		}
	}

	void OpenWebSocket(std::uint64_t generation, const std::vector<rtc::IceServer> &iceServers)
	{
		rtc::WebSocket::Configuration configuration;
		configuration.connectionTimeout =
			std::chrono::duration_cast<std::chrono::milliseconds>(WEBSOCKET_CONNECT_TIMEOUT);
		configuration.pingInterval =
			std::chrono::duration_cast<std::chrono::milliseconds>(WEBSOCKET_PING_INTERVAL);
		configuration.maxOutstandingPings = 2;
		configuration.maxMessageSize = MAX_SIGNALING_MESSAGE_BYTES;

		m_webSocket = std::make_shared<rtc::WebSocket>(configuration);
		std::weak_ptr<Impl> weakSelf = shared_from_this();
		m_webSocket->onOpen([weakSelf, generation]() {
			if (const auto self = weakSelf.lock())
			{
				self->Enqueue({ EventType::WebSocketOpen, generation });
			}
		});
		m_webSocket->onMessage([weakSelf, generation](rtc::message_variant data) {
			if (const auto self = weakSelf.lock(); self && std::holds_alternative<rtc::string>(data))
			{
				self->Enqueue({
					EventType::WebSocketMessage,
					generation,
					-1,
					std::get<rtc::string>(std::move(data)),
				});
			}
		});
		m_webSocket->onClosed([weakSelf, generation]() {
			if (const auto self = weakSelf.lock())
			{
				self->Enqueue({ EventType::WebSocketClosed, generation });
			}
		});
		m_webSocket->onError([weakSelf, generation](rtc::string error) {
			if (const auto self = weakSelf.lock())
			{
				self->Enqueue({ EventType::WebSocketError, generation, -1, std::move(error) });
			}
		});

		{
			std::lock_guard lock(m_stateMutex);
			m_lifecycle.Transition(generation, LifecycleState::Signaling);
		}
		m_webSocket->open(BuildRoomWebSocketUrl(m_config));
	}

	void HandleEvent(
		const Event &event,
		std::uint64_t generation,
		const std::vector<rtc::IceServer> &iceServers)
	{
		switch (event.type)
		{
			case EventType::WebSocketOpen:
				if (m_webSocket == nullptr || !m_webSocket->isOpen())
				{
					Fail(generation, "failed to send room join request");
				}
				else
				{
					try
					{
						m_webSocket->send(BuildJoinMessage(m_config));
					}
					catch (const std::exception &error)
					{
						Fail(generation, error.what());
					}
				}
				break;
			case EventType::WebSocketMessage:
				HandleServerMessage(event.first, generation, iceServers);
				break;
			case EventType::WebSocketClosed:
				if (!IsStopping())
				{
					if (CanContinueWithoutSignaling())
					{
						Log("signaling socket closed after peers connected; keeping DataChannels alive");
						CloseWebSocket();
					}
					else
					{
						Fail(generation, "signaling socket closed");
					}
				}
				break;
			case EventType::WebSocketError:
				if (CanContinueWithoutSignaling())
				{
					Log("signaling socket failed after peers connected; keeping DataChannels alive");
					CloseWebSocket();
				}
				else
				{
					Fail(generation, event.first.empty() ? "signaling socket failed" : event.first);
				}
				break;
			case EventType::LocalDescription:
				SendSignaling(BuildDescriptionMessage(event.slot, event.first, event.second), generation);
				break;
			case EventType::LocalCandidate:
				SendSignaling(BuildCandidateMessage(event.slot, event.first, event.second), generation);
				break;
			case EventType::IncomingDataChannel:
				BindDataChannel(event.slot, event.channel, generation);
				break;
			case EventType::DataChannelOpen:
				MarkConnected(generation);
				break;
			case EventType::DataChannelClosed:
				MarkJoinedIfNoChannels(generation);
				break;
			case EventType::PeerFailed:
				RemovePeer(event.slot);
				MarkJoinedIfNoChannels(generation);
				break;
			case EventType::Stop:
				break;
		}
	}

	void HandleServerMessage(
		const std::string &raw,
		std::uint64_t generation,
		const std::vector<rtc::IceServer> &iceServers)
	{
		std::string error;
		const std::optional<ServerMessage> message = ParseServerMessage(raw, &error);
		if (!message)
		{
			LogError("ignored malformed signaling message: " + error);
			return;
		}

		switch (message->type)
		{
			case ServerMessageType::Welcome:
				if (message->roomId != m_config.roomId)
				{
					Fail(generation, "signaling welcome room did not match the requested room");
					return;
				}
				{
					std::lock_guard lock(m_stateMutex);
					m_localSlot = message->slot;
					m_localAddress = SlotToSyntheticAddress(message->slot);
					m_lifecycle.Transition(generation, LifecycleState::Joined);
					m_stateChanged.notify_all();
				}
				UpdateRoster(message->roster, generation, iceServers);
				LogError("joined room " + message->roomId + " as synthetic address 10.0.0." +
					std::to_string(message->slot + 1));
				break;
			case ServerMessageType::Roster:
				UpdateRoster(message->roster, generation, iceServers);
				break;
			case ServerMessageType::Offer:
			case ServerMessageType::Answer:
			{
				const std::shared_ptr<rtc::PeerConnection> connection =
					EnsurePeer(message->from, generation, iceServers);
				if (connection != nullptr)
				{
					int localSlot = -1;
					{
						std::lock_guard lock(m_stateMutex);
						localSlot = m_localSlot;
					}
					const bool ignoreOffer =
						message->type == ServerMessageType::Offer &&
						!IsPolitePeer(localSlot, message->from) &&
						connection->signalingState() != rtc::PeerConnection::SignalingState::Stable;
					{
						std::lock_guard lock(m_peersMutex);
						const auto peer = m_peers.find(message->from);
						if (peer != m_peers.end())
						{
							peer->second.ignoreOffer = ignoreOffer;
						}
					}
					if (ignoreOffer)
					{
						LogError("ignored a colliding offer from peer " + std::to_string(message->from));
						break;
					}
					try
					{
						connection->setRemoteDescription(rtc::Description(
							message->sdp,
							message->type == ServerMessageType::Offer ? "offer" : "answer"));
					}
					catch (const std::exception &descriptionError)
					{
						LogError("ignored invalid remote description: " + std::string(descriptionError.what()));
					}
				}
				break;
			}
			case ServerMessageType::Ice:
			{
				const std::shared_ptr<rtc::PeerConnection> connection =
					EnsurePeer(message->from, generation, iceServers);
				if (connection != nullptr)
				{
					{
						std::lock_guard lock(m_peersMutex);
						const auto peer = m_peers.find(message->from);
						if (peer != m_peers.end() && peer->second.ignoreOffer)
						{
							break;
						}
					}
					try
					{
						connection->addRemoteCandidate(rtc::Candidate(message->candidate, message->mid));
					}
					catch (const std::exception &candidateError)
					{
						LogError("ignored invalid ICE candidate: " + std::string(candidateError.what()));
					}
				}
				break;
			}
			case ServerMessageType::PeerLeft:
				RemovePeer(message->slot);
				MarkJoinedIfNoChannels(generation);
				break;
			case ServerMessageType::Error:
				if (message->errorCode == "UNKNOWN_TARGET_SLOT" || message->errorCode == "RATE_LIMITED")
				{
					LogError("signaling warning " + message->errorCode + ": " + message->errorMessage);
				}
				else
				{
					Fail(generation, "signaling error " + message->errorCode + ": " + message->errorMessage);
				}
				break;
		}
	}

	void UpdateRoster(
		const std::vector<RosterEntry> &roster,
		std::uint64_t generation,
		const std::vector<rtc::IceServer> &iceServers)
	{
		int localSlot = -1;
		{
			std::lock_guard lock(m_stateMutex);
			localSlot = m_localSlot;
		}
		std::set<int> liveSlots;
		for (const RosterEntry &entry : roster)
		{
			liveSlots.insert(entry.slot);
			if (entry.slot != localSlot)
			{
				EnsurePeer(entry.slot, generation, iceServers);
			}
		}

		std::vector<int> removed;
		{
			std::lock_guard lock(m_peersMutex);
			for (const auto &[slot, peer] : m_peers)
			{
				(void)peer;
				if (!liveSlots.contains(slot))
				{
					removed.push_back(slot);
				}
			}
		}
		for (int slot : removed)
		{
			RemovePeer(slot);
		}
	}

	std::shared_ptr<rtc::PeerConnection> EnsurePeer(
		int slot,
		std::uint64_t generation,
		const std::vector<rtc::IceServer> &iceServers)
	{
		int localSlot = -1;
		{
			std::lock_guard lock(m_stateMutex);
			localSlot = m_localSlot;
		}
		if (slot < 0 || slot >= MAX_ROOM_CAPACITY || localSlot < 0 || slot == localSlot)
		{
			return nullptr;
		}
		{
			std::lock_guard lock(m_peersMutex);
			const auto found = m_peers.find(slot);
			if (found != m_peers.end())
			{
				return found->second.connection;
			}
		}

		rtc::Configuration configuration;
		configuration.iceServers = iceServers;
		configuration.iceTransportPolicy =
			m_config.forceRelay ? rtc::TransportPolicy::Relay : rtc::TransportPolicy::All;
		configuration.maxMessageSize = 64 * 1024;
		auto connection = std::make_shared<rtc::PeerConnection>(configuration);
		std::weak_ptr<Impl> weakSelf = shared_from_this();
		connection->onLocalDescription([weakSelf, generation, slot](rtc::Description description) {
			if (const auto self = weakSelf.lock())
			{
				self->Enqueue({
					EventType::LocalDescription,
					generation,
					slot,
					description.typeString(),
					static_cast<std::string>(description),
				});
			}
		});
		connection->onLocalCandidate([weakSelf, generation, slot](rtc::Candidate candidate) {
			if (const auto self = weakSelf.lock())
			{
				self->Enqueue({
					EventType::LocalCandidate,
					generation,
					slot,
					candidate.candidate(),
					candidate.mid(),
				});
			}
		});
		connection->onDataChannel([weakSelf, generation, slot](std::shared_ptr<rtc::DataChannel> channel) {
			if (const auto self = weakSelf.lock())
			{
				self->Enqueue({
					EventType::IncomingDataChannel,
					generation,
					slot,
					{},
					{},
					std::move(channel),
				});
			}
		});
		connection->onStateChange([weakSelf, generation, slot](rtc::PeerConnection::State state) {
			if (const auto self = weakSelf.lock();
				self && (state == rtc::PeerConnection::State::Failed || state == rtc::PeerConnection::State::Closed))
			{
				self->Enqueue({ EventType::PeerFailed, generation, slot });
			}
		});

		{
			std::lock_guard lock(m_peersMutex);
			const auto [found, inserted] = m_peers.emplace(slot, Peer { connection, nullptr });
			if (!inserted)
			{
				connection->resetCallbacks();
				connection->close();
				return found->second.connection;
			}
		}

		if (ShouldCreateDataChannel(localSlot, slot))
		{
			rtc::DataChannelInit init;
			init.reliability.unordered = true;
			init.reliability.maxRetransmits = MAX_RETRANSMITS;
			BindDataChannel(slot, connection->createDataChannel(UDP_DATA_CHANNEL_LABEL, init), generation);
		}
		return connection;
	}

	void BindDataChannel(
		int slot,
		const std::shared_ptr<rtc::DataChannel> &channel,
		std::uint64_t generation)
	{
		if (channel == nullptr || channel->label() != UDP_DATA_CHANNEL_LABEL)
		{
			if (channel != nullptr)
			{
				channel->close();
			}
			return;
		}
		const rtc::Reliability reliability = channel->reliability();
		if (!reliability.unordered || reliability.maxRetransmits != MAX_RETRANSMITS)
		{
			channel->close();
			Log("rejected a DataChannel with incompatible reliability settings");
			return;
		}

		std::weak_ptr<Impl> weakSelf = shared_from_this();
		channel->setBufferedAmountLowThreshold(MAX_BUFFERED_BYTES / 2);
		channel->onOpen([weakSelf, generation, slot]() {
			if (const auto self = weakSelf.lock())
			{
				self->Enqueue({ EventType::DataChannelOpen, generation, slot });
			}
		});
		channel->onClosed([weakSelf, generation, slot]() {
			if (const auto self = weakSelf.lock())
			{
				self->Enqueue({ EventType::DataChannelClosed, generation, slot });
			}
		});
		channel->onError([weakSelf](rtc::string error) {
			if (const auto self = weakSelf.lock())
			{
				LogError("DataChannel error: " + error);
			}
		});
		channel->onMessage([weakSelf, slot](rtc::message_variant data) {
			if (const auto self = weakSelf.lock(); self && std::holds_alternative<rtc::binary>(data))
			{
				const rtc::binary &binary = std::get<rtc::binary>(data);
				self->Deliver(
					slot,
					reinterpret_cast<const std::uint8_t *>(binary.data()),
					binary.size());
			}
		});

		std::shared_ptr<rtc::DataChannel> replaced;
		{
			std::lock_guard lock(m_peersMutex);
			const auto found = m_peers.find(slot);
			if (found == m_peers.end())
			{
				channel->resetCallbacks();
				channel->close();
				return;
			}
			replaced = std::exchange(found->second.channel, channel);
		}
		if (replaced != nullptr && replaced != channel)
		{
			replaced->resetCallbacks();
			replaced->close();
		}
	}

	void Deliver(int sourceSlot, const std::uint8_t *frame, std::size_t frameSize)
	{
		std::lock_guard lock(m_inboxMutex);
		m_inbox.Push(sourceSlot, frame, frameSize);
	}

	void RemovePeer(int slot)
	{
		Peer removed;
		bool found = false;
		{
			std::lock_guard lock(m_peersMutex);
			const auto peer = m_peers.find(slot);
			if (peer != m_peers.end())
			{
				removed = std::move(peer->second);
				m_peers.erase(peer);
				found = true;
			}
		}
		if (found)
		{
			if (removed.channel != nullptr)
			{
				removed.channel->resetCallbacks();
				removed.channel->close();
			}
			if (removed.connection != nullptr)
			{
				removed.connection->resetCallbacks();
				removed.connection->close();
			}
			std::lock_guard lock(m_inboxMutex);
			m_inbox.RemoveSourceSlot(slot);
		}
	}

	void CloseAllPeers()
	{
		std::vector<int> slots;
		{
			std::lock_guard lock(m_peersMutex);
			for (const auto &[slot, peer] : m_peers)
			{
				(void)peer;
				slots.push_back(slot);
			}
		}
		for (int slot : slots)
		{
			RemovePeer(slot);
		}
	}

	void CloseWebSocket()
	{
		if (m_webSocket != nullptr)
		{
			m_webSocket->resetCallbacks();
			m_webSocket->close();
			m_webSocket.reset();
		}
	}

	void SendSignaling(const std::string &message, std::uint64_t generation)
	{
		if (m_webSocket == nullptr || !m_webSocket->isOpen())
		{
			if (!CanContinueWithoutSignaling())
			{
				Fail(generation, "failed to send signaling message");
			}
			return;
		}
		try
		{
			m_webSocket->send(message);
		}
		catch (const std::exception &error)
		{
			if (!CanContinueWithoutSignaling())
			{
				Fail(generation, error.what());
			}
		}
	}

	void MarkConnected(std::uint64_t generation)
	{
		std::lock_guard lock(m_stateMutex);
		if (m_lifecycle.State() == LifecycleState::Joined)
		{
			m_lifecycle.Transition(generation, LifecycleState::Connected);
		}
	}

	void MarkJoinedIfNoChannels(std::uint64_t generation)
	{
		bool anyOpen = false;
		{
			std::lock_guard lock(m_peersMutex);
			for (const auto &[slot, peer] : m_peers)
			{
				(void)slot;
				if (peer.channel != nullptr && peer.channel->isOpen())
				{
					anyOpen = true;
					break;
				}
			}
		}
		if (!anyOpen)
		{
			std::lock_guard lock(m_stateMutex);
			if (m_lifecycle.State() == LifecycleState::Connected)
			{
				m_lifecycle.Transition(generation, LifecycleState::Joined);
			}
		}
	}

	void Fail(std::uint64_t generation, const std::string &error)
	{
		bool changed = false;
		{
			std::lock_guard lock(m_stateMutex);
			if (m_lifecycle.State() != LifecycleState::Failed &&
				m_lifecycle.State() != LifecycleState::Stopping &&
				m_lifecycle.State() != LifecycleState::Stopped)
			{
				m_error = error;
				changed = m_lifecycle.Transition(generation, LifecycleState::Failed);
				m_stateChanged.notify_all();
			}
		}
		if (changed)
		{
			LogError("transport failed: " + error);
		}
	}

	bool IsStopping() const
	{
		std::lock_guard lock(m_stateMutex);
		return m_lifecycle.State() == LifecycleState::Stopping ||
			m_lifecycle.State() == LifecycleState::Stopped;
	}

	bool CanContinueWithoutSignaling() const
	{
		std::lock_guard lock(m_stateMutex);
		const LifecycleState state = m_lifecycle.State();
		return state == LifecycleState::Connected;
	}

	void Enqueue(Event event)
	{
		std::lock_guard lock(m_eventMutex);
		if (m_stopQueued)
		{
			return;
		}
		if (event.type == EventType::Stop)
		{
			m_events.clear();
			m_stopQueued = true;
			m_events.push_back(std::move(event));
			m_eventChanged.notify_one();
			return;
		}
		if (m_events.size() >= MAX_CONTROL_EVENTS)
		{
			m_events.clear();
			event = {
				EventType::WebSocketError,
				event.generation,
				-1,
				"control event queue overflow",
			};
		}
		m_events.push_back(std::move(event));
		m_eventChanged.notify_one();
	}

	void Stop()
	{
		std::uint64_t generation = 0;
		{
			std::lock_guard lock(m_stateMutex);
			if (!m_worker.joinable())
			{
				return;
			}
			generation = m_lifecycle.Generation();
			m_lifecycle.Transition(generation, LifecycleState::Stopping);
		}
		Enqueue({ EventType::Stop, generation });
		m_worker.join();
	}

	mutable std::mutex m_stateMutex;
	mutable std::condition_variable m_stateChanged;
	RuntimeConfig m_config;
	bool m_configured = false;
	std::string m_error;
	int m_localSlot = -1;
	std::uint32_t m_localAddress = 0;
	LifecycleTracker m_lifecycle;

	std::mutex m_eventMutex;
	std::condition_variable m_eventChanged;
	std::deque<Event> m_events;
	bool m_stopQueued = false;
	std::thread m_worker;

	std::mutex m_inboxMutex;
	BoundedDatagramInbox m_inbox;

	std::mutex m_peersMutex;
	std::map<int, Peer> m_peers;
	std::shared_ptr<rtc::WebSocket> m_webSocket;
};

NativeWebRTCTransport &NativeWebRTCTransport::Instance()
{
	static NativeWebRTCTransport instance;
	return instance;
}

NativeWebRTCTransport::NativeWebRTCTransport()
	: m_impl(std::make_shared<Impl>())
{
}

NativeWebRTCTransport::~NativeWebRTCTransport() = default;

void NativeWebRTCTransport::ConfigureFromProcess(CompatibilityProfile compatibility)
{
	m_impl->ConfigureFromProcess(compatibility);
}

bool NativeWebRTCTransport::IsEnabled() const
{
	return m_impl->IsEnabled();
}

bool NativeWebRTCTransport::HasFailed() const
{
	return m_impl->HasFailed();
}

std::string NativeWebRTCTransport::LastError() const
{
	return m_impl->LastError();
}

std::uint16_t NativeWebRTCTransport::Bind(std::uint16_t port)
{
	return m_impl->Bind(port);
}

void NativeWebRTCTransport::Unbind(std::uint16_t port)
{
	m_impl->Unbind(port);
}

bool NativeWebRTCTransport::Send(
	std::uint32_t destinationAddress,
	std::uint16_t destinationPort,
	std::uint16_t sourcePort,
	const std::uint8_t *payload,
	std::size_t payloadSize)
{
	return m_impl->Send(destinationAddress, destinationPort, sourcePort, payload, payloadSize);
}

int NativeWebRTCTransport::Receive(
	std::uint16_t destinationPort,
	std::uint8_t *payload,
	std::size_t payloadCapacity,
	ReceivedDatagram *received)
{
	return m_impl->Receive(destinationPort, payload, payloadCapacity, received);
}

std::uint32_t NativeWebRTCTransport::LocalAddress() const
{
	return m_impl->LocalAddress();
}

std::uint32_t NativeWebRTCTransport::WaitForLocalAddress(unsigned int timeoutMilliseconds) const
{
	return m_impl->WaitForLocalAddress(timeoutMilliseconds);
}

} // namespace GeneralsX::NativeWebRTC
