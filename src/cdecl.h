#ifndef __CDECL_H__
#define __CDECL_H__

#include <stdio.h>

#ifndef min
# define min(a, b) ((a)<=(b) ? (a) : (b))
#endif
#ifndef max
# define max(a, b) ((a)>=(b) ? (a) : (b))
#endif

struct cdecl;
typedef struct cdecl cdecl;

struct cdecl *cdecl_new();
void cdecl_destroy(struct cdecl *c);

int cdecl_input(struct cdecl *c, const char *input);

int cdecl_analyze(struct cdecl *c, char *output, int output_len);
char *cdecl_identifier(struct cdecl *c);

int cdecl_error(cdecl *c);

#endif /* __CDECL_H__ */
