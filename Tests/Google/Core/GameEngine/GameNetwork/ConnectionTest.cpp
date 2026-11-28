/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <gtest/gtest.h>
#include <algorithm>
#include <vector>

#include "GameNetwork/Connection.h"

namespace {
class RecordingTransport : public Transport
{
public:
	Bool queueSend(UnsignedInt, UnsignedShort, const UnsignedByte *data, Int size) override
	{
		if (!accepting) return FALSE;
		packets.emplace_back(data, data + size);
		return TRUE;
	}
	bool accepting = true;
	std::vector<std::vector<UnsignedByte>> packets;
};

class TestHop : public Connection
{
public:
	explicit TestHop(RecordingTransport &transport)
	{
		init();
		attachTransport(&transport);
		User *user = newInstance(User)();
		user->SetIPAddr(0x7f000001);
		user->SetPort(8088);
		setUser(user);
		setFrameGrouping(0);
	}
	using Connection::sendDueCommands;
	void announce(UnsignedShort id, UnsignedInt frame, UnsignedByte origin = 1, UnsignedShort commandCount = 2)
	{
		NetFrameCommandMsg *msg = newInstance(NetFrameCommandMsg)();
		msg->setID(id);
		msg->setPlayerID(origin);
		msg->setExecutionFrame(frame);
		msg->setCommandCount(commandCount);
		sendNetCommandMsg(msg, 4);
		msg->detach();
	}
	bool ack(UnsignedShort id, UnsignedInt now, UnsignedByte origin = 1)
	{
		NetCommandRef *ref = removeAcknowledgedCommand(id, origin, now);
		if (!ref) return false;
		deleteInstance(ref);
		return true;
	}
};

TEST(FrameRecovery, ResendsOriginalAfterDistinctLaterFramesAndReorderGrace)
{
	RecordingTransport transport;
	TestHop hop(transport);
	hop.announce(65535, 80);
	hop.sendDueCommands(1000);
	ASSERT_EQ(transport.packets.size(), 1u);
	const auto original = transport.packets.front();
	hop.announce(0, 81);
	hop.announce(1, 82);
	hop.announce(2, 82); // New command ID for the same frame.
	hop.announce(3, 83);
	hop.sendDueCommands(1010);
	transport.packets.clear();
	EXPECT_TRUE(hop.ack(1, 1090));
	EXPECT_TRUE(hop.ack(0, 1090));
	EXPECT_FALSE(hop.ack(0, 1091));
	EXPECT_TRUE(hop.ack(2, 1092));
	hop.sendDueCommands(1125);
	EXPECT_TRUE(transport.packets.empty());
	EXPECT_TRUE(hop.ack(3, 1126));
	hop.sendDueCommands(1126); // Less than 1.5 times the smoothed RTT.
	EXPECT_TRUE(transport.packets.empty());
	hop.sendDueCommands(1160);
	ASSERT_EQ(transport.packets.size(), 1u);
	EXPECT_EQ(transport.packets.front(), original);
	hop.sendDueCommands(2000);
	EXPECT_EQ(transport.packets.size(), 1u);
	hop.sendDueCommands(3161); // Lost early retry still has the normal fallback.
	ASSERT_EQ(transport.packets.size(), 2u);
	EXPECT_EQ(transport.packets.back(), original);
	EXPECT_TRUE(hop.ack(65535, 3200));
	EXPECT_FALSE(hop.ack(65535, 3210));
	EXPECT_TRUE(hop.isQueueEmpty());
}

TEST(FrameRecovery, AcksAreSpecificToNextHopAndOriginalPlayer)
{
	RecordingTransport transport;
	TestHop a(transport), b(transport);
	a.announce(10, 20);
	b.announce(10, 20);
	for (UnsignedShort id = 11; id <= 13; ++id) {
		a.announce(id, id + 10, 2);
		b.announce(id, id + 10);
	}
	a.sendDueCommands(1000);
	b.sendDueCommands(1000);
	for (UnsignedShort id = 11; id <= 13; ++id) {
		EXPECT_TRUE(a.ack(id, 1010, 2));
		EXPECT_TRUE(b.ack(id, 1010));
	}
	transport.packets.clear();
	a.sendDueCommands(1100);
	b.sendDueCommands(1049);
	EXPECT_TRUE(transport.packets.empty());
	EXPECT_TRUE(b.ack(10, 1049)); // Reordered original ACK cancels the retry.
	b.sendDueCommands(1100);
	EXPECT_TRUE(transport.packets.empty());
	a.sendDueCommands(3001); // No later ACK from this origin on this hop.
	EXPECT_EQ(transport.packets.size(), 1u);
}

TEST(FrameRecovery, RejectedLocalQueueDoesNotConsumeSendOrRetry)
{
	RecordingTransport transport;
	TestHop hop(transport);
	hop.setFrameGrouping(10);
	hop.announce(30, 70);
	transport.accepting = false;
	hop.sendDueCommands(1000);
	EXPECT_FALSE(hop.ack(30, 1001));
	transport.accepting = true;
	hop.sendDueCommands(1001);
	ASSERT_EQ(transport.packets.size(), 1u);
	const auto first = transport.packets.front();
	for (UnsignedShort id = 31; id <= 33; ++id)
		hop.announce(id, id + 40);
	hop.sendDueCommands(1011);
	for (UnsignedShort id = 31; id <= 33; ++id)
		EXPECT_TRUE(hop.ack(id, 1015));
	transport.packets.clear();
	transport.accepting = false;
	hop.sendDueCommands(1100);
	transport.accepting = true;
	hop.sendDueCommands(1101);
	ASSERT_EQ(transport.packets.size(), 1u);
	EXPECT_EQ(transport.packets.front(), first);
}

TEST(FrameRecovery, ForwardedEmptyFrameKeepsItsOriginWhenPackedWithARetry)
{
	RecordingTransport transport;
	TestHop hop(transport);
	hop.announce(20, 80);
	hop.sendDueCommands(1000);
	for (UnsignedShort id = 22; id <= 24; ++id)
		hop.announce(id, id + 60);
	hop.sendDueCommands(1010);
	for (UnsignedShort id = 22; id <= 24; ++id)
		EXPECT_TRUE(hop.ack(id, 1015));

	transport.packets.clear();
	hop.announce(21, 81, 2, 0);
	hop.sendDueCommands(1100);
	ASSERT_EQ(transport.packets.size(), 1u);
	const auto &data = transport.packets.front();
	TransportMessage datagram{};
	datagram.length = static_cast<Int>(data.size());
	std::copy(data.begin(), data.end(), datagram.data);
	NetPacket packet(datagram);
	NetCommandList *commands = packet.getCommandList();
	EXPECT_NE(commands->findMessage(20, 1), nullptr);
	EXPECT_EQ(commands->findMessage(21, 1), nullptr);
	NetCommandRef *forwarded = commands->findMessage(21, 2);
	EXPECT_NE(forwarded, nullptr);
	if (forwarded) {
		EXPECT_EQ(forwarded->getCommand()->getExecutionFrame(), 81u);
		EXPECT_EQ(static_cast<NetFrameCommandMsg *>(forwarded->getCommand())->getCommandCount(), 0);
		EXPECT_EQ(forwarded->getRelay(), 4);
	}
	deleteInstance(commands);
}
} // namespace
