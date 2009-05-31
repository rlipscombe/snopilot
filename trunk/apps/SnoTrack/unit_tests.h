#pragma once

#if defined(_DEBUG)
int runUnitTests();
#else
inline int runUnitTests() { return 0; /* nothing */ }
#endif
