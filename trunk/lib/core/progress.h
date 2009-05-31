/* progress.h
 */

#ifndef PROGRESS_H
#define PROGRESS_H 1

#include "status.h"

class Progress
{
public:
	virtual STATUS OnProgress(int num, int denom) = 0;
	virtual STATUS OnComplete() = 0;
};

class TextProgress : public Progress
{
public:
	virtual STATUS OnProgress(int num, int denom);
	virtual STATUS OnComplete();
};

#endif /* PROGRESS_H */
