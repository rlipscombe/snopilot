/* buffer_collector.h
 *
 * Copyright (c) 2003 Snopilot, http://www.snopilot.com/
 */

#ifndef BUFFER_COLLECTOR_H
#define BUFFER_COLLECTOR_H 1

class BufferCollectorCallback;

/** BufferCollector is used when waiting for variable-sized packets over some sort of communications stream.
 * For example, you might use it for reading binary packets from a serial port.
 *
 * It is used to accumulate data until you have a full packet (or line or whatever).
 *
 * To use:
 *    For each chunk of data read from the stream, call PushData, passing the data and a callback.
 *    Your callback will be called to process the buffer.
 *    Typically, you'll walk through the buffer, looking for well-formed packets.
 *    You should set *bytes_eaten to control how much of the buffer is discarded.  Any remaining data in the buffer is kept
 *    until the next call to PushData, allowing it to cope with [packet][packet][pack], when [et] arrives later.
 *    Your return value is returned from PushData.  Generally, you'll return true to keep going, false to stop processing.
 *    Whether the data that caused you to return false remains in the buffer or not is controlled by *bytes_eaten.
 *    If you want to process multiple packets in a single call (assuming there's more than one in the buffer), you'll
 *    need to implement your own loop around PushData.
 *
 * @todo Change the semantics slightly -- We've had to introduce Flush to push data through the buffer when there's no more arriving
 * (because the outside user has to supply the loop).  If we provide an inner loop until the callback returns false, it's easier for
 * the user to deal with.  It won't fix the problem with the GarminPacketCollector, because that only ever picks up a single packet
 * (it always returns false once it's seen a complete packet).
 */
class BufferCollector
{
	char *m_buffer;
	int m_buffer_alloc;

	int m_buffer_used;

public:
	BufferCollector()
		: m_buffer(NULL), m_buffer_alloc(0), m_buffer_used(0)
	{
	}

	~BufferCollector();

	bool PushData(const char *data, int data_size, BufferCollectorCallback *cb);
	bool Flush(BufferCollectorCallback *cb);

private:
	bool ExpandBuffer(int data_size);
	bool AppendData(const char *data, int data_size);
	bool ProcessBuffer(BufferCollectorCallback *cb);
	void PushBufferDown(int bytes_eaten);
};

class BufferCollectorCallback
{
public:
	virtual bool ProcessBuffer(const char *buffer, int buffer_size, int *bytes_eaten) = 0;
};

#ifdef _DEBUG
class DebugBufferCollectorCallback : public BufferCollectorCallback
{
// BufferCollectorCallback
public:
	virtual bool ProcessBuffer(const char *buffer, int buffer_size, int *bytes_eaten);
};
#endif

#endif /* BUFFER_COLLECTOR_H */
