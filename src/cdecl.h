#ifndef __CDECL_H__
#define __CDECL_H__

#include <stdio.h>

struct cdecl;
typedef struct cdecl cdecl;

struct cdecl *cdecl_new(int output_len);
int cdecl_input(struct cdecl *c, const char *input);
void cdecl_destroy(struct cdecl *c);

#endif /* __CDECL_H__ */
