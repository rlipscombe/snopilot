/* basic_link_protocol.cpp
 *
 * Copyright (C) 2003 Snopilot, http://www.snopilot.com/
 */

#include "config.h"
#include "trace.h"
#include "basic_link_protocol.h"
#include "garmin_packet_ids.h"
#include "garmin_packet_collector.h"
#include "garmin_packet_builder.h"

#define TRACE_LINK 0

bool BasicLinkProtocol::SendPacket(const BYTE *packet, int packet_size)
{
//	TRACE_HEX("BasicLinkProtocol::SendPacket\n", packet, packet + packet_size);

	unsigned bytesWritten;
	bool result = m_device->Write(packet, packet_size, &bytesWritten);
	if (!result)
	{
		TRACEC(TRACE_LINK, "Write failed, error = %d\n", GetLastError());
		return false;
	}

	TRACEC(TRACE_LINK, "Wrote %d byte(s)\n", bytesWritten);
	return true;
}

bool BasicLinkProtocol::SendPacket(const GarminPacket &packet)
{
	return SendPacket(packet.GetBuffer(), packet.GetBufferLength());
}

bool BasicLinkProtocol::SendAck(BYTE acknowledged_packet_id)
{
	TRACEC(TRACE_LINK, "BasicLinkProtocol::SendAck(id = %d)\n", acknowledged_packet_id);

	// It's not quite a builder: it can only create a single packet.
	GarminPacketBuilder builder(Pid_Ack_Byte);
	builder.SetByte(acknowledged_packet_id);
	GarminPacket packet = builder.Finish();

	return SendPacket(packet);
}

/** @todo NAKs should cause retransmission -- and it should all be hidden in SendPacket. */
bool BasicLinkProtocol::WaitForAck()
{
	TRACEC(TRACE_LINK, "BasicLinkProtocol::WaitForAck()\n");

	for (;;)
	{
		GarminPacket packet;
		if (!WaitForPacket(&packet))
			return false;

		if (packet.GetPacketID() == Pid_Ack_Byte)
		{
			break;
		}
		else
		{
			TRACEC(TRACE_LINK, "Received packet ID %d while waiting for ACK.  Ignoring packet.\n", packet.GetPacketID());
			TRACEC(TRACE_LINK, "(Sending ACK to work around Geko 201 bug).\n");
			packet.Dump();

			// While downloading the track log, the Geko 201 seems to send the Trk_Hdr packet before sending the ACK.
			// For now, we'll send it an ACK.
			SendAck(packet.GetPacketID());
		}
	}

	return true;
}

bool BasicLinkProtocol::WaitForPacket(GarminPacket *packet)
{
	TRACEC(TRACE_LINK, "BasicLinkProtocol::WaitForPacket()\n");

	GarminPacketCollector cb(packet);

	// See if there's enough data in the buffer already, so we don't
	// need to go back to the serial port.
	if (!m_buffer.Flush(&cb))
		return true;

	const DWORD READ_PACKET_TIMEOUT_MS = 2000;
	long timeRemaining = READ_PACKET_TIMEOUT_MS;
	for (;;)
	{
		char buffer[1024];
		int buffer_len = 1024;
		unsigned bytesRead;

		DWORD startRead = GetTickCount();
		if (!m_device->Read(buffer, buffer_len, &bytesRead))
			return false;	// The serial connection was broken.
		DWORD endRead = GetTickCount();
		DWORD timeTaken = endRead - startRead;
		timeRemaining -= timeTaken;

		if (bytesRead != 0)
		{
			if (!m_buffer.PushData(buffer, bytesRead, &cb))
				break;
		}
		else
		{
			TRACEC(TRACE_LINK, "Warning: zero-byte Read in WaitForPacket!\n");
		}

		if (timeRemaining < 0)
		{
			TRACEC(TRACE_LINK, "Warning: timeout in WaitForPacket!\n");
			return false;
		}
	}

	return true;
}
