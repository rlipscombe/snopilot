#ifndef SERIAL_DEVICE_H
#define SERIAL_DEVICE_H

class SerialDevice
{
public:
	virtual bool Write(const void *buffer, unsigned buffer_size, unsigned *bytes_written) = 0;
	virtual bool Read(void *buffer, unsigned buffer_len, unsigned *bytes_read) = 0;
};

class Win32SerialDevice : public SerialDevice
{
	HANDLE m_hSerial;
	OVERLAPPED m_ol;

public:
	Win32SerialDevice()
		: m_hSerial(INVALID_HANDLE_VALUE)
	{
		memset(&m_ol, 0, sizeof(OVERLAPPED));
	}

	~Win32SerialDevice()
	{
		Close();
	}

	bool Open(const TCHAR *serial_device);
	void Close();

public:
	virtual bool Write(const void *buffer, unsigned buffer_size, unsigned *bytes_written);
	virtual bool Read(void *buffer, unsigned buffer_len, unsigned *bytes_read);
};

#endif /* GARMIN_DEVICE_H */
