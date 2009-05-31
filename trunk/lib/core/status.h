/* status.h
 *
 * STATUS is similar to Win32's HRESULT.  So similar, in fact, that they're compatible.
 * The main distinction is that, because Microsoft have reserved every facility except FACILITY_ITF,
 * we have to wedge our custom errors into the whole of the low 16 bits.
 * In order to make this easier, we define a "component", to take up 8 bits (256 components available),
 * and a "code", to take up 8 bits (256 codes available).
 *
 * This is similar to the scheme used by empeg's STATUS type, except that the partition is in a
 * different place.  empeg's type uses a 4/12 split.
 *
 * I did consider using a 6/10 split, but the hex digits are more readable if we use 4-bit partitions.
 *
 * Copyright (C) 2003 Roger Lipscombe, http://www.differentpla.net/~roger/
 */

#ifndef STATUS_H
#define STATUS_H 1

#include "opaque_handle.h"

#if !defined(FACILITY_ITF)
#define FACILITY_ITF 4
#endif

#define MAKE_STATUS_(sev, comp, code) \
	((((sev) & 0x03) << 30) | ((FACILITY_ITF & 0xfff) << 16) | (((comp) & 0xff) << 8) | ((code) & 0xff))

#if defined(WIN32)
/* Can't use opaque STATUS type on Win32, since we want to use it interchangably with HRESULT.
 */
typedef HRESULT STATUS;
 #define MAKE_STATUS MAKE_STATUS_
#else
/* On other platforms, we can use the opaque type.  In fact, we'd prefer to -- it makes for better type-safety.
 */
 #ifdef _DEBUG
 DECLARE_OPAQUE_HANDLE(STATUS);
 #define MAKE_STATUS(sev, comp, code) \
	MAKE_OPAQUE_HANDLE(STATUS,  MAKE_STATUS_((sev), (comp), (code)))
 #else
 /* ...but not in Release builds -- this is probably more efficient. */
 typedef unsigned long STATUS;
 #define MAKE_STATUS MAKE_STATUS_
 #endif
#endif

#define StatusFromErrno(e) \
	MAKE_STATUS(3, COMPONENT_ERRNO, e)

#define COMPONENT_ERRNO 1
#define COMPONENT_GENERAL 2

#define E_USER_CANCELLED MAKE_STATUS(2, COMPONENT_GENERAL, 1)

#endif /* ROGER_STATUS_H */
