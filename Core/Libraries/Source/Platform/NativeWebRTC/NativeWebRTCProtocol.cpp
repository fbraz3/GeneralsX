/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
*/

#include "NativeWebRTCProtocol.h"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <limits>
#include <regex>

#include <nlohmann/json.hpp>

// GeneralsX @feature Copilot 30/08/2026 Implement the native mirror of the
// Cloudflare room protocol and browser bridge's exact datagram semantics.
namespace GeneralsX::NativeWebRTC
{
namespace
{

using Json = nlohmann::json;

constexpr std::uint16_t FIRST_EPHEMERAL_PORT = 49152;
constexpr std::uint16_t LAST_EPHEMERAL_PORT = 65535;

const std::regex ROOM_ID_PATTERN("^[A-Z0-9]{4,10}$");
const std::regex PLAYER_NAME_PATTERN("^[A-Za-z0-9_ .-]{1,24}$");

bool IsTruthy(const std::string &value)
{
	std::string normalized;
	normalized.reserve(value.size());
	std::transform(value.begin(), value.end(), std::back_inserter(normalized), [](unsigned char ch) {
		return static_cast<char>(std::tolower(ch));
	});
	return normalized == "1" || normalized == "true" || normalized == "yes" || normalized == "on";
}

std::optional<int> ParseInteger(const std::string &value)
{
	int result = 0;
	const char *first = value.data();
	const char *last = first + value.size();
	const std::from_chars_result parsed = std::from_chars(first, last, result);
	if (parsed.ec != std::errc() || parsed.ptr != last)
	{
		return std::nullopt;
	}
	return result;
}

std::string NormalizeOption(std::string option)
{
	if (option.rfind("--", 0) == 0)
	{
		option.erase(0, 1);
	}
	return option;
}

std::optional<std::string> EnvironmentValue(
	const std::unordered_map<std::string, std::string> &environment,
	const char *name)
{
	const auto found = environment.find(name);
	if (found == environment.end() || found->second.empty())
	{
		return std::nullopt;
	}
	return found->second;
}

std::optional<std::string> OriginForUrl(const std::string &base, std::string *error)
{
	const std::size_t schemeEnd = base.find("://");
	if (schemeEnd == std::string::npos)
	{
		if (error != nullptr)
		{
			*error = "signaling URL must include http:// or https://";
		}
		return std::nullopt;
	}

	const std::string scheme = base.substr(0, schemeEnd);
	if (scheme != "http" && scheme != "https")
	{
		if (error != nullptr)
		{
			*error = "signaling URL scheme must be http or https";
		}
		return std::nullopt;
	}

	const std::size_t authorityStart = schemeEnd + 3;
	const std::size_t authorityEnd = base.find_first_of("/?#", authorityStart);
	const std::string authority = base.substr(authorityStart, authorityEnd - authorityStart);
	if (authority.empty() || authority.find('@') != std::string::npos ||
		std::any_of(authority.begin(), authority.end(), [](unsigned char ch) {
			return std::isspace(ch) != 0;
		}))
	{
		if (error != nullptr)
		{
			*error = "signaling URL authority is missing or contains credentials";
		}
		return std::nullopt;
	}
	return scheme + "://" + authority;
}

bool ReadRoster(const Json &input, std::vector<RosterEntry> *roster, std::string *error)
{
	if (!input.is_array() || input.size() > static_cast<std::size_t>(MAX_ROOM_CAPACITY))
	{
		if (error != nullptr)
		{
			*error = "roster must be an array within room capacity";
		}
		return false;
	}

	std::set<int> seenSlots;
	for (const Json &entry : input)
	{
		if (!entry.is_object() ||
			!entry.contains("slot") || !entry["slot"].is_number_integer() ||
			!entry.contains("name") || !entry["name"].is_string() ||
			!entry.contains("isHost") || !entry["isHost"].is_boolean())
		{
			if (error != nullptr)
			{
				*error = "roster entry is malformed";
			}
			return false;
		}

		const int slot = entry["slot"].get<int>();
		const std::string name = entry["name"].get<std::string>();
		if (slot < 0 || slot >= MAX_ROOM_CAPACITY ||
			!std::regex_match(name, PLAYER_NAME_PATTERN) ||
			!seenSlots.insert(slot).second)
		{
			if (error != nullptr)
			{
				*error = "roster entry contains an invalid or duplicate slot";
			}
			return false;
		}
		roster->push_back({ slot, name, entry["isHost"].get<bool>() });
	}
	return true;
}

std::optional<std::vector<IceServerSpec>> ParseIceServersArray(const Json &servers, std::string *error)
{
	if (!servers.is_array() || servers.size() > 16)
	{
		if (error != nullptr)
		{
			*error = "iceServers must be an array with at most 16 entries";
		}
		return std::nullopt;
	}

	std::vector<IceServerSpec> result;
	for (const Json &server : servers)
	{
		if (!server.is_object() || !server.contains("urls"))
		{
			if (error != nullptr)
			{
				*error = "ICE server entry is missing urls";
			}
			return std::nullopt;
		}

		IceServerSpec spec;
		const Json &urls = server["urls"];
		if (urls.is_string())
		{
			spec.urls.push_back(urls.get<std::string>());
		}
		else if (urls.is_array() && urls.size() <= 8)
		{
			for (const Json &url : urls)
			{
				if (!url.is_string())
				{
					if (error != nullptr)
					{
						*error = "ICE server urls must contain strings";
					}
					return std::nullopt;
				}
				spec.urls.push_back(url.get<std::string>());
			}
		}
		else
		{
			if (error != nullptr)
			{
				*error = "ICE server urls must be a string or string array";
			}
			return std::nullopt;
		}

		if (spec.urls.empty())
		{
			if (error != nullptr)
			{
				*error = "ICE server urls cannot be empty";
			}
			return std::nullopt;
		}
		for (const std::string &url : spec.urls)
		{
			if (url.size() > 2048 ||
				(url.rfind("stun:", 0) != 0 && url.rfind("stuns:", 0) != 0 &&
				 url.rfind("turn:", 0) != 0 && url.rfind("turns:", 0) != 0))
			{
				if (error != nullptr)
				{
					*error = "ICE server URL has an unsupported scheme";
				}
				return std::nullopt;
			}
		}

		if (server.contains("username"))
		{
			if (!server["username"].is_string() || server["username"].get_ref<const std::string &>().size() > 1024)
			{
				if (error != nullptr)
				{
					*error = "ICE server username is malformed";
				}
				return std::nullopt;
			}
			spec.username = server["username"].get<std::string>();
		}
		if (server.contains("credential"))
		{
			if (!server["credential"].is_string() || server["credential"].get_ref<const std::string &>().size() > 2048)
			{
				if (error != nullptr)
				{
					*error = "ICE server credential is malformed";
				}
				return std::nullopt;
			}
			spec.credential = server["credential"].get<std::string>();
		}
		result.push_back(std::move(spec));
	}
	return result;
}

bool IsValidSlot(int slot)
{
	return slot >= 0 && slot < MAX_ROOM_CAPACITY;
}

} // namespace

RuntimeConfig ParseRuntimeConfig(
	const std::vector<std::string> &arguments,
	const std::unordered_map<std::string, std::string> &environment)
{
	RuntimeConfig config;
	if (const auto value = EnvironmentValue(environment, "GENERALSX_WEBRTC"))
	{
		config.enabled = IsTruthy(*value);
	}
	if (const auto value = EnvironmentValue(environment, "GENERALSX_WEBRTC_SIGNALING_URL"))
	{
		config.signalingUrl = *value;
	}
	if (const auto value = EnvironmentValue(environment, "GENERALSX_WEBRTC_ROOM"))
	{
		config.roomId = *value;
	}
	if (const auto value = EnvironmentValue(environment, "GENERALSX_WEBRTC_PLAYER_NAME"))
	{
		config.playerName = *value;
	}
	if (const auto value = EnvironmentValue(environment, "GENERALSX_WEBRTC_CAPACITY"))
	{
		if (const auto parsed = ParseInteger(*value))
		{
			config.capacity = *parsed;
		}
	}
	if (const auto value = EnvironmentValue(environment, "GENERALSX_WEBRTC_DISABLE_TURN"))
	{
		config.fetchTurnCredentials = !IsTruthy(*value);
	}
	if (const auto value = EnvironmentValue(environment, "GENERALSX_WEBRTC_ICE_SERVERS_JSON"))
	{
		config.iceServersJson = *value;
	}

	for (std::size_t index = 1; index < arguments.size(); ++index)
	{
		const std::string option = NormalizeOption(arguments[index]);
		if (option == "-webrtc" || option == "-native-webrtc")
		{
			config.enabled = true;
		}
		else if (option == "-webrtc-no-turn")
		{
			config.fetchTurnCredentials = false;
		}
		else if (index + 1 < arguments.size())
		{
			const std::string &value = arguments[index + 1];
			if (option == "-webrtc-signaling")
			{
				config.signalingUrl = value;
				++index;
			}
			else if (option == "-webrtc-room")
			{
				config.roomId = value;
				++index;
			}
			else if (option == "-webrtc-name")
			{
				config.playerName = value;
				++index;
			}
			else if (option == "-webrtc-capacity")
			{
				if (const auto parsed = ParseInteger(value))
				{
					config.capacity = *parsed;
				}
				++index;
			}
		}
	}

	std::transform(config.roomId.begin(), config.roomId.end(), config.roomId.begin(), [](unsigned char ch) {
		return static_cast<char>(std::toupper(ch));
	});
	return config;
}

bool ValidateRuntimeConfig(const RuntimeConfig &config, std::string *error)
{
	if (!config.enabled)
	{
		return true;
	}
	if (!OriginForUrl(config.signalingUrl, error))
	{
		return false;
	}
	if (!std::regex_match(config.roomId, ROOM_ID_PATTERN))
	{
		if (error != nullptr)
		{
			*error = "room id must contain 4-10 uppercase letters or digits";
		}
		return false;
	}
	if (!std::regex_match(config.playerName, PLAYER_NAME_PATTERN))
	{
		if (error != nullptr)
		{
			*error = "player name must contain 1-24 safe printable characters";
		}
		return false;
	}
	if (config.capacity < MIN_ROOM_CAPACITY || config.capacity > MAX_ROOM_CAPACITY)
	{
		if (error != nullptr)
		{
			*error = "capacity must be between 2 and 8";
		}
		return false;
	}
	if (config.iceServersJson.size() > 64 * 1024)
	{
		if (error != nullptr)
		{
			*error = "ICE server JSON exceeds the size limit";
		}
		return false;
	}
	return true;
}

std::string BuildRoomWebSocketUrl(const RuntimeConfig &config)
{
	std::string error;
	const std::optional<std::string> origin = OriginForUrl(config.signalingUrl, &error);
	if (!origin)
	{
		return {};
	}
	std::string websocketOrigin = *origin;
	websocketOrigin.replace(0, websocketOrigin.rfind("://"), websocketOrigin.rfind("://") == 5 ? "wss" : "ws");
	return websocketOrigin + "/room?roomId=" + config.roomId + "&capacity=" + std::to_string(config.capacity);
}

std::string BuildTurnCredentialsUrl(const RuntimeConfig &config)
{
	std::string error;
	const std::optional<std::string> origin = OriginForUrl(config.signalingUrl, &error);
	return origin ? *origin + "/turn-credentials" : std::string();
}

std::uint32_t SlotToSyntheticAddress(int slot)
{
	if (slot < 0 || slot > 254)
	{
		return 0;
	}
	return 0x0A000000U | static_cast<std::uint32_t>(slot + 1);
}

std::optional<int> SyntheticAddressToSlot(std::uint32_t address)
{
	if ((address & 0xFFFFFF00U) != 0x0A000000U)
	{
		return std::nullopt;
	}
	const std::uint32_t lastOctet = address & 0xFFU;
	if (lastOctet == 0)
	{
		return std::nullopt;
	}
	return static_cast<int>(lastOctet - 1);
}

std::vector<std::uint8_t> EncodeUdpFrame(
	std::uint16_t sourcePort,
	std::uint16_t destinationPort,
	const std::uint8_t *payload,
	std::size_t payloadSize)
{
	std::vector<std::uint8_t> frame(UDP_FRAME_HEADER_SIZE + payloadSize);
	frame[0] = static_cast<std::uint8_t>(sourcePort & 0xFFU);
	frame[1] = static_cast<std::uint8_t>((sourcePort >> 8U) & 0xFFU);
	frame[2] = static_cast<std::uint8_t>(destinationPort & 0xFFU);
	frame[3] = static_cast<std::uint8_t>((destinationPort >> 8U) & 0xFFU);
	if (payloadSize > 0 && payload != nullptr)
	{
		std::copy_n(payload, payloadSize, frame.begin() + UDP_FRAME_HEADER_SIZE);
	}
	return frame;
}

std::optional<DecodedUdpFrame> DecodeUdpFrame(const std::uint8_t *frame, std::size_t frameSize)
{
	if (frame == nullptr || frameSize < UDP_FRAME_HEADER_SIZE ||
		frameSize > UDP_FRAME_HEADER_SIZE + std::numeric_limits<std::uint16_t>::max())
	{
		return std::nullopt;
	}

	DecodedUdpFrame decoded;
	decoded.sourcePort = static_cast<std::uint16_t>(frame[0] | (static_cast<std::uint16_t>(frame[1]) << 8U));
	decoded.destinationPort = static_cast<std::uint16_t>(frame[2] | (static_cast<std::uint16_t>(frame[3]) << 8U));
	decoded.payload.assign(frame + UDP_FRAME_HEADER_SIZE, frame + frameSize);
	return decoded;
}

std::uint16_t BoundedDatagramInbox::Bind(std::uint16_t port)
{
	if (port == 0)
	{
		port = allocateEphemeralPort();
	}
	if (port != 0)
	{
		++m_bindCounts[port];
		m_packets.try_emplace(port);
	}
	return port;
}

void BoundedDatagramInbox::Unbind(std::uint16_t port)
{
	const auto found = m_bindCounts.find(port);
	if (found == m_bindCounts.end())
	{
		return;
	}
	if (--found->second == 0)
	{
		m_bindCounts.erase(found);
		m_packets.erase(port);
	}
}

bool BoundedDatagramInbox::Push(int sourceSlot, const std::uint8_t *frame, std::size_t frameSize)
{
	const std::optional<DecodedUdpFrame> decoded = DecodeUdpFrame(frame, frameSize);
	if (!decoded)
	{
		return false;
	}
	const auto found = m_packets.find(decoded->destinationPort);
	if (found == m_packets.end() || found->second.size() >= MAX_INBOX_PACKETS)
	{
		return false;
	}
	const std::uint32_t address = SlotToSyntheticAddress(sourceSlot);
	if (address == 0)
	{
		return false;
	}
	found->second.push_back({ address, decoded->sourcePort, decoded->payload });
	return true;
}

std::optional<Datagram> BoundedDatagramInbox::Pop(std::uint16_t port)
{
	const auto found = m_packets.find(port);
	if (found == m_packets.end() || found->second.empty())
	{
		return std::nullopt;
	}
	Datagram datagram = std::move(found->second.front());
	found->second.pop_front();
	return datagram;
}

void BoundedDatagramInbox::RemoveSourceSlot(int slot)
{
	const std::uint32_t address = SlotToSyntheticAddress(slot);
	for (auto &[port, packets] : m_packets)
	{
		(void)port;
		std::erase_if(packets, [address](const Datagram &datagram) {
			return datagram.sourceAddress == address;
		});
	}
}

std::size_t BoundedDatagramInbox::Size(std::uint16_t port) const
{
	const auto found = m_packets.find(port);
	return found == m_packets.end() ? 0 : found->second.size();
}

std::uint16_t BoundedDatagramInbox::allocateEphemeralPort() const
{
	for (std::uint32_t port = FIRST_EPHEMERAL_PORT; port <= LAST_EPHEMERAL_PORT; ++port)
	{
		if (!m_bindCounts.contains(static_cast<std::uint16_t>(port)))
		{
			return static_cast<std::uint16_t>(port);
		}
	}
	return 0;
}

std::optional<ServerMessage> ParseServerMessage(const std::string &raw, std::string *error)
{
	if (raw.empty() || raw.size() > MAX_SIGNALING_MESSAGE_BYTES)
	{
		if (error != nullptr)
		{
			*error = "signaling message is empty or exceeds the size limit";
		}
		return std::nullopt;
	}

	const Json input = Json::parse(raw, nullptr, false);
	if (input.is_discarded() || !input.is_object() || !input.contains("type") || !input["type"].is_string())
	{
		if (error != nullptr)
		{
			*error = "signaling message is not a typed JSON object";
		}
		return std::nullopt;
	}

	ServerMessage message;
	const std::string type = input["type"].get<std::string>();
	if (type == "welcome")
	{
		if (!input.contains("roomId") || !input["roomId"].is_string() ||
			!input.contains("slot") || !input["slot"].is_number_integer() ||
			!input.contains("capacity") || !input["capacity"].is_number_integer() ||
			!input.contains("roster"))
		{
			if (error != nullptr)
			{
				*error = "welcome message is malformed";
			}
			return std::nullopt;
		}
		message.type = ServerMessageType::Welcome;
		message.roomId = input["roomId"].get<std::string>();
		message.slot = input["slot"].get<int>();
		message.capacity = input["capacity"].get<int>();
		if (!std::regex_match(message.roomId, ROOM_ID_PATTERN) ||
			!IsValidSlot(message.slot) ||
			message.capacity < MIN_ROOM_CAPACITY || message.capacity > MAX_ROOM_CAPACITY ||
			message.slot >= message.capacity ||
			!ReadRoster(input["roster"], &message.roster, error))
		{
			return std::nullopt;
		}
	}
	else if (type == "roster")
	{
		message.type = ServerMessageType::Roster;
		if (!input.contains("roster") || !ReadRoster(input["roster"], &message.roster, error))
		{
			return std::nullopt;
		}
	}
	else if (type == "offer" || type == "answer")
	{
		if (!input.contains("from") || !input["from"].is_number_integer() ||
			!input.contains("payload") || !input["payload"].is_object())
		{
			if (error != nullptr)
			{
				*error = "description message is malformed";
			}
			return std::nullopt;
		}
		const Json &payload = input["payload"];
		if (!payload.contains("type") || !payload["type"].is_string() ||
			!payload.contains("sdp") || !payload["sdp"].is_string() ||
			payload["type"].get<std::string>() != type ||
			payload["sdp"].get_ref<const std::string &>().size() > MAX_SDP_LENGTH)
		{
			if (error != nullptr)
			{
				*error = "description payload is malformed";
			}
			return std::nullopt;
		}
		message.type = type == "offer" ? ServerMessageType::Offer : ServerMessageType::Answer;
		message.from = input["from"].get<int>();
		message.sdp = payload["sdp"].get<std::string>();
		if (!IsValidSlot(message.from))
		{
			if (error != nullptr)
			{
				*error = "description sender slot is invalid";
			}
			return std::nullopt;
		}
	}
	else if (type == "ice")
	{
		if (!input.contains("from") || !input["from"].is_number_integer() ||
			!input.contains("payload") || !input["payload"].is_object())
		{
			if (error != nullptr)
			{
				*error = "ICE message is malformed";
			}
			return std::nullopt;
		}
		const Json &payload = input["payload"];
		if (!payload.contains("candidate") || !payload["candidate"].is_string() ||
			!payload.contains("sdpMid") || !payload["sdpMid"].is_string() ||
			payload["candidate"].get_ref<const std::string &>().size() > 4096 ||
			payload["sdpMid"].get_ref<const std::string &>().size() > 256)
		{
			if (error != nullptr)
			{
				*error = "ICE candidate payload is malformed";
			}
			return std::nullopt;
		}
		message.type = ServerMessageType::Ice;
		message.from = input["from"].get<int>();
		message.candidate = payload["candidate"].get<std::string>();
		message.mid = payload["sdpMid"].get<std::string>();
		if (!IsValidSlot(message.from))
		{
			if (error != nullptr)
			{
				*error = "ICE sender slot is invalid";
			}
			return std::nullopt;
		}
	}
	else if (type == "peer-left")
	{
		if (!input.contains("slot") || !input["slot"].is_number_integer())
		{
			if (error != nullptr)
			{
				*error = "peer-left message is malformed";
			}
			return std::nullopt;
		}
		message.type = ServerMessageType::PeerLeft;
		message.slot = input["slot"].get<int>();
		if (!IsValidSlot(message.slot))
		{
			if (error != nullptr)
			{
				*error = "departed peer slot is invalid";
			}
			return std::nullopt;
		}
	}
	else if (type == "error")
	{
		if (!input.contains("code") || !input["code"].is_string() ||
			!input.contains("message") || !input["message"].is_string() ||
			input["code"].get_ref<const std::string &>().size() > 64 ||
			input["message"].get_ref<const std::string &>().size() > 1024)
		{
			if (error != nullptr)
			{
				*error = "error message is malformed";
			}
			return std::nullopt;
		}
		message.type = ServerMessageType::Error;
		message.errorCode = input["code"].get<std::string>();
		message.errorMessage = input["message"].get<std::string>();
	}
	else
	{
		if (error != nullptr)
		{
			*error = "unknown signaling message type";
		}
		return std::nullopt;
	}
	return message;
}

std::string BuildJoinMessage(const RuntimeConfig &config)
{
	return Json({
		{ "type", "join" },
		{ "roomId", config.roomId },
		{ "name", config.playerName },
		{ "capacity", config.capacity },
	}).dump();
}

std::string BuildDescriptionMessage(int to, const std::string &type, const std::string &sdp)
{
	return Json({
		{ "type", type },
		{ "to", to },
		{ "payload", {
			{ "type", type },
			{ "sdp", sdp },
		} },
	}).dump();
}

std::string BuildCandidateMessage(int to, const std::string &candidate, const std::string &mid)
{
	return Json({
		{ "type", "ice" },
		{ "to", to },
		{ "payload", {
			{ "candidate", candidate },
			{ "sdpMid", mid },
		} },
	}).dump();
}

std::string BuildLeaveMessage()
{
	return R"({"type":"leave"})";
}

std::optional<std::vector<IceServerSpec>> ParseIceServersResponse(
	const std::string &raw,
	bool requireEnvelope,
	std::string *error)
{
	if (raw.empty() || raw.size() > 64 * 1024)
	{
		if (error != nullptr)
		{
			*error = "ICE server response is empty or exceeds the size limit";
		}
		return std::nullopt;
	}
	const Json input = Json::parse(raw, nullptr, false);
	if (input.is_discarded())
	{
		if (error != nullptr)
		{
			*error = "ICE server response is invalid JSON";
		}
		return std::nullopt;
	}
	if (requireEnvelope)
	{
		if (!input.is_object() || !input.contains("iceServers"))
		{
			if (error != nullptr)
			{
				*error = "TURN response is missing iceServers";
			}
			return std::nullopt;
		}
		return ParseIceServersArray(input["iceServers"], error);
	}
	return ParseIceServersArray(input, error);
}

std::uint64_t LifecycleTracker::Start(bool enabled)
{
	++m_generation;
	m_state = enabled ? LifecycleState::Starting : LifecycleState::Disabled;
	return m_generation;
}

bool LifecycleTracker::Transition(std::uint64_t generation, LifecycleState state)
{
	if (generation != m_generation || !canTransition(state))
	{
		return false;
	}
	m_state = state;
	return true;
}

LifecycleState LifecycleTracker::State() const
{
	return m_state;
}

std::uint64_t LifecycleTracker::Generation() const
{
	return m_generation;
}

bool LifecycleTracker::IsTerminal() const
{
	return m_state == LifecycleState::Disabled ||
		m_state == LifecycleState::Failed ||
		m_state == LifecycleState::Stopped;
}

bool LifecycleTracker::canTransition(LifecycleState state) const
{
	if (state == LifecycleState::Failed || state == LifecycleState::Stopping)
	{
		return m_state != LifecycleState::Disabled && m_state != LifecycleState::Stopped;
	}
	switch (m_state)
	{
		case LifecycleState::Starting:
			return state == LifecycleState::FetchingTurn || state == LifecycleState::Signaling;
		case LifecycleState::FetchingTurn:
			return state == LifecycleState::Signaling;
		case LifecycleState::Signaling:
			return state == LifecycleState::Joined;
		case LifecycleState::Joined:
			return state == LifecycleState::Connected;
		case LifecycleState::Connected:
			return state == LifecycleState::Joined;
		case LifecycleState::Stopping:
			return state == LifecycleState::Stopped;
		default:
			return false;
	}
}

} // namespace GeneralsX::NativeWebRTC
