class Thread
{
	HANDLE m_hThread;
	UINT m_idThread;

public:
	Thread()
		: m_hThread(NULL), m_idThread(0)
	{
	}

	virtual ~Thread()
	{
		CloseHandle(m_hThread);
	}

	bool Start();
	unsigned Join();

protected:
	virtual unsigned Run() = 0;

private:
	static unsigned __stdcall StaticThreadRoutine(void *pParams)
	{
		return static_cast<Thread *>(pParams)->Run();
	}
};

