/* nmea_sentence_collector.h
 *
 * Copyright (C) 2003 Snopilot, http://www.snopilot.com/
 */

#ifndef NMEA_SENTENCE_COLLECTOR_H
#define NMEA_SENTENCE_COLLECTOR_H 1

#include "buffer_collector.h"
#include <string>

class NmeaSentenceCollector : public BufferCollectorCallback
{
// BufferCollectorCallback
public:
	virtual bool ProcessBuffer(const char *buffer, int buffer_size, int *bytes_eaten);

private:
	bool FireLine(const std::string &line);
	bool OnLine(const std::string &line);

private:
	std::string GetSentenceData(const std::string &sentence) const;

	int CalculateSentenceChecksum(const std::string &sentence) const;
	bool VerifySentenceChecksum(const std::string &sentence) const;
};

#endif /* NMEA_SENTENCE_COLLECTOR_H */
