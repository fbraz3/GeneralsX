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
#ifdef NATIVE_WEBRTC_LINK_TEST
#include "NativeWebRTCTransport.h"
#endif

#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include <nlohmann/json.hpp>

// GeneralsX @feature Copilot 30/08/2026 Verify protocol compatibility,
// little-endian framing, addressing, bounded queues, and lifecycle behavior.
namespace
{

using namespace GeneralsX::NativeWebRTC;
using Json = nlohmann::json;

int g_failures = 0;

void Check(bool condition, const char *expression, int line)
{
	if (!condition)
	{
		std::cerr << "line " << line << ": check failed: " << expression << '\n';
		++g_failures;
	}
}

#define CHECK(expression) Check((expression), #expression, __LINE__)

void TestRuntimeConfig()
{
	const RuntimeConfig defaults = ParseRuntimeConfig({ "GeneralsXZH" }, {});
	CHECK(!defaults.enabled);
	CHECK(defaults.roomId == "LAN1");
	CHECK(defaults.capacity == 4);

	const RuntimeConfig environment = ParseRuntimeConfig(
		{ "GeneralsXZH" },
		{
			{ "GENERALSX_WEBRTC", "true" },
			{ "GENERALSX_WEBRTC_SIGNALING_URL", "http://127.0.0.1:8787/path" },
			{ "GENERALSX_WEBRTC_ROOM", "ab12" },
			{ "GENERALSX_WEBRTC_PLAYER_NAME", "Native Player" },
			{ "GENERALSX_WEBRTC_CAPACITY", "8" },
			{ "GENERALSX_WEBRTC_DISABLE_TURN", "1" },
		});
	CHECK(environment.enabled);
	CHECK(environment.roomId == "AB12");
	CHECK(environment.playerName == "Native Player");
	CHECK(environment.capacity == 8);
	CHECK(!environment.fetchTurnCredentials);
	CHECK(BuildRoomWebSocketUrl(environment) == "ws://127.0.0.1:8787/room?roomId=AB12&capacity=8");
	CHECK(BuildTurnCredentialsUrl(environment) == "http://127.0.0.1:8787/turn-credentials");

	const RuntimeConfig cli = ParseRuntimeConfig(
		{
			"GeneralsXZH",
			"--webrtc",
			"--webrtc-room", "ROOM7",
			"--webrtc-name", "CLI Player",
			"--webrtc-capacity", "6",
			"--webrtc-signaling", "https://signal.example",
			"--webrtc-no-turn",
		},
		{});
	CHECK(cli.enabled);
	CHECK(cli.roomId == "ROOM7");
	CHECK(cli.playerName == "CLI Player");
	CHECK(cli.capacity == 6);
	CHECK(!cli.fetchTurnCredentials);
	CHECK(BuildRoomWebSocketUrl(cli) == "wss://signal.example/room?roomId=ROOM7&capacity=6");

	std::string error;
	RuntimeConfig invalid = cli;
	invalid.roomId = "bad!";
	CHECK(!ValidateRuntimeConfig(invalid, &error));
	invalid = cli;
	invalid.signalingUrl = "https://user:secret@signal.example";
	CHECK(!ValidateRuntimeConfig(invalid, &error));
}

void TestAddressing()
{
	CHECK(SlotToSyntheticAddress(0) == 0x0A000001U);
	CHECK(SlotToSyntheticAddress(7) == 0x0A000008U);
	CHECK(SlotToSyntheticAddress(-1) == 0);
	CHECK(SyntheticAddressToSlot(0x0A000001U) == 0);
	CHECK(SyntheticAddressToSlot(0x0A000008U) == 7);
	CHECK(!SyntheticAddressToSlot(0xC0A80101U));
	CHECK(!SyntheticAddressToSlot(0x0A000000U));
}

void TestFramingAndInbox()
{
	const std::uint8_t payload[] = { 0xAA, 0xBB, 0xCC };
	const std::vector<std::uint8_t> frame = EncodeUdpFrame(0x1234, 0xABCD, payload, sizeof(payload));
	CHECK(frame == std::vector<std::uint8_t>({ 0x34, 0x12, 0xCD, 0xAB, 0xAA, 0xBB, 0xCC }));

	const auto decoded = DecodeUdpFrame(frame.data(), frame.size());
	CHECK(decoded.has_value());
	CHECK(decoded->sourcePort == 0x1234);
	CHECK(decoded->destinationPort == 0xABCD);
	CHECK(decoded->payload == std::vector<std::uint8_t>({ 0xAA, 0xBB, 0xCC }));
	CHECK(!DecodeUdpFrame(frame.data(), 3));

	BoundedDatagramInbox inbox;
	CHECK(inbox.Bind(0xABCD) == 0xABCD);
	CHECK(inbox.Push(2, frame.data(), frame.size()));
	CHECK(inbox.Size(0xABCD) == 1);
	const auto datagram = inbox.Pop(0xABCD);
	CHECK(datagram.has_value());
	CHECK(datagram->sourceAddress == 0x0A000003U);
	CHECK(datagram->sourcePort == 0x1234);
	CHECK(datagram->payload == std::vector<std::uint8_t>({ 0xAA, 0xBB, 0xCC }));

	for (std::size_t index = 0; index < MAX_INBOX_PACKETS; ++index)
	{
		CHECK(inbox.Push(1, frame.data(), frame.size()));
	}
	CHECK(!inbox.Push(1, frame.data(), frame.size()));
	inbox.RemoveSourceSlot(1);
	CHECK(inbox.Size(0xABCD) == 0);
	inbox.Unbind(0xABCD);
	CHECK(!inbox.Push(1, frame.data(), frame.size()));

	const std::uint16_t ephemeral = inbox.Bind(0);
	CHECK(ephemeral >= 49152);
	inbox.Unbind(ephemeral);
}

void TestSignalingProtocol()
{
	RuntimeConfig config;
	config.enabled = true;
	config.roomId = "ROOM7";
	config.playerName = "Native";
	config.capacity = 4;

	const Json join = Json::parse(BuildJoinMessage(config));
	CHECK(join["type"] == "join");
	CHECK(join["roomId"] == "ROOM7");
	CHECK(join["name"] == "Native");
	CHECK(join["capacity"] == 4);

	const std::string welcome = R"({
		"type":"welcome",
		"roomId":"ROOM7",
		"slot":1,
		"capacity":4,
		"roster":[
			{"slot":0,"name":"Browser","isHost":true},
			{"slot":1,"name":"Native","isHost":false}
		]
	})";
	std::string error;
	const auto parsedWelcome = ParseServerMessage(welcome, &error);
	CHECK(parsedWelcome.has_value());
	CHECK(parsedWelcome->type == ServerMessageType::Welcome);
	CHECK(parsedWelcome->slot == 1);
	CHECK(parsedWelcome->roster.size() == 2);

	const Json offer = Json::parse(BuildDescriptionMessage(0, "offer", "v=0\r\n"));
	CHECK(offer["type"] == "offer");
	CHECK(offer["to"] == 0);
	CHECK(offer["payload"]["type"] == "offer");
	CHECK(offer["payload"]["sdp"] == "v=0\r\n");

	const Json candidate = Json::parse(BuildCandidateMessage(0, "candidate:1", "0"));
	CHECK(candidate["type"] == "ice");
	CHECK(candidate["payload"]["candidate"] == "candidate:1");
	CHECK(candidate["payload"]["sdpMid"] == "0");

	CHECK(!ParseServerMessage(R"({"type":"welcome","slot":99})", &error));
	CHECK(!ParseServerMessage(R"({"type":"ice","from":0,"payload":{"candidate":4,"sdpMid":"0"}})", &error));
	CHECK(!ParseServerMessage(std::string(MAX_SIGNALING_MESSAGE_BYTES + 1, 'x'), &error));

	const std::string turn = R"({
		"iceServers":[
			{"urls":["stun:stun.example:3478"]},
			{"urls":"turn:turn.example:3478?transport=udp","username":"short","credential":"lived"}
		],
		"ttlSeconds":600
	})";
	const auto iceServers = ParseIceServersResponse(turn, true, &error);
	CHECK(iceServers.has_value());
	CHECK(iceServers->size() == 2);
	CHECK((*iceServers)[1].username == "short");
	CHECK((*iceServers)[1].credential == "lived");
	CHECK(!ParseIceServersResponse(R"({"iceServers":[{"urls":"https://bad.example"}]})", true, &error));
}

void TestLifecycle()
{
	LifecycleTracker lifecycle;
	const std::uint64_t first = lifecycle.Start(true);
	CHECK(lifecycle.State() == LifecycleState::Starting);
	CHECK(lifecycle.Transition(first, LifecycleState::FetchingTurn));
	CHECK(lifecycle.Transition(first, LifecycleState::Signaling));
	CHECK(lifecycle.Transition(first, LifecycleState::Joined));
	CHECK(lifecycle.Transition(first, LifecycleState::Connected));
	CHECK(lifecycle.Transition(first, LifecycleState::Joined));
	CHECK(!lifecycle.Transition(first - 1, LifecycleState::Failed));
	CHECK(lifecycle.Transition(first, LifecycleState::Failed));
	CHECK(lifecycle.IsTerminal());

	const std::uint64_t second = lifecycle.Start(true);
	CHECK(second == first + 1);
	CHECK(!lifecycle.Transition(first, LifecycleState::Signaling));
	CHECK(lifecycle.Transition(second, LifecycleState::Signaling));
	CHECK(lifecycle.Transition(second, LifecycleState::Stopping));
	CHECK(lifecycle.Transition(second, LifecycleState::Stopped));
	CHECK(lifecycle.IsTerminal());

	lifecycle.Start(false);
	CHECK(lifecycle.State() == LifecycleState::Disabled);
	CHECK(lifecycle.IsTerminal());
}

#ifdef NATIVE_WEBRTC_LINK_TEST
void TestTransportLinkage()
{
	CHECK(!NativeWebRTCTransport::Instance().IsEnabled());
}
#endif

} // namespace

#ifdef NATIVE_WEBRTC_LINK_TEST
int __argc = 0;
char **__argv = nullptr;
#endif

int main()
{
	TestRuntimeConfig();
	TestAddressing();
	TestFramingAndInbox();
	TestSignalingProtocol();
	TestLifecycle();
#ifdef NATIVE_WEBRTC_LINK_TEST
	TestTransportLinkage();
#endif
	if (g_failures != 0)
	{
		std::cerr << g_failures << " native WebRTC protocol test(s) failed\n";
		return EXIT_FAILURE;
	}
	std::cout << "native WebRTC protocol tests passed\n";
	return EXIT_SUCCESS;
}
