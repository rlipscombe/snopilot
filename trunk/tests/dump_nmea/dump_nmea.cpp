/* dump_nmea.cpp
 *
 * Copyright (C) 2003 Snopilot, http://www.snopilot.com/
 *
 * Authors:
 *   Roger Lipscombe <roger@snopilot.com>
 */

#include "config.h"
#include "trace.h"
#include <stdio.h>
#include "buffer_collector.h"
#include "nmea_sentence_collector.h"

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		fprintf(stderr, "Usage:\n\tgpscap serial-port\n");
		return 1;
	}

	char serial_device[MAX_PATH];
	sprintf(serial_device, "\\\\.\\%s", argv[1]);

	// Open the serial device
	HANDLE hSerial = CreateFile(serial_device, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hSerial == INVALID_HANDLE_VALUE)
	{
		fprintf(stderr, "CreateFile(%s) failed, error = %d.\n", serial_device, GetLastError());
		return 1;
	}

	DCB dcb;
	BOOL result = GetCommState(hSerial, &dcb);
	if (!result)
	{
		fprintf(stderr, "GetCommState failed, error %d.\n", GetLastError());
		return 1;
	}

	dcb.BaudRate = CBR_4800;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;

	result = SetCommState(hSerial, &dcb);
	if (!result)
	{
		fprintf(stderr, "SetCommState failed, error = %d.\n", GetLastError());
		return 1;
	}

	DWORD eventMask = EV_RXCHAR;
	result = SetCommMask(hSerial, eventMask);
	if (!result)
	{
		fprintf(stderr, "SetCommMask failed, error = %d.\n", GetLastError());
		return 1;
	}

	// Prevent the computer from powering off:
	EXECUTION_STATE oldExecutionState = SetThreadExecutionState(ES_SYSTEM_REQUIRED | ES_CONTINUOUS);
	
	NmeaSentenceCollector cb;
	BufferCollector collector;
	for (;;)
	{
		DWORD events = 0;
		result = WaitCommEvent(hSerial, &events, NULL);
		if (!result)
		{
			fprintf(stderr, "WaitCommEvent failed, error = %d.\n", GetLastError());
			return 1;
		}

		char buffer[1025];
		int buffer_len = 1024;	// Allow space for the zero terminator.
		DWORD bytesRead;
		
		result = ReadFile(hSerial, buffer, buffer_len, &bytesRead, NULL);
		if (!result)
		{
			fprintf(stderr, "ReadFile failed, error = %d.\n", GetLastError());
			return 1;
		}

		if (bytesRead != 0)
		{
            buffer[bytesRead] = '\0';
//			printf("%d bytes: %s\n", bytesRead, buffer);

			collector.PushData(buffer, bytesRead, &cb);
		}
	}

	CloseHandle(hSerial);

	return 0;
}
