/* garmin_device.cpp
 *
 * Copyright (C) 2003 Snopilot, http://www.snopilot.com/
 */

#include "config.h"
#include "trace.h"
#include "garmin_device.h"
#include "basic_link_protocol.h"
#include "garmin_packet_ids.h"
#include "garmin_packet_builder.h"
#include "progress.h"
#include <time.h>

STATUS GarminDevice::GetProductData(Product_Data_Type *productData)
{
	TRACE("GarminDevice::GetProductData\n");

	// Poke the device by sending it a Pid_Product_Rqst packet:
	GarminPacketBuilder builder(Pid_Product_Rqst);
	m_protocol->SendPacket(builder.Finish());
	if (!m_protocol->WaitForAck())
		return E_FAIL;

	// The next packet is the one we're after.
	GarminPacket packet;
	if (!m_protocol->WaitForPacket(&packet))
		return E_FAIL;

	TRACE("  Pid_Product_Data\n");

	// The packet_data in one of these is a Product_Data_Type structure.
	packet.ReadInteger(&productData->product_id);
	packet.ReadInteger(&productData->software_version);

	// This is then followed by a null-terminated string, containing the product description.
	packet.ReadString(&productData->product_description);

	// This is then followed by one or more null-terminated strings.
	std::string additional_string;
	while (packet.ReadString(&additional_string))
		productData->additional_strings.push_back(additional_string);

	m_protocol->SendAck(packet.GetPacketID());

	// If the device supports A001 - Protocol Capability Protocol,
	// then it will send a Pid_Protocol_Array packet immediately after
	// completion of the Product Data Protocol (A000).

	// The Geko 201 doesn't send anything.

	return S_OK;
}

STATUS GarminDevice::GetTrackLogs(TrackLogCallback *pCallback, Progress *pProgress)
{
	TRACE("GarminDevice::GetTrackLogs\n");

	STATUS status = S_OK;
	time_t startTime = time(NULL);

	// To ask for the track log, we send a Pid_Command_Data, with the command ID set to Cmnd_Transfer_Trk.
	GarminPacketBuilder builder(Pid_Command_Data);
	builder.SetInteger(Cmnd_Transfer_Trk);
	GarminPacket packet = builder.Finish();
	m_protocol->SendPacket(packet);
//	m_protocol->WaitForAck();

	int track_count = 0;
	int track_total = 0;

	bool complete = false;
	while (!complete)
	{
		GarminPacket response;
		if (!m_protocol->WaitForPacket(&response))
		{
			status = E_FAIL;
			break;
		}

		if (!response.IsValid())
			break;

//		TRACE("Received packet\n");

		if (response.GetPacketID() == Pid_Ack_Byte)
		{
			TRACE("Pid_Ack_Byte\n");
		}
		else if (response.GetPacketID() == Pid_Records)
		{
			TRACE("Pid_Records\n");

			if (response.GetPacketDataSize() == 2)
			{
				Records_Type records;

				response.ReadInteger(&records.packet_count);

				TRACE("Records_Type:\n");
				TRACE("  records.packet_count = %d\n", records.packet_count);

				track_total = records.packet_count;
			}
			else
			{
				TRACE("Unknown size (%d) for Pid_Records\n", response.GetPacketDataSize());
				response.Dump();
			}
		}
		else if (response.GetPacketID() == Pid_Trk_Hdr)
		{
			TRACE("Pid_Trk_Hdr\n");
			status = pProgress->OnProgress(++track_count, track_total);
			if (FAILED(status))
				break;

			// We'll have to assume that the packet data is a D310_Trk_Hdr_Type, since we can't tell by size.
			D310_Trk_Hdr_Type trk_hdr;

			response.ReadBool(&trk_hdr.dspl);
			response.ReadByte(&trk_hdr.color);
			response.ReadString(&trk_hdr.trk_ident);

			pCallback->OnTrackHeader(trk_hdr);
		}
		else if (response.GetPacketID() == Pid_Trk_Data)
		{
//			TRACE("Pid_Trk_Data\n");
			status = pProgress->OnProgress(++track_count, track_total);
			if (FAILED(status))
				break;

			if (response.GetPacketDataSize() == 24)
			{
				// I think that the Geko 201 bangs out a D301_Trk_Point_Type for this:
				D301_Trk_Point_Type trk_point;

				response.ReadLong(&trk_point.posn.lat);
				response.ReadLong(&trk_point.posn.lon);
				response.ReadULong(&trk_point.time);
				response.ReadFloat(&trk_point.alt);
				response.ReadFloat(&trk_point.dpth);
				response.ReadBool(&trk_point.new_trk);

				pCallback->OnTrackPoint(trk_point);
			}
			else
			{
				TRACE("Unknown size (%d) for Pid_Trk_Data\n", response.GetPacketDataSize());
				response.Dump();
			}
		}
		else if (response.GetPacketID() == Pid_Xfer_Cmplt)
		{
			TRACE("Pid_Xfer_Cmplt\n");
			pProgress->OnProgress(track_total, track_total);

			int command_id;

			response.ReadInteger(&command_id);

			TRACE("  command_id = %d\n", command_id);

			complete = true;
		}
		else
		{
			TRACE("Unknown packet type (%d)\n", response.GetPacketID());
			response.Dump();
		}

		// Acknowledge that packet.
		m_protocol->SendAck(response.GetPacketID());
	}

	time_t endTime = time(NULL);

	time_t timeTaken = endTime - startTime;
	if (timeTaken == 0)
		timeTaken = 1;

	printf("\nRead %d of %d records in %d second(s).  That's %d records/sec\n", track_count, track_total, timeTaken, track_count / timeTaken);
	
	return status;
}

STATUS GarminDevice::GetWaypoints(WaypointCallback *pCallback, Progress *pProgress)
{
	TRACE("GarminDevice::GetWaypoints\n");

	STATUS status = S_OK;
	time_t startTime = time(NULL);

	// To ask for the track log, we send a Pid_Command_Data, with the command ID set to Cmnd_Transfer_Wpt.
	GarminPacketBuilder builder(Pid_Command_Data);
	builder.SetInteger(Cmnd_Transfer_Wpt);
	GarminPacket packet = builder.Finish();
	m_protocol->SendPacket(packet);
//	m_protocol->WaitForAck();

	int track_count = 0;
	int track_total = 0;

	bool complete = false;
	while (!complete)
	{
		GarminPacket response;
		if (!m_protocol->WaitForPacket(&response))
		{
			status = E_FAIL;
			break;
		}

		if (!response.IsValid())
			break;

//		TRACE("Received packet\n");

		if (response.GetPacketID() == Pid_Ack_Byte)
		{
			TRACE("Pid_Ack_Byte\n");
		}
		else if (response.GetPacketID() == Pid_Records)
		{
			TRACE("Pid_Records\n");

			if (response.GetPacketDataSize() == 2)
			{
				Records_Type records;

				response.ReadInteger(&records.packet_count);

				TRACE("Records_Type:\n");
				TRACE("  records.packet_count = %d\n", records.packet_count);

				track_total = records.packet_count;
			}
			else
			{
				TRACE("Unknown size (%d) for Pid_Records\n", response.GetPacketDataSize());
				response.Dump();
			}
		}
		else if (response.GetPacketID() == Pid_Wpt_Data)
		{
			TRACE("Pid_Wpt_Data\n");
			status = pProgress->OnProgress(++track_count, track_total);
			if (FAILED(status))
				break;

#if 0
			struct D100_Wpt_Type
			{
				char ident[6];
				Semicircle_Type posn;
				long unused;
				char cmnt[40];
			};

			// The response is variable length, so we need to know what we're plugged into in order to figure this out.
			D100_Wpt_Type wpt;

			response.ReadCharArray(wpt.ident, 6);
			response.ReadLong(&wpt.posn.lat);
			response.ReadLong(&wpt.posn.lon);
			response.ReadLong(&wpt.unused);
			response.ReadCharArray(wpt.cmnt, 40);

			TRACE("Is it D100?\n");
#else
			D108_Wpt_Type wpt;

			response.ReadByte(&wpt.wpt_class);
			response.ReadByte(&wpt.color);
			response.ReadByte(&wpt.dspl);
			response.ReadByte(&wpt.attr);
			response.ReadInteger(&wpt.smbl);

			// We're good to here.

			response.ReadCharArray(wpt.subclass, 18);
			response.ReadLong(&wpt.posn.lat);
			response.ReadLong(&wpt.posn.lon);
			response.ReadFloat(&wpt.alt);
			response.ReadFloat(&wpt.dpth);
			response.ReadFloat(&wpt.dist);
			response.ReadCharArray(wpt.state, 2);
			response.ReadCharArray(wpt.cc, 2);
			response.ReadString(&wpt.ident);
			response.ReadString(&wpt.comment);
			response.ReadString(&wpt.facility);
			response.ReadString(&wpt.city);
			response.ReadString(&wpt.addr);
			response.ReadString(&wpt.cross_road);

			pCallback->OnWaypoint(wpt);
#endif
		}
		else if (response.GetPacketID() == Pid_Xfer_Cmplt)
		{
			TRACE("Pid_Xfer_Cmplt\n");
			pProgress->OnProgress(track_total, track_total);

			int command_id;

			response.ReadInteger(&command_id);

			TRACE("  command_id = %d\n", command_id);

			complete = true;
		}
		else
		{
			TRACE("Unknown packet type (%d)\n", response.GetPacketID());
			response.Dump();
		}

		// Acknowledge that packet.
		m_protocol->SendAck(response.GetPacketID());
	}

	time_t endTime = time(NULL);

	time_t timeTaken = endTime - startTime;
	if (timeTaken == 0)
		timeTaken = 1;

	printf("\nRead %d of %d records in %d second(s).  That's %d records/sec\n", track_count, track_total, timeTaken, track_count / timeTaken);
	
	return status;
}