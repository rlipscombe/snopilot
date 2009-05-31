/* line_collector.h
 *
 * Copyright (c) 2003 Snopilot, http://www.snopilot.com/
 */

#ifndef LINE_COLLECTOR_H
#define LINE_COLLECTOR_H 1

#include "buffer_collector.h"
#include <string>

class LineCollectorCallback;

class LineCollector : public BufferCollectorCallback
{
	LineCollectorCallback *m_cb;

public:
	explicit LineCollector(LineCollectorCallback *cb)
		: m_cb(cb)
	{
	}

	virtual bool ProcessBuffer(const char *buffer, int buffer_size, int *bytes_eaten);

private:
	bool FireLine(const std::string &line);
};

class LineCollectorCallback
{
public:
	virtual bool OnLine(const std::string &line) = 0;
};

/** Just dumps the line to the screen. */
class DebugLineCollectorCallback : public LineCollectorCallback
{
public:
	virtual bool OnLine(const std::string &line)
	{
		TRACE(">%s<\n", line.c_str());
		return true;
	}
};

/** Does nothing. */
class NullLineCollectorCallback : public LineCollectorCallback
{
public:
	virtual bool OnLine(const std::string &line)
	{
		return true;
	}
};

#endif /* LINE_COLLECTOR_H */
