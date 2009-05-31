#include "config.h"
#include "trace.h"

#include "Thread.h"
#include <process.h>	// for _beginthreadex

bool Thread::Start()
{
	unsigned threadId;
	HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, StaticThreadRoutine,
		this, CREATE_SUSPENDED, &threadId);
	if (!hThread)
		return false;

	m_hThread = hThread;
	m_idThread = threadId;
	ResumeThread(hThread);

	return true;
}

unsigned Thread::Join()
{
	WaitForSingleObject(m_hThread, INFINITE);

	DWORD result;
	GetExitCodeThread(m_hThread, &result);

	return result;
}
