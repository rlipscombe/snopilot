/* tokenise_string.cpp
 *
 * Copyright (C) 2002-2003 Roger Lipscombe, http://www.differentpla.net/~roger/
 * Copyright (c) 2003 Snopilot, http://www.snopilot.com/
 */

#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include "tokenise_string.h"

tokens_t TokeniseString(const std::string &s, char sep)
{
	tokens_t result;

	typedef std::string::size_type size_type;
	const size_type npos = std::string::npos;

	size_type pos0 = 0;
	for (;;)
	{
		size_type pos = s.find(sep, pos0);
		std::string arg;
		if (pos != npos)
			arg = std::string(s.begin() + pos0, s.begin() + pos);
		else
			arg = std::string(s.begin() + pos0, s.end());

		result.push_back(arg);

		if (pos == npos)
			break;

		pos0 = pos + 1;
	}

	return result;
}

