#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "rxutil.h"
#include "list.h"
#include "gstr.h"

#define MARKER "FOOBAR"

char *rx_interpolate(char *orig, char *replace_o, regmatch_t *match) {
     char key[5];
     int i;
     char *replace = strdup(replace_o);
     i = 1;
     while(match[i].rm_so != -1) {
	  char *ns;
	  char *rep;
	  GSTR *rep_gs;
	  rep_gs = gstr_new("");
	  gstr_appendn(rep_gs,orig + match[i].rm_so,match[i].rm_eo - match[i].rm_so);
	  gstr_append(rep_gs,"");
	  rep = gstr_detach(rep_gs);
	  sprintf(key,"\\\\%d",i);
	  /* now replace \(digit) with whatever in "replace" */
	  ns = regsub(replace,key,rep,0);
	  free(replace);
	  replace = ns;
	  i++;
     }
     return replace;
}

/*
 * globally substitute replace for regex in str
 */
char *regsub_c(char *str, regex_t *rx, char *replace)
{
	regmatch_t match[10];
	size_t str_offset = 0, rep_len, tail_len;
	GSTR *res;
	int initial = 0;

	res = gstr_new("");

	while ((regexec(rx, str + str_offset, 10, match,
			!initial++ ? 0 : REG_NOTBOL) == 0)) {
	     char *rep = rx_interpolate(str+str_offset,replace,match);
	     rep_len = strlen(rep);
		gstr_appendn(res, str + str_offset, match[0].rm_so);
		gstr_appendn(res, rep, rep_len);
		str_offset += match[0].rm_eo;
	}

	tail_len = strlen(str + str_offset);
	gstr_appendn(res, str + str_offset, tail_len);
	gstr_append(res,"");
	return gstr_detach (res);
}

/*
 * same as regsub, except it compiles the regex for you
 */
char *regsub(char *str, char *rx, char *replace, int flags)
{
	regex_t comp_rx;
	char *result;

	regcomp(&comp_rx, rx, flags);
	result = regsub_c(str, &comp_rx, replace);
	regfree(&comp_rx);
	return result;
}

/* multiple cascading regsubs. rxs should be in the form
 * { rx0, rep0, rx1, rep1 ... rxn, repn, NULL }
 * where each rx is a regex, and the immediately-following rep is
 * the replacement string.
 * returns the final string after all substitutions have been
 * done.
 */
char *regsubs(char *str, char **rxs, int flags)
{
	char *buf[2];
	int i, alt;

	buf[0] = str;

	for (i = 0, alt = 0; rxs[i] != NULL; i += 2, alt++) {
		register char *old = buf[alt % 2];

		buf[(alt + 1) % 2] = regsub(old, rxs[i], rxs[i + 1], flags);

		if (old != str)
			/* don't free the string originally passed in! */
			free(old);
	}

	return buf[alt % 2];
}

/*
 * return copies of all the matched subexps. NULL if failed
 */
char **regmatch_c(char *str, regex_t * rx, size_t nmatch, int eflags)
{
	char **result = NULL;
	int i;
	regmatch_t *matches;

	matches = (regmatch_t *) calloc(nmatch + 1, sizeof(regmatch_t));

	i = regexec(rx, str, nmatch + 1, matches, eflags);

	if (i)
		goto finish;

	result = (char **) calloc(nmatch, sizeof(char *));
	for (i = 1; i < nmatch + 1; i++) {
		int len = matches[i].rm_eo - matches[i].rm_so;

		result[i - 1] = (char *)malloc(len + 1);
		strncpy(result[i - 1], str + matches[i].rm_so, len);
		result[i - 1][len] = '\0';
	}

finish:
	free(matches);
	return result;
}

/*
 * same as regmatch_c, except it compiles the regex for you
 */
char **regmatch(char *str, char *rx, size_t nmatch, int cflags, int eflags)
{
	regex_t rx_comp;
	char **result;

	regcomp(&rx_comp, rx, cflags);

	result = regmatch_c(str, &rx_comp, nmatch, eflags);

	regfree(&rx_comp);

	return result;
}

/*
 * split str into substrings separated by rx
 */
LIST *split_c(char *str, regex_t * rx)
{
	regmatch_t match[1];
	size_t str_offset = 0;
	LIST *result = list_new();

	while (regexec(rx, str + str_offset, 1, match, REG_NOTBOL) == 0) {
		int len = match[0].rm_so;
		char *frag = (char *) malloc(len + 1);

		strncpy(frag, str + str_offset, len);
		*(frag + len) = '\0';
		list_add(result, frag);
		str_offset += match[0].rm_eo;
	}

	list_add(result, strdup(str + str_offset));
	return result;
}

/*
 * same as regsub, except it compiles the regex for you
 */
LIST *split(char *str, char *rx, int flags)
{
	regex_t comp_rx;
	LIST *result;

	regcomp(&comp_rx, rx, flags);

	result = split_c(str, &comp_rx);

	regfree(&comp_rx);
	return result;
}
