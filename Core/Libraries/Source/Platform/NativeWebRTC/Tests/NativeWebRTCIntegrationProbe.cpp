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
#include "NativeWebRTCTransport.h"

#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <thread>

// GeneralsX @feature Copilot 30/08/2026 Exercise the real native transport
// against a browser peer without bypassing the legacy UDP-facing API.
int __argc = 0;
char **__argv = nullptr;

namespace
{

constexpr std::uint16_t BROWSER_PORT = 40000;
constexpr std::uint16_t NATIVE_PORT = 40001;
constexpr std::array<std::uint8_t, 4> BROWSER_PAYLOAD = { 0x42, 0x52, 0x57, 0x53 };
constexpr std::array<std::uint8_t, 4> NATIVE_PAYLOAD = { 0x4E, 0x41, 0x54, 0x56 };
constexpr auto TIMEOUT = std::chrono::seconds(45);

std::string FormatAddress(std::uint32_t address)
{
	return std::to_string((address >> 24U) & 0xFFU) + "." +
		std::to_string((address >> 16U) & 0xFFU) + "." +
		std::to_string((address >> 8U) & 0xFFU) + "." +
		std::to_string(address & 0xFFU);
}

std::string FormatHeader(std::uint16_t sourcePort, std::uint16_t destinationPort)
{
	const std::array<unsigned int, 4> bytes = {
		sourcePort & 0xFFU,
		(sourcePort >> 8U) & 0xFFU,
		destinationPort & 0xFFU,
		(destinationPort >> 8U) & 0xFFU,
	};
	return std::to_string(bytes[0]) + "," + std::to_string(bytes[1]) + "," +
		std::to_string(bytes[2]) + "," + std::to_string(bytes[3]);
}

} // namespace

int main(int argc, char **argv)
{
	__argc = argc;
	__argv = argv;

	auto &transport = GeneralsX::NativeWebRTC::NativeWebRTCTransport::Instance();
	transport.ConfigureFromProcess(GeneralsX::NativeWebRTC::CurrentBuildCompatibilityProfile());
	if (!transport.IsEnabled())
	{
		std::cerr << "native WebRTC integration probe requires -webrtc\n";
		return EXIT_FAILURE;
	}

	const std::uint16_t boundPort = transport.Bind(NATIVE_PORT);
	if (boundPort != NATIVE_PORT)
	{
		std::cerr << "failed to bind native probe port " << NATIVE_PORT << '\n';
		return EXIT_FAILURE;
	}

	const std::uint32_t localAddress = transport.WaitForLocalAddress(15000);
	if (localAddress == 0)
	{
		std::cerr << "native transport did not join the signaling room: " << transport.LastError() << '\n';
		return EXIT_FAILURE;
	}
	std::cout << "NATIVE_READY address=" << FormatAddress(localAddress) << " port=" << NATIVE_PORT << '\n';
	std::cout.flush();

	const auto deadline = std::chrono::steady_clock::now() + TIMEOUT;
	std::array<std::uint8_t, 64> payload = {};
	while (std::chrono::steady_clock::now() < deadline)
	{
		GeneralsX::NativeWebRTC::ReceivedDatagram received;
		const int size = transport.Receive(NATIVE_PORT, payload.data(), payload.size(), &received);
		if (size < 0)
		{
			std::cerr << "native transport failed while receiving: " << transport.LastError() << '\n';
			return EXIT_FAILURE;
		}
		if (size > 0)
		{
			if (received.sourcePort != BROWSER_PORT ||
				size != static_cast<int>(BROWSER_PAYLOAD.size()) ||
				std::memcmp(payload.data(), BROWSER_PAYLOAD.data(), BROWSER_PAYLOAD.size()) != 0)
			{
				std::cerr << "received an unexpected browser datagram\n";
				return EXIT_FAILURE;
			}
			if (!transport.Send(
					received.sourceAddress,
					BROWSER_PORT,
					NATIVE_PORT,
					NATIVE_PAYLOAD.data(),
					NATIVE_PAYLOAD.size()))
			{
				std::cerr << "native reply was not accepted by an open DataChannel\n";
				return EXIT_FAILURE;
			}
			std::cout << "INTEROP_OK local=" << FormatAddress(localAddress)
					  << " remote=" << FormatAddress(received.sourceAddress)
					  << " rxHeader=" << FormatHeader(BROWSER_PORT, NATIVE_PORT)
					  << " txHeader=" << FormatHeader(NATIVE_PORT, BROWSER_PORT) << '\n';
			return EXIT_SUCCESS;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	std::cerr << "timed out waiting for the browser datagram\n";
	return EXIT_FAILURE;
}
