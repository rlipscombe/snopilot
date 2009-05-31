#include "StdAfx.h"
#include "PostFileTaskThread.h"
#include "PostFileTask.h"

unsigned PostFileTaskThread::Run()
{
	PostFileTask task(m_pSettings, m_pResults, m_pObserver);
	task.Run();

	return 0;
}
