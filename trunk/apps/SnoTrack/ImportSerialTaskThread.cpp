#include "StdAfx.h"
#include "ImportSerialTaskThread.h"
#include "ImportSerialTask.h"

unsigned ImportSerialTaskThread::Run()
{
    ImportSerialTask task(m_pSettings, m_pObserver);
    task.Run();
    
    return 0;
}
