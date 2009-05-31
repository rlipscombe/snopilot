/* buffer_collector.cpp
 *
 * Copyright (c) 2003 Snopilot, http://www.snopilot.com/
 */

#include "config.h"
#include "trace.h"
#include "buffer_collector.h"
#include <stdlib.h>

BufferCollector::~BufferCollector()
{
	free(m_buffer);
}

/** Push some data into the buffer, and then process it. */
bool BufferCollector::PushData(const char *data, int data_size, BufferCollectorCallback *cb)
{
//	TRACE_HEX("BufferCollector::PushData()\n", data, data + data_size);
	
	if (!ExpandBuffer(data_size))
		return false;

	if (!AppendData(data, data_size))
		return false;

	return ProcessBuffer(cb);
}

bool BufferCollector::Flush(BufferCollectorCallback *cb)
{
	return ProcessBuffer(cb);
}

/** Do we need to expand the buffer? */
bool BufferCollector::ExpandBuffer(int data_size)
{
	int required_size = m_buffer_used + data_size;
	if (required_size > m_buffer_alloc)
	{
		char *new_buffer = (char *)realloc(m_buffer, required_size);
		ASSERT(new_buffer);
		if (!new_buffer)
			return false;

		m_buffer = new_buffer;
		m_buffer_alloc = required_size;
	}

	return true;
}

/** Put the new data on the end of the buffer. */
bool BufferCollector::AppendData(const char *data, int data_size)
{
	memcpy(m_buffer + m_buffer_used, data, data_size);
	m_buffer_used += data_size;

	return true;
}

/** See if the callback wants to take any data out of the buffer.
 * Used from PushData (for when new data arrives), and from Flush (when we want to get rid of old data).
 */
bool BufferCollector::ProcessBuffer(BufferCollectorCallback *cb)
{
	// Since the callback isn't allowed to eat more data than is present, this is a safe sentinel value.
	int bytes_eaten = m_buffer_used + 1;
	bool result = cb->ProcessBuffer(m_buffer, m_buffer_used, &bytes_eaten);

	// Check that the callback didn't forget to set this.
	ASSERT(bytes_eaten <= m_buffer_used);

	PushBufferDown(bytes_eaten);
	return result;
}

/** Push any remaining data back down in the buffer. */
void BufferCollector::PushBufferDown(int bytes_eaten)
{
	if (bytes_eaten)
	{
		int buffer_remaining = m_buffer_used - bytes_eaten;
		if (buffer_remaining)
			memmove(m_buffer, m_buffer + bytes_eaten, buffer_remaining);
		m_buffer_used = buffer_remaining;
	}
}

#ifdef _DEBUG
bool DebugBufferCollectorCallback::ProcessBuffer(const char *buffer, int buffer_size, int *bytes_eaten)
{
	TRACE_HEX("buffer:\n", buffer, buffer + buffer_size);
	*bytes_eaten = buffer_size;
	return true;	// Want more.
}
#endif
