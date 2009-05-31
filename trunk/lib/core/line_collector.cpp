/* line_collector.cpp
 *
 * Copyright (c) 2003 Snopilot, http://www.snopilot.com/
 */

#include "config.h"
#include "trace.h"
#include "line_collector.h"

/** Now, look through the combined buffer to see if there are any line breaks in there. */
bool LineCollector::ProcessBuffer(const char *buffer, int buffer_size, int *bytes_eaten)
{
	bool result = true;

	const char *buffer_end = buffer + buffer_size;
	const char *b = buffer;
	const char *e;
	while((e = (const char *)memchr(b, '\n', buffer_end - b)) != NULL)
	{
		std::string line(b, e);
		b = e + 1;

		result = FireLine(line);
		if (!result)
			break;
	}

	*bytes_eaten = (int)(b - buffer);
	return result;
}

/** Remove Trailing CR. */
bool LineCollector::FireLine(const std::string &line)
{
	char last = *line.rbegin();
	if (last == '\r')
		return m_cb->OnLine(std::string(line.begin(), line.begin() + line.size() - 1));
	else
		return m_cb->OnLine(line);
}

