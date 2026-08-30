/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
*/

#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

// GeneralsX @feature Copilot 30/08/2026 Define the native WebRTC room protocol,
// synthetic addressing, UDP framing, bounded inbox, and lifecycle primitives.
namespace GeneralsX::NativeWebRTC
{

constexpr std::uint32_t BROADCAST_ADDRESS = 0xFFFFFFFFU;
constexpr std::size_t UDP_FRAME_HEADER_SIZE = 4;
constexpr std::size_t MAX_SIGNALING_MESSAGE_BYTES = 16 * 1024;
constexpr std::size_t MAX_SDP_LENGTH = 12 * 1024;
constexpr std::size_t MAX_INBOX_PACKETS = 1024;
constexpr std::size_t MAX_BUFFERED_BYTES = 1024 * 1024;
constexpr unsigned int MAX_RETRANSMITS = 5;
constexpr int MIN_ROOM_CAPACITY = 2;
constexpr int MAX_ROOM_CAPACITY = 8;

struct RuntimeConfig
{
	bool enabled = false;
	bool fetchTurnCredentials = true;
	std::string signalingUrl = "https://signaling.generalsx.org";
	std::string roomId = "LAN1";
	std::string playerName = "native";
	int capacity = 4;
	std::string iceServersJson;
};

RuntimeConfig ParseRuntimeConfig(
	const std::vector<std::string> &arguments,
	const std::unordered_map<std::string, std::string> &environment);
bool ValidateRuntimeConfig(const RuntimeConfig &config, std::string *error);
std::string BuildRoomWebSocketUrl(const RuntimeConfig &config);
std::string BuildTurnCredentialsUrl(const RuntimeConfig &config);

std::uint32_t SlotToSyntheticAddress(int slot);
std::optional<int> SyntheticAddressToSlot(std::uint32_t address);

struct DecodedUdpFrame
{
	std::uint16_t sourcePort = 0;
	std::uint16_t destinationPort = 0;
	std::vector<std::uint8_t> payload;
};

std::vector<std::uint8_t> EncodeUdpFrame(
	std::uint16_t sourcePort,
	std::uint16_t destinationPort,
	const std::uint8_t *payload,
	std::size_t payloadSize);
std::optional<DecodedUdpFrame> DecodeUdpFrame(const std::uint8_t *frame, std::size_t frameSize);

struct Datagram
{
	std::uint32_t sourceAddress = 0;
	std::uint16_t sourcePort = 0;
	std::vector<std::uint8_t> payload;
};

class BoundedDatagramInbox
{
public:
	std::uint16_t Bind(std::uint16_t port);
	void Unbind(std::uint16_t port);
	bool Push(int sourceSlot, const std::uint8_t *frame, std::size_t frameSize);
	std::optional<Datagram> Pop(std::uint16_t port);
	void RemoveSourceSlot(int slot);
	std::size_t Size(std::uint16_t port) const;

private:
	std::uint16_t allocateEphemeralPort() const;

	std::map<std::uint16_t, std::size_t> m_bindCounts;
	std::map<std::uint16_t, std::deque<Datagram>> m_packets;
};

struct RosterEntry
{
	int slot = -1;
	std::string name;
	bool isHost = false;
};

enum class ServerMessageType
{
	Welcome,
	Roster,
	Offer,
	Answer,
	Ice,
	PeerLeft,
	Error,
};

struct ServerMessage
{
	ServerMessageType type = ServerMessageType::Error;
	std::string roomId;
	int slot = -1;
	int capacity = 0;
	std::vector<RosterEntry> roster;
	int from = -1;
	std::string sdp;
	std::string candidate;
	std::string mid;
	std::string errorCode;
	std::string errorMessage;
};

std::optional<ServerMessage> ParseServerMessage(const std::string &raw, std::string *error);
std::string BuildJoinMessage(const RuntimeConfig &config);
std::string BuildDescriptionMessage(int to, const std::string &type, const std::string &sdp);
std::string BuildCandidateMessage(int to, const std::string &candidate, const std::string &mid);
std::string BuildLeaveMessage();

struct IceServerSpec
{
	std::vector<std::string> urls;
	std::string username;
	std::string credential;
};

std::optional<std::vector<IceServerSpec>> ParseIceServersResponse(
	const std::string &raw,
	bool requireEnvelope,
	std::string *error);

enum class LifecycleState
{
	Disabled,
	Starting,
	FetchingTurn,
	Signaling,
	Joined,
	Connected,
	Failed,
	Stopping,
	Stopped,
};

class LifecycleTracker
{
public:
	std::uint64_t Start(bool enabled);
	bool Transition(std::uint64_t generation, LifecycleState state);
	LifecycleState State() const;
	std::uint64_t Generation() const;
	bool IsTerminal() const;

private:
	bool canTransition(LifecycleState state) const;

	std::uint64_t m_generation = 0;
	LifecycleState m_state = LifecycleState::Disabled;
};

} // namespace GeneralsX::NativeWebRTC
