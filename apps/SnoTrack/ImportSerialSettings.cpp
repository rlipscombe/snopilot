#include "StdAfx.h"
#include "ImportSerialSettings.h"
#include "win32/ApplicationProfile.h"

void ImportSerialSettings::Load(ApplicationProfile *pProfile)
{
	m_serialDevice = pProfile->GetString(_T("Serial Import"), _T("Device"), _T("COM1"));
}

void ImportSerialSettings::Save(ApplicationProfile *pProfile)
{
	pProfile->WriteString(_T("Serial Import"), _T("Device"), m_serialDevice.c_str());
}

void ImportSerialResults::SetProductData(const Product_Data_Type &product)
{
	m_product = product;
}

void ImportSerialResults::GetProductData(Product_Data_Type *product)
{
	*product = m_product;
}

void ImportSerialResults::ClearPositions()
{
	m_positions.clear();
}

void ImportSerialResults::AddPositions(const PositionFixCollection &positions)
{
	m_positions.reserve(positions.size());
	std::copy(positions.begin(), positions.end(), std::back_inserter(m_positions));
}

void ImportSerialResults::GetPositions(PositionFixCollection *positions) const
{
	positions->reserve(m_positions.size());
	std::copy(m_positions.begin(), m_positions.end(), std::back_inserter(*positions));
}

int ImportSerialResults::GetPositionCount() const
{
	return (int)m_positions.size();
}

void ImportSerialResults::ClearWaypoints()
{
	m_waypoints.clear();
}

void ImportSerialResults::AddWaypoints(const WaypointCollection &waypoints)
{
	m_waypoints.reserve(waypoints.size());
	std::copy(waypoints.begin(), waypoints.end(), std::back_inserter(m_waypoints));
}

void ImportSerialResults::GetWaypoints(WaypointCollection *waypoints) const
{
	waypoints->reserve(m_waypoints.size());
	std::copy(m_waypoints.begin(), m_waypoints.end(), std::back_inserter(*waypoints));
}

int ImportSerialResults::GetWaypointCount() const
{
	return (int)m_waypoints.size();
}
