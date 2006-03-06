#ifndef GSTR_H_
#define GSTR_H_

#include <stdlib.h>

typedef struct _gstr {
     size_t size;
     size_t len;
     char *buf;
} GSTR;

/* growable string implementation */

extern GSTR *gstr_new(char *); /* create a new growable string */
extern void gstr_appendn(GSTR *,char *, size_t); /* append n characters to the end of a growable string */
extern void gstr_append(GSTR *,char *); /* append a null-terminated string to the end of a growable string */
extern void gstr_appendc(GSTR *,char); /* append a single character to the end of a growable string */
extern char *gstr_detach(GSTR *); /* return a copy of a growable string and free the growable string */
extern char *gstr_cat(char *, char *); /* concatenate two strings and return that as a growable string */
extern char *gstr_catf(char *, char *); /* same as gstr_cat but frees the strings */

#endif
