#ifndef GSTR_H_
#define GSTR_H_

#include <stdlib.h>

typedef struct _gstr {
     size_t size;
     size_t len;
     char *buf;
} GSTR;

extern GSTR *gstr_new(char *);
extern void gstr_appendn(GSTR *,char *, size_t);
extern void gstr_append(GSTR *,char *);
extern void gstr_appendc(GSTR *,char);
extern char *gstr_detach(GSTR *);
extern char *gstr_cat(char *, char *);
extern char *gstr_catf(char *, char *); /* concat and free */

#endif
