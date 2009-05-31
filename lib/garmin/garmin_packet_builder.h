#ifndef GARMIN_PACKET_BUILDER_H
#define GARMIN_PACKET_BUILDER_H 1

#include <string>

class GarminPacket;

/** It's not quite a GoF "Builder", it's more of a "mould".
 * You can only create a single packet with it.
 */
class GarminPacketBuilder
{
	BYTE m_packet_id;

	typedef std::basic_string<BYTE> bytes_t;
	bytes_t m_packet_data;

public:
	GarminPacketBuilder(BYTE packet_id)
		: m_packet_id(packet_id)
	{
	}

	void SetByte(BYTE b);
	void SetInteger(INT16 i);

	GarminPacket Finish();
};

#endif /* GARMIN_PACKET_BUILDER_H */
