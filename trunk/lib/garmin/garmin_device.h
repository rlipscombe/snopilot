/* garmin_device.h
 *
 * Copyright (C) 2003 Snopilot, http://www.snopilot.com/
 */

#ifndef GARMIN_DEVICE_H
#define GARMIN_DEVICE_H

#include "status.h"

class BasicLinkProtocol;
struct Product_Data_Type;
class TrackLogCallback;
class WaypointCallback;
class Progress;

struct D310_Trk_Hdr_Type;
struct D301_Trk_Point_Type;

struct D108_Wpt_Type;

class GarminDevice
{
	BasicLinkProtocol *m_protocol;

public:
	GarminDevice(BasicLinkProtocol *protocol)
		: m_protocol(protocol)
	{
	}

	STATUS GetProductData(Product_Data_Type *productData);
	STATUS GetTrackLogs(TrackLogCallback *pCallback, Progress *pProgress);
	STATUS GetWaypoints(WaypointCallback *pCallback, Progress *pProgress);
};

/** Track logs come back from the device as a sequence of Trk_Hdr and Trk_Data packets.
 * Each new track starts with a Trk_Hdr packet, and is followed by the Trk_Data packets
 * that make up that track.
 */
class TrackLogCallback
{
public:
	virtual void OnTrackHeader(const D310_Trk_Hdr_Type &trk_hdr) = 0;
	virtual void OnTrackPoint(const D301_Trk_Point_Type &trk_point) = 0;
};

class WaypointCallback
{
public:
	virtual void OnWaypoint(const D108_Wpt_Type &wpt) = 0;
};

#endif /* GARMIN_DEVICE_H */
