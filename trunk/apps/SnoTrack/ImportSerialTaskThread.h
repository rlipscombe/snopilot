#include "thread/thread.h"

class ImportSerialSettings;
class ImportSerialTaskObserver;

class ImportSerialTaskThread : public Thread
{
	const ImportSerialSettings *m_pSettings;
	ImportSerialTaskObserver *m_pObserver;

public:
	ImportSerialTaskThread(const ImportSerialSettings *pSettings, ImportSerialTaskObserver *pObserver)
		: m_pSettings(pSettings), m_pObserver(pObserver)
	{
	}

    virtual unsigned Run();
};
