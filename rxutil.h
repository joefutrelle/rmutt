#ifndef REGUTIL_H
#define REGUTIL_H

#include <sys/types.h>
#include <regex.h>

#include "list.h"

char *regsub (char *str, char *rx, char *replace, int flags);
char *regsub_c (char *str, regex_t *rx, char *replace);
char *regsubs (char *str, char **rxs, int flags);
char **regmatch (char *str, char *rx, size_t nmatch, int cflags, int eflags);
char **regmatch_c (char *str, regex_t *rx, size_t nmatch, int eflags);
LIST *split (char *str, char *rx, int flags);

#endif	/* REGUTIL_H */
