#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef DEBUG_ENABLE
#define dprintf(...) printf(__VA_ARGS__)
#else
#define dprintf(...)
#endif

#endif//__DEBUG_H__
