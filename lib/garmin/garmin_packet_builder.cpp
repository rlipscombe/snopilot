/* garmin_packet_builder.cpp
 *
 * Copyright (C) 2003 Snopilot, http://www.snopilot.com/
 */

#include "config.h"
#include "trace.h"
#include "garmin_packet_builder.h"
#include "garmin_packet.h"

void GarminPacketBuilder::SetByte(BYTE b)
{
	m_packet_data.push_back(b);

	ASSERT(m_packet_data.size() <= 255);
}

void GarminPacketBuilder::SetInteger(INT16 i)
{
	// Integers are 16-bit, little endian.
	SetByte(i & 0xff);
	SetByte((i & 0xff00) >> 8);
}

/** @todo DLE-stuffing. */
GarminPacket GarminPacketBuilder::Finish()
{
	size_t packet_size = m_packet_data.size() + 6;
	BYTE *p = (BYTE *)malloc(packet_size);

	p[0] = ASCII_DLE;
	p[1] = m_packet_id;
	p[2] = (BYTE)m_packet_data.size();

	memcpy(p + 3, m_packet_data.data(), m_packet_data.size());

	BYTE checksum = GarminPacket::CalculatePacketChecksum(p);
	p[packet_size-3] = checksum;
	p[packet_size-2] = ASCII_DLE;
	p[packet_size-1] = ASCII_ETX;

	GarminPacket packet;
	packet.Assign(p);

	free(p);
	return packet;
}
