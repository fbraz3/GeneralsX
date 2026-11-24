/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////


#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "GameNetwork/Connection.h"
#include "GameNetwork/networkutil.h"
#include "GameLogic/GameLogic.h"

enum { MaxQuitFlushTime = 30000 }; // wait this many milliseconds at most to retry things before quitting

/**
 * The constructor.
 */
Connection::Connection() {
	m_transport = nullptr;
	m_user = nullptr;
	m_netCommandList = nullptr;
	m_retryTime = 2000; // set retry time to 2 seconds.
	m_lastTimeSent = 0;
	m_frameGrouping = 1;
	m_isQuitting = false;
	m_quitTime = 0;
}

/**
 * The destructor.
 */
Connection::~Connection() {
	deleteInstance(m_user);
	m_user = nullptr;

	deleteInstance(m_netCommandList);
	m_netCommandList = nullptr;
}

/**
 * Initialize the connection and any subsystems.
 */
void Connection::init() {
	m_transport = nullptr;

	deleteInstance(m_user);
	m_user = nullptr;

	if (m_netCommandList == nullptr) {
		m_netCommandList = newInstance(NetCommandList);
		m_netCommandList->init();
	}
	m_netCommandList->reset();

	m_lastTimeSent = 0;
	m_frameGrouping = 1;
	m_numRetries = 0;
	m_retryMetricsTime = 0;

	m_latestFrameAcks = {};
	m_frameAckMs = 0;
	m_isQuitting = FALSE;
	m_quitTime = 0;
}

/**
 * Take the connection back to the initial state.
 */
void Connection::reset() {
	init();
}

/**
 * Doesn't really do anything.
 */
void Connection::update() {
}

/**
 * Attach the transport object that this connection should use.
 */
void Connection::attachTransport(Transport *transport) {
	m_transport = transport;
}

/**
 * Assign this connection a user.  This is the user to whome we send all our packetized goodies.
 */
void Connection::setUser(User *user) {
	deleteInstance(m_user);
	m_user = user;
}

/**
 * Return the user object.
 */
User * Connection::getUser() {
	return m_user;
}

/**
 * Add this network command to the send queue for this connection.
 * The relay is the mask specifying the people the person we are sending to should send to.
 * The relay mostly has to do with the packet router.
 */
void Connection::sendNetCommandMsg(NetCommandMsg *msg, UnsignedByte relay) {
	if (m_isQuitting)
		return;

	if (m_netCommandList != nullptr) {
		NetPacket packet;

		// check to see if this command will fit in a packet.  If not, we need to split it up.
		// we are splitting up the command here so that the retry logic will not try to
		// resend the ENTIRE command (i.e. multiple packets work of data) and only do the retry
		// one wrapper command at a time.

		NetCommandRef *tempref = NEW_NETCOMMANDREF(msg);
		if (packet.addCommand(tempref)) {
			deleteInstance(tempref);
			tempref = nullptr;
		} else {
			tempref->setRelay(relay);

			// the message doesn't fit in a single packet, need to split it up.
			if (NetCommandList* list = NetPacket::ConstructBigCommandList(tempref)) {
				for (NetCommandRef* ref1 = list->getFirstMessage(); ref1 != nullptr; ref1 = ref1->getNext()) {
					if (NetCommandRef* ref2 = m_netCommandList->addMessage(ref1->getCommand())) {
						ref2->setRelay(relay);
					}
				}

				deleteInstance(list);
				list = nullptr;
			}

			deleteInstance(tempref);
			tempref = nullptr;

			return;
		}

		// the message fits in a packet, add to the command list normally.
		NetCommandRef *ref = m_netCommandList->addMessage(msg);

		if (ref != nullptr) {

/*
#if defined(RTS_DEBUG)
			if (msg->getNetCommandType() == NETCOMMANDTYPE_GAMECOMMAND) {
				DEBUG_LOG(("Connection::sendNetCommandMsg - added game command %d to net command list for frame %d.",
					msg->getID(), msg->getExecutionFrame()));
			} else if (msg->getNetCommandType() == NETCOMMANDTYPE_FRAMEINFO) {
				DEBUG_LOG(("Connection::sendNetCommandMsg - added frame info for frame %d", msg->getExecutionFrame()));
			}
#endif // RTS_DEBUG
*/

			ref->setRelay(relay);
		}
	}
}

void Connection::clearCommandsExceptFrom( Int playerIndex )
{
	m_latestFrameAcks = {};
	NetCommandRef *tmp = m_netCommandList->getFirstMessage();
	while (tmp)
	{
		NetCommandRef *next = tmp->getNext();
		NetCommandMsg *msg = tmp->getCommand();

		if (msg->getPlayerID() != playerIndex)
		{
			DEBUG_LOG(("Connection::clearCommandsExceptFrom(%d) - clearing a command from player %d for frame %d",
				playerIndex, tmp->getCommand()->getPlayerID(), tmp->getCommand()->getExecutionFrame()));

			m_netCommandList->removeMessage(tmp);
			deleteInstance(tmp);
		}

		tmp = next;
	}
}

Bool Connection::isQueueEmpty() {
	if (m_netCommandList->getFirstMessage() == nullptr) {
		return TRUE;
	}
	return FALSE;
}

void Connection::setQuitting()
{
	m_isQuitting = TRUE;
	m_quitTime = timeGetTime();
	DEBUG_LOG(("Connection::setQuitting() at time %d", m_quitTime));
}

// GeneralsX @bugfix Codex 24/11/2026 Recover an isolated hole in the reliable frame stream before lockstep exhausts its buffer.
Bool Connection::shouldSend(const NetCommandRef *ref, UnsignedInt now) const
{
	const time_t last = ref->getTimeLastSent();
	const UnsignedInt age = now - static_cast<UnsignedInt>(last);
	if (last == -1 || age > m_retryTime)
		return TRUE;
	const NetCommandMsg *cmd = ref->getCommand();
	if (cmd->getNetCommandType() != NETCOMMANDTYPE_FRAMEINFO || ref->wasRetried() || cmd->getPlayerID() >= MAX_SLOTS)
		return FALSE;
	// Allow one early retry after three distinct later frames have been acknowledged. Leave time for reordered ACKs.
	return cmd->getExecutionFrame() < m_latestFrameAcks[cmd->getPlayerID()].back() && age >= 50 && age >= 1.5f * m_frameAckMs;
}

void Connection::acknowledgeFrame(const NetCommandRef *ref, UnsignedInt now)
{
	const NetCommandMsg *cmd = ref->getCommand();
	if (cmd->getNetCommandType() != NETCOMMANDTYPE_FRAMEINFO || cmd->getPlayerID() >= MAX_SLOTS || ref->getTimeLastSent() == -1)
		return;
	// An ACK after retransmission cannot identify which copy arrived.
	if (!ref->wasRetried()) {
		const Real sample = static_cast<Real>(now - static_cast<UnsignedInt>(ref->getTimeLastSent()));
		m_frameAckMs = m_frameAckMs == 0 ? sample : m_frameAckMs + (sample - m_frameAckMs) / 8.0f;
	}
	UnsignedInt frame = cmd->getExecutionFrame();
	for (UnsignedInt &recent : m_latestFrameAcks[cmd->getPlayerID()]) {
		if (recent == frame)
			break;
		if (frame > recent)
			std::swap(frame, recent);
	}
}

/**
 * This is the good part. We take all the network commands queued up for this connection,
 * packetize them and put them on the transport's send queue for actual sending.
 */
UnsignedInt Connection::doSend() {
	return sendDueCommands(timeGetTime());
}

UnsignedInt Connection::sendDueCommands(time_t curtime) {
	Int numpackets = 0;
	Bool couldQueue = TRUE;

	// Do this check first, since it's an important fail-safe
	if (m_isQuitting && curtime > m_quitTime + MaxQuitFlushTime)
	{
		DEBUG_LOG(("Timed out a quitting connection.  Deleting all %d messages", m_netCommandList->length()));
		m_netCommandList->reset();
		return 0;
	}

	if ((static_cast<UnsignedInt>(curtime) - static_cast<UnsignedInt>(m_lastTimeSent)) < m_frameGrouping) {
//		DEBUG_LOG(("not sending packet, time = %d, m_lastFrameSent = %d, m_frameGrouping = %d", curtime, m_lastTimeSent, m_frameGrouping));
		return 0;
	}

	// iterate through all the messages and put them into a packet(s).
	NetCommandRef *msg = m_netCommandList->getFirstMessage();

	while ((msg != nullptr) && couldQueue) {
		NetPacket packet;
		packet.setAddress(m_user->GetIPAddr(), m_user->GetPort());

		// Every command uses at least one wire byte. Commit this batch only after the transport accepts the datagram.
		NetCommandRef *batch[MAX_PACKET_SIZE];
		while (msg != nullptr) {
			if (shouldSend(msg, static_cast<UnsignedInt>(curtime))) {
				if (!packet.addCommand(msg)) {
					// Oversized commands are split by sendNetCommandMsg before they reach this queue.
					if (packet.getNumCommands() == 0)
						msg = msg->getNext();
					break;
				}
				batch[packet.getNumCommands() - 1] = msg;
			}
			msg = msg->getNext();
		}

		if (msg != nullptr) {
			DEBUG_LOG(("didn't finish sending all commands in connection"));
		}

		++numpackets;

		/// @todo Make the act of giving the transport object a packet to send more efficient.  Make the transport take a NetPacket object rather than the raw data, thus avoiding an extra memcpy.
		if (packet.getNumCommands() > 0) {
			// If the packet actually has any information to give, give it to the transport object
			// for transmission.
			couldQueue = m_transport->queueSend(packet.getAddr(), packet.getPort(), packet.getData(), packet.getLength());
			if (couldQueue) {
				m_lastTimeSent = curtime;
				for (Int i = 0; i < packet.getNumCommands(); ++i) {
					NetCommandRef *sent = batch[i];
					if (CommandRequiresAck(sent->getCommand())) {
						if (sent->getTimeLastSent() != -1)
							++m_numRetries;
						doRetryMetrics();
						sent->markSent(curtime);
					} else {
						m_netCommandList->removeMessage(sent);
						deleteInstance(sent);
					}
				}
			}
		}
	}

	return numpackets;
}

NetCommandRef * Connection::processAck(NetAckStage1CommandMsg *msg) {
	return processAck(msg->getCommandID(), msg->getOriginalPlayerID());
}

NetCommandRef * Connection::processAck(NetAckBothCommandMsg *msg) {
	return processAck(msg->getCommandID(), msg->getOriginalPlayerID());
}

NetCommandRef * Connection::processAck(NetCommandMsg *msg) {
	if (msg->getNetCommandType() == NETCOMMANDTYPE_ACKSTAGE1) {
		NetAckStage1CommandMsg *ackmsg = (NetAckStage1CommandMsg *)msg;
		return processAck(ackmsg);
	}

	if (msg->getNetCommandType() == NETCOMMANDTYPE_ACKBOTH) {
		NetAckBothCommandMsg *ackmsg = (NetAckBothCommandMsg *)msg;
		return processAck(ackmsg);
	}

	return nullptr;
}

/**
 * The person we are sending to has ack'd one of the messages we sent him.
 * Take that message off the list of commands to send.
 */
NetCommandRef * Connection::processAck(UnsignedShort commandID, UnsignedByte originalPlayerID) {
	return removeAcknowledgedCommand(commandID, originalPlayerID, timeGetTime());
}

NetCommandRef *Connection::removeAcknowledgedCommand(UnsignedShort commandID, UnsignedByte originalPlayerID, UnsignedInt now) {
	NetCommandRef *temp = m_netCommandList->findMessage(commandID, originalPlayerID);
	if (temp == nullptr || temp->getTimeLastSent() == -1) {
		return nullptr;
	}

#if defined(RTS_DEBUG)
	Bool doDebug = FALSE;
	if (temp->getCommand()->getNetCommandType() == NETCOMMANDTYPE_DISCONNECTFRAME) {
		doDebug = TRUE;
	}
#endif

	acknowledgeFrame(temp, now);

#if defined(RTS_DEBUG)
	if (doDebug == TRUE) {
		DEBUG_LOG(("Connection::processAck - disconnect frame command %d found, removing from command list.", commandID));
	}
#endif
	m_netCommandList->removeMessage(temp);
	return temp;
}

void Connection::setFrameGrouping(time_t frameGrouping) {
	m_frameGrouping = frameGrouping;
//	m_retryTime = frameGrouping * 4;
}

void Connection::doRetryMetrics() {
	static Int numSeconds = 0;
	time_t curTime = timeGetTime();

	if ((curTime - m_retryMetricsTime) > 10000) {
		m_retryMetricsTime = curTime;
		++numSeconds;
		m_numRetries = 0;
	}
}

#if defined(RTS_DEBUG)
void Connection::debugPrintCommands() {
	NetCommandRef *ref = m_netCommandList->getFirstMessage();
	while (ref != nullptr) {
		DEBUG_LOG(("Connection::debugPrintCommands - ID: %d\tType: %s\tRelay: 0x%X for frame %d",
			ref->getCommand()->getID(), GetNetCommandTypeAsString(ref->getCommand()->getNetCommandType()),
			ref->getRelay(), ref->getCommand()->getExecutionFrame()));
		ref = ref->getNext();
	}
}
#endif
