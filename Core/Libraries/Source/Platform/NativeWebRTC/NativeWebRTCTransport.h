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
#include <memory>
#include <string>

// GeneralsX @feature Copilot 30/08/2026 Expose the opt-in native WebRTC
// datagram service to the legacy UDP wrapper without leaking WebRTC types.
namespace GeneralsX::NativeWebRTC
{

struct CompatibilityProfile;

struct ReceivedDatagram
{
	std::uint32_t sourceAddress = 0;
	std::uint16_t sourcePort = 0;
	std::size_t size = 0;
};

class NativeWebRTCTransport
{
public:
	static NativeWebRTCTransport &Instance();

	NativeWebRTCTransport(const NativeWebRTCTransport &) = delete;
	NativeWebRTCTransport &operator=(const NativeWebRTCTransport &) = delete;

	void ConfigureFromProcess(CompatibilityProfile compatibility);
	bool IsEnabled() const;
	bool HasFailed() const;
	std::string LastError() const;

	std::uint16_t Bind(std::uint16_t port);
	void Unbind(std::uint16_t port);
	bool Send(
		std::uint32_t destinationAddress,
		std::uint16_t destinationPort,
		std::uint16_t sourcePort,
		const std::uint8_t *payload,
		std::size_t payloadSize);
	int Receive(
		std::uint16_t destinationPort,
		std::uint8_t *payload,
		std::size_t payloadCapacity,
		ReceivedDatagram *received);

	std::uint32_t LocalAddress() const;
	std::uint32_t WaitForLocalAddress(unsigned int timeoutMilliseconds) const;

private:
	NativeWebRTCTransport();
	~NativeWebRTCTransport();

	class Impl;
	std::shared_ptr<Impl> m_impl;
};

} // namespace GeneralsX::NativeWebRTC
