#include "config.h"
#include "trace.h"
#include "serial_device.h"
#include <stdio.h>

#define TRACE_SERIAL 0

bool Win32SerialDevice::Open(const TCHAR *serial_device)
{
	ASSERT(m_hSerial == INVALID_HANDLE_VALUE);

	// Open the serial device
	m_hSerial = CreateFile(serial_device, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if (m_hSerial == INVALID_HANDLE_VALUE)
	{
		TRACEC(TRACE_SERIAL, "CreateFile(%s) failed, error = %d.\n", serial_device, GetLastError());
		return false;
	}

	// Configure the serial timeouts.  Otherwise it appears to use 100% CPU, presumably because it spends
	// all of its time cycling through ReadFile with zero bytes until we've actually got a full packet.
	// See http://empeg.comms.net/php/showthreaded.php?Cat=&Board=hackers_prog&Number=20464
	// for details.
	COMMTIMEOUTS commTimeouts;
	commTimeouts.ReadIntervalTimeout = 0x10;
	commTimeouts.ReadTotalTimeoutMultiplier = 0;
	commTimeouts.ReadTotalTimeoutConstant = 0;
	commTimeouts.WriteTotalTimeoutMultiplier = 0;
	commTimeouts.WriteTotalTimeoutConstant = 0x5000;

	SetCommTimeouts(m_hSerial, &commTimeouts);
	
	DCB dcb;
	BOOL result = GetCommState(m_hSerial, &dcb);
	if (!result)
	{
		TRACEC(TRACE_SERIAL, "GetCommState failed, error %d.\n", GetLastError());
		return false;
	}

	dcb.BaudRate = CBR_9600;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;

	result = SetCommState(m_hSerial, &dcb);
	if (!result)
	{
		TRACEC(TRACE_SERIAL, "SetCommState failed, error = %d.\n", GetLastError());
		return false;
	}

	DWORD eventMask = EV_RXCHAR;
	result = SetCommMask(m_hSerial, eventMask);
	if (!result)
	{
		TRACEC(TRACE_SERIAL, "SetCommMask failed, error = %d.\n", GetLastError());
		return false;
	}

	ASSERT(m_ol.hEvent == NULL);
	m_ol.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	return true;
}

void Win32SerialDevice::Close()
{
	if (m_hSerial != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hSerial);
		m_hSerial = INVALID_HANDLE_VALUE;
	}

	if (m_ol.hEvent != NULL)
	{
		CloseHandle(m_ol.hEvent);
		m_ol.hEvent = NULL;
	}
}

bool Win32SerialDevice::Read(void *buffer, unsigned buffer_len, unsigned *bytes_read)
{
#if 0
	DWORD events = 0;
	BOOL result = WaitCommEvent(m_hSerial, &events, NULL);
	if (!result)
	{
		TRACEC(TRACE_SERIAL, "WaitCommEvent failed, error = %d.\n", GetLastError());
		return false;
	}
#endif

	BOOL result = ReadFile(m_hSerial, buffer, buffer_len, (DWORD *)bytes_read, &m_ol);
	if (result)
	{
		// ReadFile completed immediately.
//		TRACEC(TRACE_SERIAL, "Read %d byte(s)\n", *bytes_read);
		return true;
	}
	else if (GetLastError() == ERROR_IO_PENDING)
	{
		// ReadFile didn't complete immediately.  We'll need to wait for the results.
		const DWORD READ_TIMEOUT_MS = 1000;
		DWORD waitResult = WaitForSingleObject(m_ol.hEvent, READ_TIMEOUT_MS);
		switch (waitResult)
		{
		case WAIT_OBJECT_0:
			{
				ResetEvent(m_ol.hEvent);

				// Supposedly, we can skip the WaitForSingleObject call and pass TRUE
				// as the last parameter here.
				BOOL overlappedResult = GetOverlappedResult(m_hSerial, &m_ol, (DWORD *)bytes_read, FALSE);
				if (!overlappedResult)
				{
					TRACEC(TRACE_SERIAL, "GetOverlappedResult failed.\n");
					return false;
				}
				else
				{
//					TRACEC(TRACE_SERIAL, "Read %d byte(s)\n", *bytes_read);
					return true;
				}
			}
			break;

		case WAIT_TIMEOUT:
			{
				TRACEC(TRACE_SERIAL, "WaitForSingleObject timed out.\n");
				return false;
			}
			break;

		default:
			{
				// There's something wrong with the event handle.
				TRACEC(TRACE_SERIAL, "WaitForSingleObject failed.\n");
				return false;
			}
		}
	}
	else
	{
		// Some kind of error.
		TRACEC(TRACE_SERIAL, "ReadFile failed, error = %d.\n", GetLastError());
		return false;
	}
}


bool Win32SerialDevice::Write(const void *buffer, unsigned buffer_len, unsigned *bytes_written)
{
	BOOL result = WriteFile(m_hSerial, buffer, buffer_len, (DWORD *)bytes_written, &m_ol);
	if (result)
	{
		// WriteFile completed immediately.
		TRACEC(TRACE_SERIAL, "Wrote %d byte(s)\n", *bytes_written);
		return true;
	}
	else if (GetLastError() == ERROR_IO_PENDING)
	{
		// ReadFile didn't complete immediately.  We'll need to wait for the results.
		const DWORD WRITE_TIMEOUT_MS = 1000;
		DWORD waitResult = WaitForSingleObject(m_ol.hEvent, WRITE_TIMEOUT_MS);
		switch (waitResult)
		{
		case WAIT_OBJECT_0:
			{
				ResetEvent(m_ol.hEvent);

				// Supposedly, we can skip the WaitForSingleObject call and pass TRUE
				// as the last parameter here.
				BOOL overlappedResult = GetOverlappedResult(m_hSerial, &m_ol, (DWORD *)bytes_written, FALSE);
				if (!overlappedResult)
				{
					TRACEC(TRACE_SERIAL, "GetOverlappedResult failed.\n");
					return false;
				}
				else
				{
					TRACEC(TRACE_SERIAL, "Wrote %d byte(s)\n", *bytes_written);
					return true;
				}
			}
			break;

		case WAIT_TIMEOUT:
			{
				TRACEC(TRACE_SERIAL, "WaitForSingleObject timed out.\n");
				return false;
			}
			break;

		default:
			{
				// There's something wrong with the event handle.
				TRACEC(TRACE_SERIAL, "WaitForSingleObject failed.\n");
				return false;
			}
		}
	}
	else
	{
		// Some kind of error.
		TRACEC(TRACE_SERIAL, "WriteFile failed, error = %d.\n", GetLastError());
		return false;
	}

}