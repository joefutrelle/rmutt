#include <stdio.h>
#include "dict.h"
#include "grambit.h"

#define GRAMMAR DICT

extern void grammar_add(GRAMMAR *, RULE *);
extern RULE *grammar_lookUp(GRAMMAR *, char *);

/* expand the grambit of the grammar into a sequence of
   grambits, selected according to the rules of the grammar */
extern LIST *grammar_expand(GRAMMAR *, GRAMBIT *g);

/* do the same thing but actually produce a string */
extern char *grammar_produce(GRAMMAR *, GRAMBIT *g);

/* produce the string for just one grambit */
extern char *grambit_toString(GRAMMAR *, GRAMBIT *);

extern char *transform(GRAMMAR *g, char *,GRAMBIT *);

