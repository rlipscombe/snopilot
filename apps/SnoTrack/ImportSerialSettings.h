#pragma once

#include <string>
#include "safe_snprintf.h"
#include "status.h"
#include "PositionFix.h"
#include "Waypoint.h"
#include "garmin/garmin_packet_ids.h"

class ApplicationProfile;
class Projection;

/** As the user goes through the Import Serial wizard, settings are accumulated
 * in here.
 */
class ImportSerialSettings
{
	tstring m_serialDevice;
	Projection *m_proj;

public:
	ImportSerialSettings(Projection *proj)
		: m_serialDevice(_T("COM1")), m_proj(proj)
	{
	}

	void SetSerialDevice(const tstring &device)
	{
		m_serialDevice = device;
	}

	/// Get the device name in a user-readable form (i.e. 'COM1').
	tstring GetUserSerialDeviceName() const
	{
		return m_serialDevice;
	}

	tstring GetSerialDevice() const
	{
		TCHAR temp[MAX_PATH];
		safe_sntprintf(temp, MAX_PATH, _T("\\\\.\\%s"), m_serialDevice.c_str());
		return tstring(temp);
	}

	void Load(ApplicationProfile *pProfile);
	void Save(ApplicationProfile *pProfile);
};

/** The results go in here.
 */
class ImportSerialResults
{
	STATUS m_taskStatus;		///< What happened?
	Product_Data_Type m_product;
	PositionFixCollection m_positions;	///< Tracklog positions go here.
	WaypointCollection m_waypoints;	///< Waypoints go here.

public:
	ImportSerialResults()
		: m_taskStatus(S_OK)
	{
	}

	void SetTaskStatus(STATUS taskStatus) { m_taskStatus = taskStatus; }
	STATUS GetTaskStatus() const { return m_taskStatus; }

	void SetProductData(const Product_Data_Type &product);
	void GetProductData(Product_Data_Type *product);

	void ClearPositions();
	void AddPositions(const PositionFixCollection &positions);
	void GetPositions(PositionFixCollection *positions) const;
	int GetPositionCount() const;

	void ClearWaypoints();
	void AddWaypoints(const WaypointCollection &waypoints);
	void GetWaypoints(WaypointCollection *waypoints) const;
	int GetWaypointCount() const;
};