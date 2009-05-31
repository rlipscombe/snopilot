/* tokenise_string.h
 *
 * Copyright (C) 2003 Roger Lipscombe, http://www.differentpla.net/~roger/
 * Copyright (c) 2003 Snopilot, http://www.snopilot.com/
 */

#ifndef TOKENISE_STRING_H
#define TOKENISE_STRING_H 1

#include <vector>
#include <string>

typedef std::vector<std::string> tokens_t;
tokens_t TokeniseString(const std::string &s, char sep);

#endif /* TOKENISE_STRING_H */
