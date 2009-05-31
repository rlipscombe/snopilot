#pragma once
#if defined(SNOTRACK_EXPORT)
 #define SNOTRACK_API __declspec(dllexport)
#else
 #define SNOTRACK_API __declspec(dllimport)
#endif

#include "utf8.h"

/** Writing .snotrack files.
 *
 * It's a little involved (and, if it wasn't for speed/size requirements, wouldn't have to be):
 *
 * You instantiate a writer object (CSnoTrackFileWriter) and ask it to write the file.
 * Because it needs to get the data from somewhere (the document), you pass a callback pointer.
 * At the relevant points during the writing of the file, the callback will be asked for the data.
 * Where it gets a little more complicated is when the tracklog and waypoints need to be written.
 * Rather than ask for the whole lot as an array, the writer callback is given a pointer to
 * a TracklogWriterCallback object (in the case of the tracklog).  Each tracklog position should be
 * given to this object, which is responsible for getting them into the file.
 */
class CSnoTrackFileWriterCallback;

class SNOTRACK_API CSnoTrackFileWriter
{
public:
	void WriteFile(CFile *pFile, CSnoTrackFileWriterCallback *pCallback);
};

struct Product_Data_Type;

class CSnoTrackFileWriterCallback
{
public:
	virtual void GetProductData(Product_Data_Type *productData) = 0;

	virtual tstring GetResortFileName() = 0;
	virtual utf8_string GetCustomResortName() = 0;

	virtual utf8_string GetComments() = 0;

	/** Writing Tracklogs: To allow writing the tracklog, there's a collaboration between your callback and the writer's callback.
	 * Your callback should implement the two GetTracklog methods.  The first will be called when the writer
	 * needs to know how many positions are in the tracklog.  The second will be called when it needs to write the positions.
	 * You will call the TracklogWriterCallback once for each tracklog position.
	 */
	class TracklogWriterCallback
	{
	public:
		virtual void WritePositionFix(FILETIME timestamp, const char *trk_ident, bool is_new, double lon_deg, double lat_deg, double gps_elev) = 0;
	};

	virtual DWORD GetTracklogSize() = 0;
	virtual void GetTracklog(TracklogWriterCallback *pCallback) = 0;

	/** Similarly for waypoints:
	 */
	class WaypointWriterCallback
	{
	public:
		virtual void WriteWaypoint(const char *label, double lon_deg, double lat_deg) = 0;
	};

	virtual DWORD GetWaypointCount() = 0;
	virtual void GetWaypoints(WaypointWriterCallback *pCallback) = 0;
};

