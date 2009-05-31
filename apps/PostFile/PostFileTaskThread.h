#pragma once
#include "thread/Thread.h"

class PostFileSettings;
class PostFileResults;
class PostFileTaskObserver;

class PostFileTaskThread : public Thread
{
	PostFileSettings *m_pSettings;
	PostFileResults *m_pResults;
	PostFileTaskObserver *m_pObserver;

public:
	PostFileTaskThread(PostFileSettings *pSettings, PostFileResults *pResults, PostFileTaskObserver *pObserver)
		: m_pSettings(pSettings), m_pResults(pResults), m_pObserver(pObserver)
	{
	}

	virtual unsigned Run();
};

