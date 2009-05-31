/* file.cpp
 */

#include "config.h"
#include "trace.h"
#include "file.h"

tstring GetTemporaryFileName(const TCHAR *pszPrefix)
{
	TCHAR temp_path[MAX_PATH];
	GetTempPath(MAX_PATH, temp_path);

	TCHAR temp_file[MAX_PATH];
	GetTempFileName(temp_path, pszPrefix, 0, temp_file);

	return tstring(temp_file);
}
