#include <string.h>
#include "grammar.h"
#include "gstr.h"
#include "list.h"
#include "rxutil.h"
#include "choose.h"

extern int maxStackDepth;
extern int dynamic;

int stackDepth = 0;

GRAMMAR *grammar_binding(GRAMMAR *gram, char *label);

GRAMMAR *grammar_new() {
     GRAMMAR *ng = calloc(1,sizeof(GRAMMAR));
     ng->parent = NULL;
     ng->contents = dict_new();
     return ng;
}

/* destroy a grammar */
void grammar_free(GRAMMAR *g) {
     dict_freeValues(g->contents,(Destructor)grambit_free);
     free(g); 
}

static char *getLabel(char *packagedLabel);

void grammar_add(GRAMMAR *g, RULE *r) {
     char *label = rule_getLabel(r);
     /* if the scope is lexical, add to this grammar */
     if(r->scope == LEXICAL_SCOPE) {
	  dict_put(g->contents,label,r);
     } else {
	  /* otherwise walk up the grammar stack */
	  GRAMMAR *frame = grammar_binding(g, label);
	  if(!frame) { /* no binding. walk up to top frame */
	       for(frame = g; frame->parent; frame = frame->parent)
		    ;
	  }
	  /* now we're at the right frame. */
	  dict_put(frame->contents,label,r);
     }
}

static char *getLabel(char *packagedLabel) {
     int i;
     char *p;
     for(p = packagedLabel, i = 0; *p && *p != '.'; p++, i++)
	  ;
     if(*p) { /* found a '.' */
	  GSTR *g = gstr_new("");
	  gstr_append(g, packagedLabel + i + 1);
	  return gstr_detach(g);
     } else { /* didn't--this isn't a packaged label */
	  return NULL;
     }
}

/* walk the grammar stack to find the grammar with a binding for the given label */
GRAMMAR *grammar_binding(GRAMMAR *gram, char *label) {
     char *noPackage = NULL;
     RULE *r = (RULE *)dict_get(gram->contents,label);
     if(r) {
	  return gram;
     }
     /* not found. try it without the package (if any) */
     noPackage = getLabel(label);
     if(noPackage) {
	  r = (RULE *)dict_get(gram->contents, noPackage);
     }
     if(r) {
	  return gram;
     }
     /* not found. try the parent grammar */
     if(gram->parent) {
	  return grammar_binding(gram->parent, label);
     }
     /* OK, there really is no such binding. */
     return NULL;
}

RULE *grammar_lookUp(GRAMMAR *gram, char *label) {
     char *noPackage = NULL;
     RULE *r = NULL;
     if(!dynamic) {
	  gram = grammar_binding(gram, label);
     }
     if(gram) {
	  r = (RULE *)dict_get(gram->contents, label);
	  if(r) return r;
	  noPackage = getLabel(label); /* this is redundant but that's no biggie */
	  if(!noPackage) {
	       /* this is fatal, because it's inconsistent with the
		grammar_binding results in hand */
	       fprintf(stderr,"binding disappeared for %s\n",label);
	       exit(-1);
	  }
	  r = (RULE *)dict_get(gram->contents, noPackage);
	  if(r) return r;
     }
     return NULL;
}

LIST *expand_choice(GRAMMAR *gram, LIST *c) {
     int i;
     LIST *l = list_new();
     for(i = 0; i < list_length(c); i++) {
	  GRAMBIT *g = (GRAMBIT *)list_get(c,i);
	       list_appendAndFree(l,grammar_expand(gram,g));
     }
     return l;
}

LIST *grammar_expand(GRAMMAR *parentGram, GRAMBIT *g) {
     RULE *r;
     LIST *result;
     int x, j;
     GRAMMAR *gram;

     stackDepth++;

     if(maxStackDepth > 0 && stackDepth > maxStackDepth) {
	  fprintf(stderr,"warning: stack depth exceeded\n");
	  result = list_new();
	  return result;
     }

     if(dynamic) {
	  gram = parentGram;
     } else {
	  /* create a new grammar stack frame */
	  gram = grammar_new();
	  gram->parent = parentGram;
     }

     result = list_new();

     x = choose_next((g->max_x - g->min_x) + 1) + g->min_x;

     for(j = 0; j < x; j++) {
	  switch(g->type) {
	  case LITERAL_T:
	  case TRANS_T:
	       list_add(result,literal_new(grambit_toString(gram, g)));
	       break;
	  case LABEL_T:
#ifdef DEBUG
	       {
		    int i;
		    for(i = 0; i < stackDepth-1; i++)
			 fputc('>',stdout);
		    grambit_print(g,stdout);
		    printf("\n");
	       }
#endif
	       r = grammar_lookUp(gram,g->l);
	       if(r) {
		    LIST *cs;
		    LIST *c, *ex;
		    cs = (LIST *)rule_getChoices(r);
		    c = (LIST *)list_get(cs, choose_next(list_length(cs)));
		    ex = expand_choice(gram,c);
#ifdef DEBUG
		    {
			 int i;
			 for(i = 0; i < stackDepth-1; i++)
			      fputc('<',stdout);
			 for(i = 0; i < ex->length; i++) {
			      GRAMBIT *gb = (GRAMBIT *)list_get(ex,i);
			      grambit_print(gb,stdout);
			 }
			 printf("\n");
		    }

#endif		    
		    list_appendAndFree(result,ex);
	       } else {
		    /* rule not found; produce the rule name */
		    fprintf(stderr,"rmutt: warning: rule not found: %s\n",g->l);
		    list_add(result,literal_new(g->l));
	       }
	       break;
	  case RULE_T:
	       /* copy the rule to the grammar */
	       grammar_add(parentGram,grambit_copy(g));
	       break;
	  case ASSIGNMENT_T:
	       /* produce the rhs, and then create a new literal
		  rule with the result as its only choice, and
		  add it to the grammar */
	       if(1) {
		    RULE *r;
		    GRAMBIT *lit;
		    LIST *onlyChoice;
		    LIST *choices;
		    char *str;
		    /* first, produce the rhs */
		    str = grammar_produce(gram,choice_new(rule_getChoices(g)));
		    /* now create a single term and single choice */
		    onlyChoice = list_new();
		    lit = literal_new(str);
		    free(str);
		    list_add(onlyChoice,lit);
		    choices = list_new();
		    list_add(choices,onlyChoice);
		    /* make that the rhs of a new rule */
		    r = rule_new(rule_getLabel(g),choices,g->scope);
		    /* add the rule to the grammar */
		    grammar_add(parentGram,r);
	       }
	       break;
	  case CHOICE_T:
	       if(1) {
		    LIST *c;
		    LIST *cs;
		    cs = (LIST *)choice_getChoices(g);
		    c = (LIST *)list_get(cs, choose_next(list_length(cs)));
		    list_appendAndFree(result,expand_choice(gram,c));
	       }
	       break;
	  default:
	       list_add(result,g);
	       break;
	  }
     }

     stackDepth--;

     if(!dynamic) {
	  /* release the local frame */
	  grammar_free(gram);
     }

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
		    ns = transform(gram, str, (GRAMBIT *)list_get(tl,i));
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
char *transform(GRAMMAR *gram, char *str, GRAMBIT *trans) {
     switch(trans->type) {
     case RXSUB_T:
#ifdef DEBUG
	  printf(">/%s/%s/",trans->rx_rx,trans->rx_rep);
#endif
	  return regsub(str,trans->rx_rx,trans->rx_rep,REG_EXTENDED);
     case MAPPING_T:
	  if(!strcmp(str,trans->rx_rx))
	       return(grammar_produce(gram, trans->trans));
	  else
	       return(strdup(str));
     }
     fprintf(stderr,"error: illegal transformation\n");
     return NULL;
}

