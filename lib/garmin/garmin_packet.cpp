/* garmin_packet.cpp
 */

#include "config.h"
#include "trace.h"
#include "garmin_packet.h"
#include "garmin_packet_ids.h"

GarminPacket::GarminPacket(const GarminPacket &other)
	: m_packet(NULL)
{
	Assign(other.m_packet);

	ptrdiff_t cursor_offset = other.m_ptr - other.m_packet;

	m_ptr = m_packet + cursor_offset;
}

GarminPacket::~GarminPacket()
{
	free(m_packet);
}

void GarminPacket::Assign(const BYTE *packet)
{
	free(m_packet);

	BYTE packet_data_size = packet[2];
	int packet_size = packet_data_size + 6;
	m_packet = (BYTE *)malloc(packet_size);
	if (m_packet)
		memcpy(m_packet, packet, packet_size);

	MoveFirst();
}

void GarminPacket::Dump()
{
	if (m_packet)
	{
		BYTE packet_id = m_packet[1];
		BYTE packet_data_size = m_packet[2];
		int packet_size = packet_data_size + 6;
		const BYTE *packet_data = m_packet + 3;
		const BYTE *packet_data_end = packet_data + packet_data_size;

		TRACE("DLE              = %3d (0x%02x)\n", m_packet[0], m_packet[0]);
		TRACE("packet_id        = %3d (0x%02x) (%s)\n", m_packet[1], m_packet[1], DescribePacketId(m_packet[1]).c_str());
		TRACE("packet_data_size = %3d (0x%02x)\n", m_packet[2], m_packet[2]);
		TRACE_HEX("packet_data\n", packet_data, packet_data + packet_data_size);
		TRACE("Checksum         = %3d (0x%02x)\n", m_packet[packet_size-3], m_packet[packet_size-3]);
		TRACE("DLE              = %3d (0x%02x)\n", m_packet[packet_size-2], m_packet[packet_size-2]);
		TRACE("ETX              = %3d (0x%02x)\n", m_packet[packet_size-1], m_packet[packet_size-1]);
	}
	else
		TRACE("Empty packet!\n");
}

void GarminPacket::MoveFirst()
{
	BYTE *packet_data = m_packet + 3;
	m_ptr = packet_data;
}

bool GarminPacket::ReadInteger(int *pi)
{
	// Integers are stored little-endian.
	BYTE lsb, msb;
	if (ReadByte(&lsb) && ReadByte(&msb))
	{
		*pi = lsb | (msb << 8);
		return true;
	}

	return false;
}

bool GarminPacket::ReadByte(BYTE *pb)
{
	if (!m_packet || !m_ptr)
		return false;

	BYTE packet_data_size = m_packet[2];
	const BYTE *packet_data = m_packet + 3;
	const BYTE *packet_data_end = packet_data + packet_data_size;

	if (m_ptr < packet_data_end)
	{
		*pb = *m_ptr++;
		return true;
	}

	return false;
}

bool GarminPacket::ReadLong(long *pl)
{
	// Longs are stored little-endian
	BYTE a, b, c, d;

	if (ReadByte(&a) && ReadByte(&b) && ReadByte(&c) && ReadByte(&d))
	{
		*pl = a | (b << 8) | (c << 16) | (d << 24);
		return true;
	}

	return false;
}

bool GarminPacket::ReadULong(unsigned long *pl)
{
	return ReadLong((long *)pl);
}

bool GarminPacket::ReadBool(bool *pb)
{
	BYTE b;
	if (ReadByte(&b))
	{
		*pb = b ? true : false;
		return true;
	}

	return false;
}

bool GarminPacket::ReadFloat(float *pf)
{
	if (!m_packet || !m_ptr)
		return false;

	BYTE packet_data_size = m_packet[2];
	const BYTE *packet_data = m_packet + 3;
	const BYTE *packet_data_end = packet_data + packet_data_size;

	ASSERT(sizeof(float) == 4);

	/** Conveniently, Visual C++ uses the same format for 'float' as is stored in the packet data. */
	if (m_ptr + sizeof(float) < packet_data_end)
	{
		*pf = *((float *)m_ptr);
		m_ptr += sizeof(float);
		return true;
	}

	return false;
}

// A string is just a NULL-terminated ASCII string.
// We just need to ensure that we've actually got enough packet for it.
bool GarminPacket::ReadString(std::string *s)
{
	if (!m_packet || !m_ptr)
		return false;

	BYTE packet_data_size = m_packet[2];
	const BYTE *packet_data = m_packet + 3;
	const BYTE *packet_data_end = packet_data + packet_data_size;

	// Remember where the string starts.
	char *string_begin = (char *)m_ptr;

	// Go in search of the end of the string.
	while (*m_ptr && m_ptr < packet_data_end)
		++m_ptr;

	// We didn't run out of packet data.
	if (m_ptr < packet_data_end)
	{
		s->assign(string_begin, (char *)m_ptr);

		++m_ptr;	// Advance to the next value.
		return true;
	}

	return false;
}

// Some Garmin packet types contain fixed-length strings.  This function reads them into fixed-length char arrays.
bool GarminPacket::ReadCharArray(char *pa, int len)
{
	if (!m_packet || !m_ptr)
		return false;

	for (int i = 0; i < len; ++i)
	{
		BYTE b;
		if (!ReadByte(&b))
			return false;

		pa[i] = b;
	}

	return true;
}

/* static */
BYTE GarminPacket::CalculatePacketChecksum(const BYTE *packet)
{
	// The checksum is the 2's complement of the sum of all bytes from
	// byte 1 to byte n-4 (the last packet_data byte).
	BYTE checksum = 0;

	BYTE packet_data_size = packet[2];

	// The packet_data starts at byte 3.
	for (int i = 1; i < 3 + packet_data_size; ++i)
		checksum += packet[i];
	checksum -= 1;
	checksum = ~checksum;

	return checksum;
}
