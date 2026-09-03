// GeneralsX @feature GeneralsOnline NextGenTransport implementation
#include "PreRTS.h"

#include "Common/crc.h"
#include "GameNetwork/NetworkInterface.h"
#include "GameNetwork/GeneralsOnline/NextGenTransport.h"
#include "GameNetwork/GeneralsOnline/NGMPGame.h"
#include "GameNetwork/GeneralsOnline/OnlineServices_Manager.h"
#include "GameNetwork/GeneralsOnline/NetworkMesh.h"
#include <cstring>
#include <cstdio>

NextGenTransport::NextGenTransport()
{
	m_statisticsSlot = 0;
	m_lastSecond = timeGetTime();
	m_useLatency = FALSE;
	m_usePacketLoss = FALSE;
	reset();
}

NextGenTransport::~NextGenTransport()
{
	reset();
}

Bool NextGenTransport::init(AsciiString, UnsignedShort)
{
	return TRUE;
}

Bool NextGenTransport::init(UnsignedInt, UnsignedShort)
{
	return TRUE;
}

void NextGenTransport::reset(void)
{
	std::memset(m_inBuffer, 0, sizeof(m_inBuffer));
	std::memset(m_outBuffer, 0, sizeof(m_outBuffer));
	std::memset(m_incomingPackets, 0, sizeof(m_incomingPackets));
	std::memset(m_incomingBytes, 0, sizeof(m_incomingBytes));
	std::memset(m_outgoingPackets, 0, sizeof(m_outgoingPackets));
	std::memset(m_outgoingBytes, 0, sizeof(m_outgoingBytes));
	std::memset(m_unknownPackets, 0, sizeof(m_unknownPackets));
	std::memset(m_unknownBytes, 0, sizeof(m_unknownBytes));

	for (int i = 0; i < MAX_MESSAGES; ++i)
	{
		m_outPacketState[i].retryCount = 0;
		m_inBufferOccupied[i] = false;
	}
}

Bool NextGenTransport::update(void)
{
	Bool retval = TRUE;
	if (doRecv() == FALSE)
	{
		retval = FALSE;
	}
	if (doSend() == FALSE)
	{
		retval = FALSE;
	}
	return retval;
}

Bool NextGenTransport::doRecv(void)
{
	bool bRet = FALSE;
	int numRead = 0;

	UnsignedInt now = timeGetTime();
	if (m_lastSecond + 1000 < now)
	{
		m_lastSecond = now;
		m_statisticsSlot = (m_statisticsSlot + 1) % MAX_TRANSPORT_STATISTICS_SECONDS;
		m_outgoingPackets[m_statisticsSlot] = 0;
		m_outgoingBytes[m_statisticsSlot] = 0;
		m_incomingPackets[m_statisticsSlot] = 0;
		m_incomingBytes[m_statisticsSlot] = 0;
		m_unknownPackets[m_statisticsSlot] = 0;
		m_unknownBytes[m_statisticsSlot] = 0;
	}

	NetworkMesh* pMesh = NGMP_OnlineServicesManager::GetNetworkMesh();
	if (!pMesh)
	{
		return FALSE;
	}

	auto& connections = pMesh->GetAllConnections();
	for (auto& kvPair : connections)
	{
		SteamNetworkingMessage_t* pMsg[255] = { nullptr };
		int numPackets = kvPair.second.Recv(pMsg);
		if (numPackets <= 0)
			continue;

		for (int iPacket = 0; iPacket < numPackets; ++iPacket)
		{
			SteamNetworkingMessage_t* msg = pMsg[iPacket];
			if (!msg)
				continue;

			const uint32_t numBytesWithHeader = msg->m_cbSize;
			// Packets have 1 byte for channel prefix + TransportMessageHeader + payload
			if (numBytesWithHeader < (1 + sizeof(TransportMessageHeader)))
			{
				msg->Release();
				continue;
			}

			const uint8_t* rawData = static_cast<const uint8_t*>(msg->GetData());
			uint8_t channel = rawData[0];
			if (channel != 0) // 0 == ENetworkChannels::Game / NETWORK_CHANNEL_GAME
			{
				msg->Release();
				continue;
			}

			TransportMessage incomingMessage{};
			std::memset(&incomingMessage, 0, sizeof(incomingMessage));

			const uint8_t* pGameData = rawData + 1;
			std::memcpy(&incomingMessage.header, pGameData, sizeof(TransportMessageHeader));

			const uint32_t payloadLen = numBytesWithHeader - 1 - static_cast<uint32_t>(sizeof(TransportMessageHeader));
			const size_t dstCap = sizeof(incomingMessage.data);
			const size_t toCopy = (payloadLen <= dstCap) ? payloadLen : dstCap;

			if (payloadLen > 0)
			{
				std::memcpy(incomingMessage.data, pGameData + sizeof(TransportMessageHeader), toCopy);
				incomingMessage.length = static_cast<Int>(toCopy);
			}

			msg->Release();

			if (!isGeneralsPacket(&incomingMessage))
			{
				fprintf(stderr, "[NGMP-TRANSPORT] Dropping invalid generals packet: magic=0x%X (expected 0x%X), crc=0x%X, len=%d\n",
					incomingMessage.header.magic, GENERALS_MAGIC_NUMBER, incomingMessage.header.crc, incomingMessage.length);
				fflush(stderr);
				m_unknownPackets[m_statisticsSlot]++;
				m_unknownBytes[m_statisticsSlot] += numBytesWithHeader;
				continue;
			}

			bool bSlotFound = false;
			if (TheNGMPGame != nullptr)
			{
				for (int s = 0; s < MAX_SLOTS; ++s)
				{
					NGMPGameSlot* sSlot = TheNGMPGame->getGameSpySlot(s);
					if (sSlot && sSlot->m_userID == kvPair.first)
					{
						incomingMessage.addr = static_cast<UnsignedInt>(s);
						bSlotFound = true;
						break;
					}
				}
			}

			if (!bSlotFound)
			{
				continue;
			}

			m_incomingPackets[m_statisticsSlot]++;
			m_incomingBytes[m_statisticsSlot] += numBytesWithHeader;

			for (int i = 0; i < MAX_MESSAGES; ++i)
			{
				if (m_inBuffer[i].length == 0 && m_inBufferOccupied[i])
				{
					m_inBufferOccupied[i] = false;
				}

				if (m_inBufferOccupied[i])
					continue;

				std::memset(&m_inBuffer[i], 0, sizeof(m_inBuffer[i]));
				m_inBuffer[i].header = incomingMessage.header;
				m_inBuffer[i].addr = incomingMessage.addr;
				m_inBuffer[i].port = incomingMessage.port;
				if (payloadLen > 0)
				{
					std::memcpy(m_inBuffer[i].data, incomingMessage.data, toCopy);
					m_inBuffer[i].length = static_cast<Int>(toCopy);
				}
				m_inBufferOccupied[i] = true;
				++numRead;
				bRet = TRUE;
				break;
			}
		}
	}

	return bRet;
}

Bool NextGenTransport::doSend(void)
{
	Bool retval = TRUE;
	int numSent = 0;

	for (int i = 0; i < MAX_MESSAGES; ++i)
	{
		if (m_outBuffer[i].length == 0)
		{
			m_outPacketState[i].retryCount = 0;
			continue;
		}

		if (TheNGMPGame == nullptr)
		{
			return FALSE;
		}

		NGMPGameSlot* pSlot = nullptr;
		if (m_outBuffer[i].addr < MAX_SLOTS)
		{
			pSlot = static_cast<NGMPGameSlot*>(TheNGMPGame->getSlot(m_outBuffer[i].addr));
		}
		else
		{
			for (int s = 0; s < MAX_SLOTS; ++s)
			{
				NGMPGameSlot* sSlot = static_cast<NGMPGameSlot*>(TheNGMPGame->getSlot(s));
				if (sSlot && (sSlot->getIP() == m_outBuffer[i].addr || sSlot->m_userID == m_outBuffer[i].addr))
				{
					pSlot = sSlot;
					break;
				}
			}
		}

		if (pSlot != nullptr && pSlot->m_userID > 0)
		{
			const uint32_t totalLen = static_cast<uint32_t>(m_outBuffer[i].length) + sizeof(TransportMessageHeader);
			if (totalLen > (sizeof(TransportMessageHeader) + MAX_PACKET_SIZE))
			{
				m_outBuffer[i].length = 0;
				m_outPacketState[i].retryCount = 0;
				retval = FALSE;
				continue;
			}

			NetworkMesh* pMesh = NGMP_OnlineServicesManager::GetNetworkMesh();
			if (pMesh == nullptr)
			{
				retval = FALSE;
				continue;
			}

			std::vector<uint8_t> packetData(totalLen);
			std::memcpy(packetData.data(), &m_outBuffer[i].header, sizeof(TransportMessageHeader));
			std::memcpy(packetData.data() + sizeof(TransportMessageHeader), m_outBuffer[i].data, m_outBuffer[i].length);

			int sendResult = pMesh->SendGamePacket(packetData.data(), totalLen, pSlot->m_userID);
			if (sendResult >= 0)
			{
				++numSent;
				m_outgoingPackets[m_statisticsSlot]++;
				m_outgoingBytes[m_statisticsSlot] += totalLen;
				m_outBuffer[i].length = 0;
				m_outPacketState[i].retryCount = 0;
			}
			else
			{
				m_outPacketState[i].retryCount++;
				if (m_outPacketState[i].retryCount >= OutgoingPacketState::MAX_RETRIES)
				{
					m_outBuffer[i].length = 0;
					m_outPacketState[i].retryCount = 0;
					retval = FALSE;
				}
			}
		}
		else
		{
			m_outBuffer[i].length = 0;
			m_outPacketState[i].retryCount = 0;
			retval = FALSE;
		}
	}

	return retval;
}

Bool NextGenTransport::queueSend(UnsignedInt addr, UnsignedShort port, const UnsignedByte* buf, Int len)
{
	if (buf == nullptr || len < 1 || len > MAX_PACKET_SIZE)
	{
		return FALSE;
	}

	for (int i = 0; i < MAX_MESSAGES; ++i)
	{
		if (m_outBuffer[i].length != 0)
			continue;

		const size_t dstCap = sizeof(m_outBuffer[i].data);
		if (static_cast<size_t>(len) > dstCap)
		{
			return FALSE;
		}

		std::memset(&m_outBuffer[i], 0, sizeof(m_outBuffer[i]));
		m_outBuffer[i].length = len;
		std::memcpy(m_outBuffer[i].data, buf, static_cast<size_t>(len));
		m_outBuffer[i].addr = addr;
		m_outBuffer[i].port = port;
		m_outBuffer[i].header.magic = GENERALS_MAGIC_NUMBER;

		CRC crc;
		const size_t crcLen = static_cast<size_t>(m_outBuffer[i].length) + sizeof(TransportMessageHeader) - sizeof(UnsignedInt);
		crc.computeCRC(reinterpret_cast<unsigned char*>(&(m_outBuffer[i].header.magic)), static_cast<unsigned int>(crcLen));
		m_outBuffer[i].header.crc = crc.get();

		return TRUE;
	}

	return FALSE;
}
