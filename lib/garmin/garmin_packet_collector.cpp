#include "config.h"
#include "trace.h"
#include "garmin_packet_collector.h"
#include "garmin_packet.h"

// BufferCollectorCallback
bool GarminPacketCollector::ProcessBuffer(const char *buffer, int buffer_size, int *bytes_eaten)
{
//	TRACE_HEX("buffer:\n", buffer, buffer + buffer_size);

	/* A packet looks like this:
	Byte Number Byte Description		Notes
	0			Data Link Escape		ASCII DLE character (16 decimal)
	1			Packet ID				identifies the type of packet
	2			Size of Packet Data		number of bytes of packet data (bytes n-4)
	3 to n-4	Packet Data				0 to 255 bytes
	n-3			Checksum				2's complement of the sum of all bytes from byte 1 to byte n-4
	n-2			Data Link Escape		ASCII DLE character (16 decimal)
	n-1			End of Text				ASCII ETX character (3 decimal)
	*/

	// Reset the counter, for sanity's sake.
	*bytes_eaten = 0;

	// Look for the start of any possible packet.
	const BYTE *packet_start = (const BYTE *)memchr(buffer, ASCII_DLE, buffer_size);
	if (!packet_start)
		return true;

//	TRACE("Found possible packet @ 0x%x\n", packet_start - (const BYTE *)buffer);

	const BYTE *buffer_end = (const BYTE *)buffer + buffer_size;

	std::basic_string<BYTE> packet_buffer;

	// Walk from there, looking for DLE/ETX.  Copy each byte as we find it.
	for (const BYTE *p = packet_start; p < buffer_end - 1; ++p)
	{
		if (p[0] == ASCII_DLE)
		{
			if (p[1] == ASCII_DLE)
			{
				++p;
			}
			else if (p[1] == ASCII_ETX)
			{
				BYTE actual_checksum = packet_buffer[packet_buffer.size()-1];
				BYTE expected_checksum = GarminPacket::CalculatePacketChecksum(packet_buffer.data());
				if (expected_checksum != actual_checksum)
				{
					TRACE("Checksum verification failed. (expected 0x%02x, got 0x%02x)\n", expected_checksum, actual_checksum);
					TRACE_HEX("buffer:\n", buffer, buffer_end);
					TRACE("offset = %x\n", p - (const BYTE *)buffer);
					*bytes_eaten = 0;
					return false;
				}

				// Make sure that we've actually copied the terminator.
				packet_buffer.push_back(p[0]);
				packet_buffer.push_back(p[1]);

				m_packet->Assign(packet_buffer.data());

//				TRACE_HEX("Collected packet:\n", packet_buffer.data(), packet_buffer.data() + packet_buffer.size());
				
				// Allow 2 bytes for the DLE/ETX that we've not actually advanced past.
				*bytes_eaten = (int)(p - (const BYTE *)buffer) + 2;
				return false;
			}
		}

		packet_buffer.push_back(*p);
	}

	// If we're leaving this way, we didn't find anything -- make sure that the counter's still zero.
	*bytes_eaten = 0;
	return true;
}
