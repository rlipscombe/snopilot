/* basic_link_protocol.h
 *
 * Copyright (C) 2003 Snopilot, http://www.snopilot.com/
 */

#ifndef BASIC_LINK_PROTOCOL_H
#define BASIC_LINK_PROTOCOL_H 1

#include "buffer_collector.h"
#include "serial_device.h"
#include "garmin_packet.h"

/** @todo This should have just SendPacket and ReceivePacket calls.
 * The ACK/NAK stuff should be hidden in here.
 */
class BasicLinkProtocol
{
	SerialDevice *m_device;
	BufferCollector m_buffer;

public:
	BasicLinkProtocol(SerialDevice *device)
		: m_device(device)
	{
	}

	/** Send a packet.  Don't wait for a reply. */
	bool SendPacket(const BYTE *packet, int packet_size);
	bool SendPacket(const GarminPacket &packet);
	
	/** Specifically wait for an ACK/NAK. */
	bool WaitForAck();

	/** Wait for a packet -- any packet. */
	bool WaitForPacket(GarminPacket *packet);

	/** Acknowledge receipt of a particular packet.  Pass the ID of the packet to be acknowledged. */
	bool SendAck(BYTE acknowledged_packet_id);
};

#endif /* BASIC_LINK_PROTOCOL_H */
