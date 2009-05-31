#ifndef GARMIN_PACKET_COLLECTOR_H
#define GARMIN_PACKET_COLLECTOR_H 1

#include "buffer_collector.h"

class GarminPacket;

/** Used with BufferCollector to grab a single Garmin packet from the serial port. */
class GarminPacketCollector : public BufferCollectorCallback
{
	GarminPacket *m_packet;

public:
	GarminPacketCollector(GarminPacket *packet)
		: m_packet(packet)
	{
	}

// BufferCollectorCallback
public:
	virtual bool ProcessBuffer(const char *buffer, int buffer_size, int *bytes_eaten);
};

#endif /* GARMIN_PACKET_COLLECTOR_H */
