#ifndef GARMIN_PACKET_H
#define GARMIN_PACKET_H 1

#include <string>

/** This class both holds a packet and allows reading values from it. */
class GarminPacket
{
	BYTE *m_packet;		///< A copy of the packet.
	BYTE *m_ptr;		///< The iterator -- where's the next value coming from?

public:
	GarminPacket()
		: m_packet(NULL), m_ptr(NULL)
	{
	}

	~GarminPacket();

	GarminPacket(const GarminPacket &other);
	const GarminPacket &operator= (const GarminPacket &other);	// not implemented.

public:
	void Assign(const BYTE *packet);
	void Dump();

	bool IsValid() const { return (m_packet != NULL); }

	static BYTE CalculatePacketChecksum(const BYTE *packet);

public:
	BYTE GetPacketID() const { return IsValid() ? m_packet[1] : 0; }
	BYTE GetPacketDataSize() const { return IsValid() ? m_packet[2] : 0; }
	
	const BYTE *GetBuffer() const { return m_packet; }
	int GetBufferLength() const { return IsValid() ? GetPacketDataSize() + 6 : 0; }

	void MoveFirst();

	/** Read a single byte from the packet data. */
	bool ReadByte(BYTE *pb);

	/** Read a 16-bit, little-endian signed integer from the packet data. */
	bool ReadInteger(int *pi);

	/** Read an ASCIIZ string from the packet data. */
	bool ReadString(std::string *s);

	/** Read a 32-bit, little-endian signed integer from the packet data. */
	bool ReadLong(long *pl);

	/** Read a 32-bit, little-endian unsigned integer from the packet data. */
	bool ReadULong(unsigned long *pl);

	/** Read a 32-bit IEEE floating point number (1 sign, 8 exponent, 23 mantissa bits) from the packet data. */
	bool ReadFloat(float *pf);

	/** Read a boolean from the packet data.  It's a single byte, zero => false, non-zero => true. */
	bool ReadBool(bool *pb);

	/** Read a fixed-length char array from the packet data. */
	bool ReadCharArray(char *pa, int len);
};

const BYTE ASCII_DLE = 16;
const BYTE ASCII_ETX = 3;

#endif /* GARMIN_PACKET_H */
