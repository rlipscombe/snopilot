/* nmea_sentence_collector.h
 *
 * Copyright (C) 2003 Snopilot, http://www.snopilot.com/
 */

#ifndef NMEA_SENTENCE_COLLECTOR_H
#define NMEA_SENTENCE_COLLECTOR_H 1

#include "line_collector.h"
#include <string>
#include "tokenise_string.h"

#include "PositionFix.h"
class Projection;

class NmeaSentenceCollector : public LineCollectorCallback
{
	PositionFixCollection *m_positions;
	Projection *m_proj;

public:
	NmeaSentenceCollector(PositionFixCollection *positions, Projection *proj)
		: m_positions(positions), m_proj(proj)
	{
	}

// LineCollectorCallback
public:
	virtual bool OnLine(const std::string &line);

private:
	std::string GetSentenceData(const std::string &sentence) const;

	int CalculateSentenceChecksum(const std::string &sentence) const;
	bool VerifySentenceChecksum(const std::string &sentence) const;

private:
	void OnGPBOD(const tokens_t &tokens);
	void OnGPGGA(const tokens_t &tokens);
	void OnGPGLL(const tokens_t &tokens);
	void OnGPGSA(const tokens_t &tokens);
	void OnGPGSV(const tokens_t &tokens);
	void OnGPRMB(const tokens_t &tokens);
	void OnGPRMC(const tokens_t &tokens);
	void OnPGRME(const tokens_t &tokens);
	void OnPGRMZ(const tokens_t &tokens);
	void OnGPRTE(const tokens_t &tokens);
};

#endif /* NMEA_SENTENCE_COLLECTOR_H */
