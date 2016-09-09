#ifndef __BERROR_H__

#define eprintf(...) fprintf(stderr,__VA_ARGS__)
#define eline(line) eprintf("    at line %d\n", line);

#endif
