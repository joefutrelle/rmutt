#include <stdio.h>
#include "dict.h"
#include "grambit.h"

typedef struct _grammar {
    struct _grammar *parent;
    DICT *contents;
} GRAMMAR;

/* rmutt grammar implementation */

extern GRAMMAR *grammar_new(); /* create a grammar */
extern void grammar_free(GRAMMAR *); /* free a grammar */

extern void grammar_add(GRAMMAR *, RULE *); /* add a rule to a grammar */
extern void grammar_addAll(GRAMMAR *, LIST *); /* add rules to a grammar */
extern RULE *grammar_lookUp(GRAMMAR *, char *); /* look up a named rule in the grammar */

/* expand the grambit of the grammar into a sequence of
 grambits, selected according to the rules of the grammar */
extern LIST *grammar_expand(GRAMMAR *, GRAMBIT *g);

/* do the same thing but actually produce a string */
extern char *grammar_produce(GRAMMAR *, GRAMBIT *g);

/* produce the string for just one grambit */
extern char *grambit_toString(GRAMMAR *, GRAMBIT *);

/* apply a transformation to a string in the context of a grammar */
extern char *transform(GRAMMAR *g, char *, GRAMBIT *);
