#ifndef REGUTIL_H
#define REGUTIL_H

#include <sys/types.h>
#include <regex.h>

#include "list.h"

/* perform a regex sub. str=source string, rx=regex, replace=replacement string, flags=flags */
char *regsub(char *str, char *rx, char *replace, int flags);
/* same as regsub, but compiles the regex first */
char *regsub_c(char *str, regex_t *rx, char *replace);
/* multiple cascading regsubs */
char *regsubs(char *str, char **rxs, int flags);
/* match. str=source string, rx=regex, nmatch=matches expected, ... */
char **regmatch(char *str, char *rx, size_t nmatch, int cflags, int eflags);
/* same as regmatch but compiles the regex first */
char **regmatch_c(char *str, regex_t *rx, size_t nmatch, int eflags);
/* split a string by a regex and return a list */
LIST *split(char *str, char *rx, int flags);

#endif	/* REGUTIL_H */
