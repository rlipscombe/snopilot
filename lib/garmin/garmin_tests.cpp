#include "config.h"
#include "trace.h"
#include "test/test_assert.h"
#include "garmin_packet_ids.h"
#include "buffer_collector.h"
#include "garmin_packet.h"
#include "garmin_packet_collector.h"
#include <time.h>
#include "tokenise_string.h"
#include <algorithm>
#include <float.h>
#include "serial_device.h"
#include "basic_link_protocol.h"

class AppendHexBytes
{
	std::basic_string<BYTE> *m_result;

public:
	AppendHexBytes(std::basic_string<BYTE> *result)
		: m_result(result)
	{
	}

	void operator() (const std::string &hex_bytes)
	{
		if (!hex_bytes.empty())
		{
			char *p = NULL;
			unsigned long byte_value = strtoul(hex_bytes.c_str(), &p, 16);
			ASSERT(byte_value >= 0 && byte_value <= 255);

			m_result->push_back((BYTE)byte_value);
		}
	}
};

std::basic_string<BYTE> DataFromString(const char *str)
{
	std::basic_string<BYTE> r;

	tokens_t hex_bytes = TokeniseString(str, ' ');
	std::for_each(hex_bytes.begin(), hex_bytes.end(), AppendHexBytes(&r));

	return r;
}

void testPacketCollector_Ack_Byte()
{
	// We feed the data at a BufferCollector.  It hands it off to a GarminPacketCollector.
	// When there's a complete packet, the GarminPacket object is filled in, and the
	// BufferCollector returns false (i.e. "no more work").
	BufferCollector bufferCollector;

	// Feed it the first complete packet: a simple test.
	const char *ack_packet_str = "10 06 02 fe 00 fa 10 03";
	std::basic_string<BYTE> ack_packet = DataFromString(ack_packet_str);

	GarminPacket packet;
	GarminPacketCollector packetCollector(&packet);
	for (;;)
	{
		bool next = bufferCollector.PushData((const char *)ack_packet.data(), (int)ack_packet.size(), &packetCollector);
		if (!next)
			break;
	}

	// It should be an ACK packet.
	assertTrue(packet.IsValid());
	assertIntegersEqual(Pid_Ack_Byte, packet.GetPacketID());
	assertIntegersEqual(2, packet.GetPacketDataSize());
	BYTE b0, b1;
	assertTrue(packet.ReadByte(&b0));
	assertTrue(packet.ReadByte(&b1));
	assertIntegersEqual(0xfe, b0);
	assertIntegersEqual(0x00, b1);
}

void testPacketCollector_Product_Data()
{
	BufferCollector bufferCollector;

	// The packet originally arrived in chunks (the Geko 201 appears to have a 16-byte FIFO)
	// So we'll push it in chunks:
	const char *product_data_packet_str[] = {
		"10 ff 23 f8 00 e6",
		"00 47 65 6b 6f 20 32 30 31 20 53 6f 66 74",
		"77 61 72 65 20 56 65 72 73 69 6f 6e 20 32",
		"2e 33 30 00 73 10 03",
	};

	GarminPacket packet;
	GarminPacketCollector packetCollector(&packet);
	for (int i = 0; i < COUNTOF(product_data_packet_str); ++i)
	{
		const char *data_str = product_data_packet_str[i];
		std::basic_string<BYTE> data = DataFromString(data_str);

		bool next = bufferCollector.PushData((const char *)data.data(), (int)data.size(), &packetCollector);
		if (!next)
			break;
	}

	assertTrue(packet.IsValid());
	assertIntegersEqual(Pid_Product_Data, packet.GetPacketID());
	assertIntegersEqual(0x23, packet.GetPacketDataSize());
}

void testPacketCollector_Trk_Data()
{
	BufferCollector bufferCollector;

	// The packet originally arrived in chunks (the Geko 201 appears to have a 16-byte FIFO)
	// So we'll push it in chunks:
	const char *trk_data_packet_str[] = {
		"10 22 18 00 e5 20 25 00 05 1b 00 0c 64 02 1a 00",
		"00 a7 bf 51 59 04 69 00 20 56 65 98",
		"10 03",
	};

	GarminPacket packet;
	GarminPacketCollector packetCollector(&packet);
	for (int i = 0; i < COUNTOF(trk_data_packet_str); ++i)
	{
		const char *data_str = trk_data_packet_str[i];
		std::basic_string<BYTE> data = DataFromString(data_str);

		bool next = bufferCollector.PushData((const char *)data.data(), (int)data.size(), &packetCollector);
		if (!next)
			break;
	}

	assertTrue(packet.IsValid());
	assertIntegersEqual(Pid_Trk_Data, packet.GetPacketID());
	assertIntegersEqual(24, packet.GetPacketDataSize());
}

void testPacketCollector_Trk_Data_Stuffed()
{
	BufferCollector bufferCollector;

	// The packet originally arrived in chunks (the Geko 201 appears to have a 16-byte FIFO)
	// So we'll push it in chunks:
	const char *trk_data_packet_str[] = {
		"10 22 18 00 df 20 25 00 f4 1a",
		"00 10 10 64 02 1a 00 a1 b3 c1 51 59 04 69",
		"00 20 56 65 fd 10 03 10 22 18 00 c6 20 25",	// there's a little of the next packet in here.
	};

	GarminPacket packet;
	GarminPacketCollector packetCollector(&packet);
	for (int i = 0; i < COUNTOF(trk_data_packet_str); ++i)
	{
		const char *data_str = trk_data_packet_str[i];
		std::basic_string<BYTE> data = DataFromString(data_str);

		bool next = bufferCollector.PushData((const char *)data.data(), (int)data.size(), &packetCollector);
		if (!next)
			break;
	}

	assertTrue(packet.IsValid());
	assertIntegersEqual(Pid_Trk_Data, packet.GetPacketID());
	assertIntegersEqual(24, packet.GetPacketDataSize());
}

/** It fakes out a serial device.  You provide it with a list of data "chunks", formatted as hex pairs.
 * In response to a call to Read, it will return the next chunk in the list.
 */
class MockSerialDevice : public SerialDevice
{
	const char **m_data_str;	///< Source data.
	int m_data_str_count;		///< How much source data.

	int m_current_data_index;		///< Who gets fed over next?
	
	typedef std::basic_string<BYTE> bytes_t;
	bytes_t m_current_data;		///< The current "chunk" we're processing.

public:
	MockSerialDevice(const char *data_str[], int data_str_count)
		: m_data_str(data_str), m_data_str_count(data_str_count), m_current_data_index(0)
	{
	}

	virtual bool Write(const void *buffer, unsigned buffer_size, unsigned *bytes_written)
	{
		// We're faking Read, not Write, so we'll just return 'true' for this one.
		// At some point, we might have a MockGarminSerialDevice that actually watches the packets
		// being "written", and then fakes the responses when "Read" is called.

		// We do nothing, so pretend that it was written successfully.
		return true;
	}

	virtual bool Read(void *buffer, unsigned buffer_len, unsigned *bytes_read)
	{
		*bytes_read = 0;

		// Do we need to refresh our buffer?
		if (m_current_data.empty())
		{
			if (m_current_data_index >= m_data_str_count)
			{
				Sleep(5000);	// Induce an artificially large delay, so that the Unit Test can notice.
				return false;
			}

			m_current_data = DataFromString(m_data_str[m_current_data_index]);
			++m_current_data_index;
		}

		// Feed out some data.
		unsigned bytes = min(buffer_len, m_current_data.size());
		memcpy(buffer, m_current_data.data(), bytes);

		// push the data down.
		if (bytes < m_current_data.size())
			m_current_data.assign(m_current_data.begin() + bytes + 1, m_current_data.end());
		else
			m_current_data.clear();

		*bytes_read = bytes;
		return true;
	}
};

/** Hugo found a bug where if _exactly_ two packets arrive at once, the second
 * call to WaitForPacket doesn't flush it out.
 */
void testBasicLinkProtocol_Product_Data_eTrex()
{
	// This test data contains an ACK immediately followed by a Product_Data_Type.
	const char *etrex_product_data_packet_str[] = {
		"10 06 02 fe 00 fa 10 03 10 ff 6a a9 00 5e 01 65 "
		"54 72 65 78 20 56 69 73 74 61 20 53 6f 66 74 77 "
		"61 72 65 20 56 65 72 73 69 6f 6e 20 33 2e 35 30 "
		"00 56 45 52 42 4d 41 50 20 41 74 6c 61 6e 74 69 "
		"63 20 48 69 67 68 77 61 79 20 31 2e 30 30 00 56 "
		"45 52 53 4d 41 50 20 49 6e 74 65 72 6e 61 74 69 "
		"6f 6e 61 6c 20 4d 61 72 69 6e 65 20 50 4f 49 20 "
		"31 2e 30 30 00 e3 10 03 "
	};

	// The MockSerialDevice sleeps for 5 seconds if there's no more data left and it's presented with a Read call.
	// We can test for this here.  Normally, this code shouldn't take more than 2 seconds.
	DWORD startTime = GetTickCount();

	MockSerialDevice fake_serial(etrex_product_data_packet_str, COUNTOF(etrex_product_data_packet_str));

	BasicLinkProtocol protocol(&fake_serial);

	GarminPacket packet;
	assertTrue(protocol.WaitForPacket(&packet));
	assertTrue(protocol.WaitForPacket(&packet));

	DWORD endTime = GetTickCount();
	DWORD ticksTaken = endTime - startTime;

	/** @todo assertTimeTakenLt.  We'll need a "time" object that (in this case) holds the tick count. */
	assertTrue(ticksTaken <= 2000);
}

/** Push data into the system, and verify that the correct packets come out.
 * Needs extending to include corrupted packets.
 */
void testPacketCollector()
{
	testPacketCollector_Ack_Byte();
	testPacketCollector_Product_Data();
	testPacketCollector_Trk_Data();
	testPacketCollector_Trk_Data_Stuffed();

	/** @todo Some testing with packets spanning chunks, etc.?  The tests for BufferCollector should have caught
	 * that, but you never know.  Also check with a packet of more than 16 bytes that arrives in one go.
	 * The Geko 201 never seems to send us these, so we need to test it in case something else does.
	 */
}

void testSemicircleConversions()
{
	assertFloatsEqual(180, DegreesFromSemicircles(2147483647), FLT_EPSILON);
	assertFloatsEqual(-180, DegreesFromSemicircles(-2147483647), FLT_EPSILON);
	assertFloatsEqual(90, DegreesFromSemicircles(1073741824), FLT_EPSILON);
	assertFloatsEqual(-90, DegreesFromSemicircles(-1073741824), FLT_EPSILON);
	assertFloatsEqual(45, DegreesFromSemicircles(536870912), FLT_EPSILON);
	assertFloatsEqual(-45, DegreesFromSemicircles(-536870912), FLT_EPSILON);
	assertFloatsEqual(0, DegreesFromSemicircles(0), FLT_EPSILON);

	assertIntegersEqual(2147483647, SemicirclesFromDegrees(+180));
	assertIntegersEqual(-2147483647, SemicirclesFromDegrees(-180));
	assertIntegersEqual(1073741823, SemicirclesFromDegrees(+90));
	assertIntegersEqual(-1073741823, SemicirclesFromDegrees(-90));
	assertIntegersEqual(536870911, SemicirclesFromDegrees(+45));
	assertIntegersEqual(-536870911, SemicirclesFromDegrees(-45));
	assertIntegersEqual(0, SemicirclesFromDegrees(0));
}

void testTimeConversions()
{
	time_t unix_time;
	struct tm *ptm;
	char temp[MAX_PATH];

	unix_time = UnixTimeFromGarminTime(0x15180); // Jan 1 1990 00:00:00
	ptm = gmtime(&unix_time);
	strftime(temp, MAX_PATH, "%b %d %Y %H:%M:%S", ptm);

	unix_time = UnixTimeFromGarminTime(0x2deff07d); // Jun 4 1994 03:09:49
	ptm = gmtime(&unix_time);
	strftime(temp, MAX_PATH, "%b %d %Y %H:%M:%S", ptm);
}

void testBasicLinkProtocol()
{
	testBasicLinkProtocol_Product_Data_eTrex();
}

/** Ad-hoc testing showed that DataFromString was broken. */
void testDataFromString()
{
	const char *str1 = "10 06 02 fe 00 fa 10 03 10 ff 6a a9 00 5e 01 65 "
					  "54 72 65 78 20 56 69 73 74 61 20 53 6f 66 74 77";

	std::basic_string<BYTE> data1 = DataFromString(str1);
	assertIntegersEqual(32, (int)data1.size());

	const char *str2 = "00 56 45 52 42 4d 41 50 20 41 74 6c 61 6e 74 69 "
					   "63 20 48 69 67 68 77 61 79 20 31 2e 30 30 00 56 ";

	std::basic_string<BYTE> data2 = DataFromString(str2);
	assertIntegersEqual(32, (int)data2.size());
}

int runTests_lib_garmin()
{
	testDataFromString();

	testSemicircleConversions();
	testTimeConversions();
	testPacketCollector();
	testBasicLinkProtocol();

	return assertSummarise();
}

