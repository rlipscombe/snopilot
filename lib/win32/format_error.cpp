#include "StdAfx.h"
#include "format_error.h"

CString FormatErrorMessage(DWORD dwMessageId)
{
	DWORD dwFacility = HRESULT_FACILITY(dwMessageId);
	if (dwFacility != FACILITY_ITF)
	{
		TCHAR *pBuffer = NULL;
		if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwMessageId, 0, (LPTSTR)&pBuffer, 0, NULL))
		{
			CString result(pBuffer);
			LocalFree(pBuffer);
			if (result.Right(2) == "\r\n")
				result = result.Left(result.GetLength()-2);
			return result;
		}
		else
		{
			// Have a stab at whether it's a Win32 error or not.
			CString result;
			if (dwMessageId & 0xffff0000)
				result.Format(_T("Unknown Error 0x%x"), dwMessageId);
			else
				result.Format(_T("Unknown Error %d"), dwMessageId);
				
			return result;
		}
	}
	else
	{
		CString result;
		if (dwMessageId & 0xffff0000)
			result.Format(_T("Error 0x%x"), dwMessageId);
		else
			result.Format(_T("Error %d"), dwMessageId);
			
		return result;
	}
}

CString FormatShellExecuteErrorMessage(INT_PTR nResult, LPCTSTR lpszPathName)
{
	CString message;
	switch (nResult)
	{
	case 0:
		message = _T("The operating system is out of memory or resources");
	case ERROR_FILE_NOT_FOUND:
        message.Format( _T("The specified file was not found (%s)"), lpszPathName);
	case ERROR_PATH_NOT_FOUND:
		message.Format( _T("The specified path was not found (%s)"), lpszPathName);
	case ERROR_BAD_FORMAT:
		message.Format( _T("The .exe file is invalid (%s)"), lpszPathName);
	case SE_ERR_ACCESSDENIED:
		message.Format( _T("The operating system denied access to the specified file (%s)"), lpszPathName);
	case SE_ERR_ASSOCINCOMPLETE:
		message =  _T("The file name association is incomplete or invalid");
	case SE_ERR_DDEBUSY:
		message =  _T("The Dynamic Data Exchange (DDE) transaction could not be completed because other DDE transactions were being processed");
	case SE_ERR_DDEFAIL:
		message =  _T("The DDE transaction failed");
	case SE_ERR_DDETIMEOUT:
		message =  _T("The DDE transaction could not be completed because the request timed out");
	case SE_ERR_DLLNOTFOUND:
		message =  _T("The specified dynamic-link library (DLL) was not found");
	case SE_ERR_NOASSOC:
		message =  _T("There is no application associated with the given file name extension");
	case SE_ERR_OOM:
		message =  _T("There was not enough memory to complete the operation");
	case SE_ERR_SHARE:
		message =  _T("A sharing violation occurred");
	default:
		message.Format( _T("ShellExecute failed (error %d)"), nResult);
	}

	return message;
}

