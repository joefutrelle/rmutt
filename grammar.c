#include <string.h>
#include "grammar.h"
#include "gstr.h"
#include "list.h"
#include "rxutil.h"

extern int maxStackDepth;

int stackDepth = 0;

/*
static char *getPackage(char *packagedLabel);
*/
static char *getLabel(char *packagedLabel);

void grammar_add(GRAMMAR *g, RULE *r) {
#ifdef DEBUG
     printf("adding rule: %s\n",rule_getLabel(r));
#endif
     dict_put(g,rule_getLabel(r),r);
}

/*
static char *getPackage(char *packagedLabel) {
     int i;
     char *p;
     for(p = packagedLabel, i = 0; *p && *p != '.'; p++, i++)
	  ;
     if(*p) {
	  GSTR *g = gstr_new("");
	  gstr_appendn(g, packagedLabel, i);
	  gstr_appendc(g,'\0');
	  return gstr_detach(g);
     } else {
	  return gstr_detach(gstr_new(""));
     }
}
*/

static char *getLabel(char *packagedLabel) {
     int i;
     char *p;
     for(p = packagedLabel, i = 0; *p && *p != '.'; p++, i++)
	  ;
     if(*p) {
	  GSTR *g = gstr_new("");
	  gstr_append(g, packagedLabel + i + 1);
	  return gstr_detach(g);
     } else {
	  /* this should never happen */
	  fprintf(stderr, "malformed label: %s\n",packagedLabel);
	  exit(-1);
     }
}

RULE *grammar_lookUp(GRAMMAR *gram, char *label) {
     RULE *r = (RULE *)dict_get(gram,label);
     if(r) return r;
     /* not found. try it without the package (if any) */
     r = (RULE *)dict_get(gram,getLabel(label));
     return r;
}

LIST *expand_choice(GRAMMAR *gram, LIST *c) {
     int i;
     LIST *l = list_new();
     for(i = 0; i < list_length(c); i++) {
	  int j;
	  int x;
	  GRAMBIT *g = (GRAMBIT *)list_get(c,i);
	  x = (rand() % ((g->max_x - g->min_x) + 1)) + g->min_x;
	  for(j = 0; j < x; j++)
	       list_appendAndFree(l,grammar_expand(gram,g));
     }
     return l;
}

LIST *grammar_expand(GRAMMAR *gram, GRAMBIT *g) {
     RULE *r;
     LIST *result;

     stackDepth++;
     if(maxStackDepth > 0 && stackDepth > maxStackDepth) {
	  fprintf(stderr,"warning: stack depth exceeded\n");
	  result = list_new();
	  return result;
     }

     result = list_new();
     switch(g->type) {
     case LITERAL_T:
	  list_add(result,g);
	  break;
     case LABEL_T:
#ifdef DEBUG
     {
	  int i;
	  for(i = 0; i < stackDepth-1; i++)
	       fputc('|',stderr);
	  fprintf(stderr,"+-%s\n",g->l);
     }
#endif
	  r = grammar_lookUp(gram,g->l);
	  if(r) {
	       LIST *cs;
	       LIST *c;
	       cs = (LIST *)rule_getChoices(r);
	       c = (LIST *)list_getRand(cs);
	       list_appendAndFree(result,expand_choice(gram,c));
	  } else {
	       fprintf(stderr,"error: rule %s not found\n",g->l);
	  }
	  break;
     case RULE_T:
	  /* add the rule to the grammar */
	  grammar_add(gram,g);
	  break;
     case ASSIGNMENT_T:
	  /* produce the rhs, and then create a new literal
	     rule with the result as its only choice, and
	     add it to the grammar */
	  if(1) {
	       RULE *r;
	       LIST *onlyChoice;
	       char *str;
	       str = grammar_produce(gram,choice_new(rule_getChoices(g)));
	       /* FIXME: leaks the new choice */
	       r = rule_new(rule_getLabel(g),NULL);
	       onlyChoice = list_new();
	       list_add(onlyChoice,literal_new(str));
	       free(str);
	       rule_addChoice(r,onlyChoice);
	       grammar_add(gram,r);
	  }
	  break;
     case CHOICE_T:
	  if(1) {
	       LIST *c;
	       c = (LIST *)list_getRand(choice_getChoices(g));
	       list_appendAndFree(result,expand_choice(gram,c));
	  }
	  break;
     default:
	  list_add(result,g);
	  break;
     }

     stackDepth--;

     return result;
}

char *grammar_produce(GRAMMAR *gram, GRAMBIT *g) {
     LIST *result;
     int i;
     GSTR *str;

     result = grammar_expand(gram,g);
     str = gstr_new("");
     for(i = 0; i < result->length; i++) {
	  char *s;
	  GRAMBIT *g = (GRAMBIT *)list_get(result,i);
	  s = grambit_toString(gram, g);
#ifdef DEBUG
	  printf("produced %s\n",s);
#endif
	  gstr_append(str,s);
	  free(s);
     }
     list_free(result);
     return gstr_detach(str);
}

/**
 * a big function, this transforms a grambit into a string,
 * whether it be a literal, an operation of some sort such as
 * a transformation or call to an external script, or what have you
 */
char *grambit_toString(GRAMMAR *gram, GRAMBIT *g) {
     switch(g->type) {
     case LITERAL_T:
	  return strdup(g->l);
     case TRANS_T:
	  if(1){
	       char *str;
	       LIST *tl;
	       int i;
	       /* first, expand the source grambit into a string */
	       str = grammar_produce(gram, g->source);
#ifdef DEBUG
	       printf("transforming \"%s\"",str);
#endif
	       /* now expand the (list of) transformation(s) */
	       tl = grammar_expand(gram, g->trans);
	       /* now apply them in series */
	       for(i = 0; i < tl->length; i++) {
		    char *ns;
		    ns = transform(str, (GRAMBIT *)list_get(tl,i));
#ifdef DEBUG
		    printf("%s",ns);
#endif
		    free(str);
		    str = ns;
	       }
#ifdef DEBUG
	       printf("\n");
#endif
	       return str;
	  }
     }
     return NULL;
}

/* apply a transformation to a string */
char *transform(char *str, GRAMBIT *trans) {
     switch(trans->type) {
     case RXSUB_T:
#ifdef DEBUG
	  printf(">/%s/%s/",trans->rx_rx,trans->rx_rep);
#endif
	  return regsub(str,trans->rx_rx,trans->rx_rep,REG_EXTENDED);
     case MAPPING_T:
	  if(!strcmp(str,trans->rx_rx))
	       return(strdup(trans->rx_rep));
	  else
	       return(strdup(str));
     }
     fprintf(stderr,"error: illegal transformation\n");
     return NULL;
}
