#ifndef GRAMBIT_H_
#define GRAMBIT_H_

#include "list.h"
#include "stdio.h"

/* grammatical unit ("grammatical bit" or "grambit") implementation */

/* types of grambits */
#define LABEL_T      0 /* a label i.e. "fish" in "dog: fish, nose;" */
#define LITERAL_T    1 /* a literal i.e. '"cow"' in 'shoe: "cow", nose;" */
#define RULE_T       2 /* a rule i.e. "fish: brain, cow;" or "(dog:
                          brain, cow)" */
#define ASSIGNMENT_T 3 /* an assignment i.e. "(shoe = nose, dog)" */
#define RXSUB_T      4 /* a def of a regex sub, i.e. "/(.)\1/dog/" */
#define MAPPING_T    5 /* a simple string->string mapping */
#define CHOICE_T     6 /* an anonymous choice i.e. "(foo, bar)" */
#define TRANS_T      7 /* transformation i.e. "foo > /fish/duck/" */

/* scoping parameters controlling what grammar a rule or assignment is added to */
#define LOCAL_SCOPE  0     /* local grammar */
#define NON_LOCAL_SCOPE 1  /* first ancestor grammar in which the label is bound */
#define GLOBAL_SCOPE 2     /* first ancestor grammar without a parent */

typedef struct _grambit {
     int type;      /* what type of grambit */
     char *l;       /* label or literal */
     int scope;     /* scope of rule/assignment */
     LIST *choices; /* choices (for choice, rule or assignment) */
     int min_x; /* minimum number of times (for label, literal, or choice) */
     int max_x; /* max      "     "   " ... */
     char *rx_rx; /* regex to replace */
     char *rx_rep; /* string to replace it with */
     /* rx_rx is also used for simple mappings */
     struct _grambit *source; /* source grambit for transformation */
     struct _grambit *trans; /* trans grambit for transformation or mapping */
} GRAMBIT;

/* synonyms for grambit for readability of code */
#define RULE GRAMBIT
#define ASSIGNMENT GRAMBIT

extern GRAMBIT *grambit_copy(GRAMBIT *); /* copy a grambit */
extern void grambit_free(GRAMBIT *); /* free a grambit */

extern GRAMBIT *literal_new(char *); /* create a new literal with the given string */
extern GRAMBIT *label_new(char *); /* create a new label with the given name */
extern GRAMBIT *rule_new(char *, LIST *, int); /* create a rule with the given alternatives and scope */
extern GRAMBIT *rule_newWithArguments(char *, LIST *, LIST *, int); /* create a rule with the given arguments, alternatives and scope */
extern GRAMBIT *assignment_new(char *, LIST *, int); /* same, but assignment */
extern GRAMBIT *binding_new(char *, GRAMBIT *, int); /* simple assignment with only one choice */
extern GRAMBIT *rxsub_new(char *, char *); /* create a regex transformation given regex and replacement */
extern GRAMBIT *mapping_new(char *, GRAMBIT *); /* create a new mapping between a string and a grambit */
extern GRAMBIT *choice_new(LIST *); /* create a new choice with the given alternatives */
extern GRAMBIT *trans_new(GRAMBIT *, GRAMBIT *); /* create a new transformation given source, result */
extern GRAMBIT *trans_new(GRAMBIT *, GRAMBIT *); /* create a new transformation given source, result */
extern GRAMBIT *call_new(char *, LIST *); /* create a new rule w/arg call given label, params */
extern void grambit_addChoice(GRAMBIT *, LIST *); /* add a choice to a rule or assn given alternatives */
extern char *rule_getLabel(RULE *); /* get the label of a rule */
extern LIST *rule_getChoices(RULE *); /* get the choices of a rule */

extern void literal_append(GRAMBIT *, char *); /* append a string to the end of a literal */

/* convenient synonyms for readability */
#define rule_addChoice grambit_addChoice
#define assignment_addChoice grambit_addChoice
#define choice_addChoice grambit_addChoice
#define choice_getChoices rule_getChoices

extern void grambit_print(GRAMBIT *, FILE *); /* for debugging */

/* maximum number of digits in number of positional arguments.
   8 is ridiculously high allowing for 99999999 positional arguments. */
#define MAX_VAR_LENGTH 8

#endif
