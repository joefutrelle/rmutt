#ifndef GRAMBIT_H_
#define GRAMBIT_H_

#include "list.h"
#include "stdio.h"

#define LABEL_T      0 /* a label i.e. "fish" in "dog: fish | nose;" */
#define LITERAL_T    1 /* a literal i.e. '"cow"' in 'shoe: "cow" | nose;" */
#define RULE_T       2 /* a rule i.e. "fish: brain | cow;" or "[dog:
                          brain | cow]" */
#define ASSIGNMENT_T 3 /* an assignment i.e. "[shoe = nose | dog]" */
#define RXSUB_T      4 /* a def of a regex sub, i.e. "/(.)\1/dog/" */
#define MAPPING_T    5 /* a simple string->string mapping */
#define CHOICE_T     6 /* an anonymous choice i.e. "(foo | bar)" */
#define USE_T        7 /* a namespace directive i.e. 'use "blah"' */
#define TRANS_T      8 /* transformation i.e. "foo > /fish/duck/" */

typedef struct _grambit {
     int type;      /* one of the type constants above */
     char *l;       /* label or literal */
     LIST *choices; /* choices (for choice, rule or assignment) */
     int min_x; /* minimum number of times (for label, literal, or choice) */
     int max_x; /* max      "     "   " ... */
     char *rx_rx; /* regex to replace */
     char *rx_rep; /* string to replace it with */
     /* rx_rx and rx_rep are also used for simple mappings */
     struct _grambit *source; /* source grambit for transformation */
     struct _grambit *trans; /* trans grambit for transformation */
} GRAMBIT;

#define RULE GRAMBIT
#define ASSIGNMENT GRAMBIT

extern void grambit_free(GRAMBIT *);
extern GRAMBIT *literal_new(char *);
extern GRAMBIT *label_new(char *);
extern GRAMBIT *rule_new(char *, LIST *);
extern GRAMBIT *assignment_new(char *, LIST *);
extern GRAMBIT *rxsub_new(char *, char *);
extern GRAMBIT *mapping_new(char *, char *);
extern GRAMBIT *choice_new(LIST *);
extern GRAMBIT *trans_new(GRAMBIT *, GRAMBIT *);

extern void grambit_addChoice(GRAMBIT *, LIST *);
extern char *rule_getLabel(RULE *);
extern LIST *rule_getChoices(RULE *);

extern void literal_append(GRAMBIT *, char *);

#define rule_addChoice grambit_addChoice
#define assignment_addChoice grambit_addChoice
#define choice_addChoice grambit_addChoice
#define choice_getChoices rule_getChoices

extern void grambit_print(GRAMBIT *, FILE *);

#endif
