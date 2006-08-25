#include <stdlib.h>
#include "grambit.h"
#include "list.h"
#include "string.h"
#include "gstr.h"

static GRAMBIT *grambit_new(int type);

/* create a new grammatical bit */
GRAMBIT *grambit_new(int t) {
  GRAMBIT *g = (GRAMBIT *)calloc(1, sizeof(GRAMBIT));
  g->type = t;
  g->l = NULL;
  g->choices = list_new();
  g->min_x = 1;
  g->max_x = 1;
  g->source = NULL;
  g->trans = NULL;
  return g;
}

void choices_free(LIST *choices) {
     /* each choice is a list of terms, i.e. grambits */
     list_freeData(choices, (Destructor)grambit_free);
}

/* destroy a grammatical bit */
void grambit_free(GRAMBIT *g) {
     free(g->l);
     if(g->choices) {
	  list_freeData(g->choices, (Destructor)choices_free);
     }
     if(g->rx_rx) free(g->rx_rx);
     if(g->rx_rep) free(g->rx_rep);
     if(g->source) grambit_free(g->source);
     if(g->trans) grambit_free(g->trans);
     free(g);
}

/* create a new literal */
GRAMBIT *literal_new(char *text) {
  GRAMBIT *ng = grambit_new(LITERAL_T);
  ng->l = strdup(text);
  return ng;
}

/* create a new label */
GRAMBIT *label_new(char *text) {
  GRAMBIT *ng = literal_new(text);
  ng->type = LABEL_T;
  return ng;
}

/* create a new rule with the given label and choices.
   choices is a list of lists because each choice is a sequence.
   if choices are NULL, create a new empty list so that you
   can add choices with rule_addChoice */
GRAMBIT *rule_new(char *label, LIST *choices, int scope) {
  GRAMBIT *ng = grambit_new(RULE_T);
  if(label) {
       ng->l = strdup(label);
  }
  if(choices) {
    list_free(ng->choices);
    ng->choices = choices;
  }
  ng->scope = scope;
  return ng;
}

/* copy the given grambit */
GRAMBIT *grambit_copy(GRAMBIT *g) {
     int i,j;
     GRAMBIT *ng = grambit_new(g->type);
     ng->scope = g->scope;
     if(g->l) {
	  ng->l = strdup(g->l);
     }
     if(g->choices) {
	  for(i = 0; i < list_length(g->choices); i++) {
	       LIST *cs = (LIST *)list_get(g->choices,i);
	       LIST *nc = list_new();
	       for(j = 0; j < list_length(cs); j++) {
		    list_add(nc, grambit_copy((GRAMBIT *)list_get(cs, j)));
	       }
	       list_add(ng->choices, nc);
	  }
     }
     ng->min_x = g->min_x;
     ng->max_x = g->max_x;
     if(g->rx_rx) { ng->rx_rx = strdup(g->rx_rx); }
     if(g->rx_rep) { ng->rx_rep = strdup(g->rx_rep); }
     if(g->source) { ng->source = grambit_copy(g->source); }
     if(g->trans) { ng->trans = grambit_copy(g->trans); }
     return ng;
}

/* create a new assignment with the given label and choices.
   choices is a list of lists because each choice is a sequence.
   if choices are NULL, create a new empty list so that you
   can add choices with assignment_addChoice */
GRAMBIT *assignment_new(char *label, LIST *choices, int scope) {
  GRAMBIT *ng = rule_new(label, choices, scope);
  ng->type = ASSIGNMENT_T;
  return ng;
}

/* create a new assignment of a label to a single choice. */
GRAMBIT *binding_new(char *label, GRAMBIT *choice, int scope) {
     return assignment_new(label, list_addToNew(list_addToNew(choice)), scope);
}

GRAMBIT *rxsub_new(char *rx, char *rep) {
     GRAMBIT *ng = grambit_new(RXSUB_T);
     ng->rx_rx = strdup(rx);
     ng->rx_rep = strdup(rep);
     return ng;
}

GRAMBIT *mapping_new(char *rx, GRAMBIT *g) {
     GRAMBIT *ng = grambit_new(MAPPING_T);
     ng->rx_rx = strdup(rx);
     ng->trans = g;
     return ng;
}

GRAMBIT *trans_new(GRAMBIT *s, GRAMBIT *t) {
     GRAMBIT *ng = grambit_new(TRANS_T);
     ng->source = s;
     ng->trans = t;
     return ng;
}

GRAMBIT *choice_new(LIST *choices) {
     GRAMBIT *ng = rule_new(NULL, choices, 0);
     ng->type = CHOICE_T;
     return ng;
}

void grambit_addChoice(GRAMBIT *g, LIST *choice) {
  list_add(g->choices, choice);
}

/* return a rule's label */
char *rule_getLabel(RULE *g) {
  return g->l;
}

/* return a rule's choices (a list of lists) */
LIST *rule_getChoices(RULE *g) {
  return g->choices;
}

/* append a string to the end of a literal */
void literal_append(GRAMBIT *g, char *suffix) {
     char *ns = gstr_cat(g->l,suffix);
     free(g->l);
     g->l = ns;
}

/* for debugging */
void grambit_print(GRAMBIT *g, FILE *fp) {
  long i, j;
  char *l = g->l ? g->l : "(null)";
  fprintf(fp, "[");
  switch(g->type) {
  case LABEL_T:
    fprintf(fp, "label: %s]", l);
    return;
  case LITERAL_T:
    fprintf(fp, "literal: \"%s\"]", l);
    return;
  case RULE_T:
    fprintf(fp, "rule: %s -> ", l);
    break;
  case ASSIGNMENT_T:
    fprintf(fp, "assignment: %s = ", l);
    break;
  case CHOICE_T:
    fprintf(fp, "choice:");
    break;
  case RXSUB_T:
       fprintf(fp, "rxsub:");
       break;
  case TRANS_T:
       fprintf(fp, "trans:");
       break;
  default:
    fprintf(fp, "unknown");
    return;
  }
  fflush(fp);
  for(i = 0; i < list_length(g->choices); i++) {
    LIST *cs = (LIST *)list_get(g->choices,i);
    for(j = 0; j < list_length(cs); j++) {
	 grambit_print((GRAMBIT *)list_get(cs, j), fp);
	 fprintf(fp, ", ");
    }
    fprintf(fp, " | ");
  }
  fprintf(fp, "]");
  fflush(fp);
}
