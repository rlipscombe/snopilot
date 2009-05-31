#pragma once

#include "status.h"
#include "PositionFix.h"
#include "Waypoint.h"

class ImportSerialSettings;
class ImportSerialTaskObserver;
struct Product_Data_Type;

class ImportSerialTask
{
	const ImportSerialSettings *m_pSettings;
	ImportSerialTaskObserver *m_pObserver;

public:
	ImportSerialTask(const ImportSerialSettings *pSettings, ImportSerialTaskObserver *pObserver)
		: m_pSettings(pSettings), m_pObserver(pObserver)
	{
	}

	void Run();

private:
	inline void ReportBegin();
	inline void ReportTrackLogProgress(int current, int maximum);
	inline void ReportTrackLogComplete();
	inline void ReportWaypointProgress(int current, int maximum);
	inline void ReportWaypointComplete();
	inline void ReportComplete(STATUS taskStatus, const Product_Data_Type *product, const PositionFixCollection *positions, const WaypointCollection *waypoints);

	inline bool QueryCancelTask();

	friend class TrackLogProgress;
	friend class WaypointProgress;
};

class ImportSerialTaskObserver
{
public:
	virtual void OnTaskBegin() = 0;
	virtual void OnTrackLogProgress(int current, int maximum) = 0;
	virtual void OnTrackLogComplete() = 0;
	virtual void OnWaypointProgress(int current, int maximum) = 0;
	virtual void OnWaypointComplete() = 0;
	virtual void OnTaskComplete(STATUS taskStatus, const Product_Data_Type *product, const PositionFixCollection *positions, const WaypointCollection *waypoints) = 0;

	virtual bool QueryCancelTask() = 0;
};

inline void ImportSerialTask::ReportBegin()
{
	if (m_pObserver)
		m_pObserver->OnTaskBegin();
}

inline void ImportSerialTask::ReportTrackLogProgress(int current, int maximum)
{
	if (m_pObserver)
		m_pObserver->OnTrackLogProgress(current, maximum);
}

inline void ImportSerialTask::ReportTrackLogComplete()
{
	if (m_pObserver)
		m_pObserver->OnTrackLogComplete();
}

inline void ImportSerialTask::ReportWaypointProgress(int current, int maximum)
{
	if (m_pObserver)
		m_pObserver->OnWaypointProgress(current, maximum);
}

inline void ImportSerialTask::ReportWaypointComplete()
{
	if (m_pObserver)
		m_pObserver->OnWaypointComplete();
}

inline void ImportSerialTask::ReportComplete(STATUS taskStatus, const Product_Data_Type *product, const PositionFixCollection *positions, const WaypointCollection *waypoints)
{
	if (m_pObserver)
		m_pObserver->OnTaskComplete(taskStatus, product, positions, waypoints);
}

inline bool ImportSerialTask::QueryCancelTask()
{
	if (m_pObserver)
		return m_pObserver->QueryCancelTask();

	// No observer => no way to cancel the ImportSerialTask.
	return false;
}
