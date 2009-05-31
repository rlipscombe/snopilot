/* opaque_handle.h
 *
 * Copyright (C) 2003 Roger Lipscombe, http://www.differentpla.net/~roger/
 */

#ifndef OPAQUE_HANDLE_H
#define OPAQUE_HANDLE_H 1

#define DECLARE_OPAQUE_HANDLE(x) \
	struct opaque__##x { int unused; }; \
	typedef struct opaque__##x * x

#define MAKE_OPAQUE_HANDLE(x, n) \
	(reinterpret_cast<opaque__##x *>(n))

#endif /* OPAQUE_HANDLE_H */
