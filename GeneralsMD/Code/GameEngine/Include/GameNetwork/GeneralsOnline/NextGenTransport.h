// GeneralsX @feature GeneralsOnline NextGenTransport header
#pragma once

#include "GameNetwork/udp.h"
#include "GameNetwork/NetworkDefs.h"
#include "GameNetwork/Transport.h"
#include "GameNetwork/GeneralsOnline/PluginInterfaces.h"
#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingmessages.h>

// Struct to track retry state for outgoing packets
struct OutgoingPacketState
{
	Int retryCount = 0;
	static constexpr Int MAX_RETRIES = 3;
};

class NextGenTransport : public Transport
{
public:
	NextGenTransport();
	~NextGenTransport() override;

	Bool init(AsciiString ip, UnsignedShort port) override;
	Bool init(UnsignedInt ip, UnsignedShort port) override;
	void reset(void) override;
	Bool update(void) override;

	Bool doRecv(void) override;
	Bool doSend(void) override;

	Bool queueSend(UnsignedInt addr, UnsignedShort port, const UnsignedByte *buf, Int len) override;

	inline Bool allowBroadcasts(Bool) override { return false; }

	void clearInBufferSlot(int slotIndex)
	{
		if (slotIndex >= 0 && slotIndex < MAX_MESSAGES)
		{
			m_inBuffer[slotIndex].length = 0;
			m_inBufferOccupied[slotIndex] = false;
		}
	}

private:
	OutgoingPacketState m_outPacketState[MAX_MESSAGES];
	bool m_inBufferOccupied[MAX_MESSAGES];
};
