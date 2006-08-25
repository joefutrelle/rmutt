%{
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "global.h"
#include "list.h"
#include "dict.h"
#include "grambit.h"
#include "grammar.h"
#include "gstr.h"

extern GRAMMAR *grammar;
extern char *topRule;

#define CURRENT_PACKAGE g_package[includeStackPtr]

#define MAX_VAR_LENGTH 8

%}

%union {
  struct _grambit *grambit;
  struct _dict_node *dict;
  char *str;
  struct _list *list;
  int integer;
  struct {
    char *rx;
    char *rep;
  } rx;
}

%token PACKAGE
%token USE
%token <str> LABEL
%token <str> LITERAL
%token <integer> INTEGER
%token <rx> RXSUB
%type <str> Label
%type <list> Labels
%type <str> PackagedLabel
%type <str> Namespace
%type <list> Choice
%type <list> Choices
%type <list> Terms
%type <grambit> Term
%type <grambit> QualifiedTerm
%type <grambit> Rule
%type <grambit> Statement
%type <dict> Grammar

%start Top

%pure-parser

%expect 5

%%

Top:
  Grammar {
    grammar = (GRAMMAR *) $1;
    CURRENT_PACKAGE = NULL;
  };

Grammar: /* a Grammar is a DICT of Rules, keyed by label */
  Statement {
     GRAMMAR *gram = grammar_new();
     if($1) {
	  grammar_add(gram,$1);
     }
     $$=(void *)gram;
  }
  | Grammar Statement {
    if($2) {
	 grammar_add((GRAMMAR *)$1,$2);
    }
    $$=$1;
  }
  ;

Statement: /* each Statement either adds a Rule or changes the package */
  Rule ';' {
       $$=$1;
       if(!topRule && includeStackPtr == 0) {
	    topRule = rule_getLabel($1);
       }
  }
  | PACKAGE Label ';' {
#ifdef DEBUG
       printf("setting package to %s\n", $2);
#endif
       if(CURRENT_PACKAGE) {
	    free(CURRENT_PACKAGE);
       }
       CURRENT_PACKAGE = strdup($2);
       free($2);
       $$=NULL;
  }
  ;

Rule: /* A Rule consists of a label and a list of Choices, among other things */
   PackagedLabel ':' Choices {
     GRAMBIT *nr;

     nr = rule_new($1, $3, LEXICAL_SCOPE);

     free($1);

     $$=nr;
   }
   | PackagedLabel '(' Labels ')' ':' Choices { /* positional argument form */
	GRAMBIT *nr;
	LIST *choice, *terms;
	int i, len;

	/* the rhs of this type of rule is a single choice consisting of
	   1. positional variable bindings, followed by
	   2. an anon choice wrapping the original rhs */
	terms = list_new(); /* single choice for rhs */
	choice = list_addToNew(terms); /* rhs */

	/* for each positional argument */
	len = list_length($3);
	for(i = 0; i < len; i++) {
	     GRAMBIT *binding;
	     char *argName, *varName;

	     argName = (char *)list_get($3,i); /* positional arg name (e.g., "foo") */
	     varName = calloc(MAX_VAR_LENGTH,sizeof(char));
	     sprintf(varName,"_%d",i+1); /* positional var name (e.g., "_2") */

	     /* create a binding equivalent to e.g., {foo=_2} */
	     binding = binding_new(argName,label_new(varName),LEXICAL_SCOPE);
	     free(varName);

	     /* append this binding to the head of the rule's rhs */
	     list_add(terms, binding);
	}
	
	/* append the original rhs of the rule as an anon choice to the tail of the new rule's rhs */
	list_add(terms,choice_new($6));
	list_add(choice,terms);

	nr = rule_new($1, choice, LEXICAL_SCOPE);
	
	free($1);

	$$=nr;
   }
   | PackagedLabel '=' Choices { 
     GRAMBIT *na;

     na = assignment_new($1, $3, LEXICAL_SCOPE);

     free($1);

     $$=na;
   }
   | '$' PackagedLabel ':' Choices {
     GRAMBIT *nr;

     nr = rule_new($2, $4, DYNAMIC_SCOPE);
     free($2);

     $$=nr;
   }
   | '$' PackagedLabel '=' Choices { 
     GRAMBIT *na;

     na = assignment_new($2, $4, DYNAMIC_SCOPE);
     free($2);

     $$=na;
   }
;

Choices: /* Choices is a list of choices, one of which will be chosen */
   Choice {
	LIST *choices=list_new();

	list_add(choices,$1);
	$$=choices;
   }
   | Choice INTEGER {
	LIST *choices=list_new();
	{
	     int i;
	     for(i = 0; i < $2; i++) {
		  list_add(choices,$1);
	     }
	}
	$$=choices;
   }
   | Choices ',' Choice {
	list_add($1,$3);
	$$=$1;
   }
   | Choices '|' Choice {
	list_add($1,$3);
	$$=$1;
   }
   | Choices ',' Choice INTEGER {
	{
	     int i;
	     for(i = 0; i < $4; i++) {
		  list_add($1,$3);
	     }
	}
	$$=$1;
   }
   | Choices '|' Choice INTEGER {
	{
	     int i;
	     for(i = 0; i < $4; i++) {
		  list_add($1,$3);
	     }
	}
	$$=$1;
   }
   ;

Choice: /* A Choice is a list of Terms */
   Terms {
	$$=$1;
   }
   | {
	$$=list_new();
   }
   ;

Terms:
   QualifiedTerm {
	LIST *terms=list_new();
	list_add(terms,$1);
	$$=terms;
   }
   | Terms QualifiedTerm {
	list_add($1,$2);
	$$=$1;
   }
   ;

QualifiedTerm: /* A QualifiedTerm is a term with a repetition of transformation qualifier */
   Term {
	$$=$1;
   }
   | QualifiedTerm '<' INTEGER ',' INTEGER '>' {
	$$=$1;
	$1->min_x = $3;
	$1->max_x = $5;
   }
   | QualifiedTerm '<' INTEGER '>' {
        $$=$1;
	$1->min_x = $3;
	$1->max_x = $3;
   }
   | QualifiedTerm '*' {
        $$=$1;
	$1->min_x = 0;
	$1->max_x = 5;
   }
   | QualifiedTerm '+' {
        $$=$1;
	$1->min_x = 1;
	$1->max_x = 5;
   }
   | QualifiedTerm '?' {
        $$=$1;
	$1->min_x = 0;
	$1->max_x = 1;
   }
   | QualifiedTerm '>' Term {
	$$=trans_new($1,$3);
   }
   ;

Term: /* A Term is a GRAMBIT */
   PackagedLabel { 
     $$=label_new($1);
     free($1);
   }
   | PackagedLabel '(' Choices ')' {
	LIST *choices = list_new();
	LIST *assignments = list_new();
	int len=list_length($3), i;
	/* rewrite the choices as a single choice consisting of a list
	   of assignments to positional variables */
	for(i = 0; i < len; i++) {
	     LIST *rhs = list_new();
	     GRAMBIT *ass;
	     char *varName = calloc(MAX_VAR_LENGTH,sizeof(char));
	     sprintf(varName,"_%d",i+1); /* lhs of assignment */
	     list_add(rhs,list_get($3,i)); /* rhs of assignment */
	     ass = assignment_new(varName, rhs, LEXICAL_SCOPE);
	     list_add(assignments, ass);
	}
	/* now add the label to the *end* of the assignments */
	list_add(assignments,label_new($1));
	free($1);
	/* now construct the single choice */
	list_add(choices,assignments);
	$$=choice_new(choices);
   }
   | LITERAL {
     $$=literal_new($1);
     free($1);
   }
   | '{' Rule '}' {
     $$=$2;
   }
   | '{' Choices '}' {
     $$=choice_new($2);
   }
   | RXSUB {
     $$=rxsub_new($1.rx, $1.rep);
     free($1.rx);
     free($1.rep);
   }
   | LITERAL '%' QualifiedTerm {
     $$=mapping_new($1,$3);
     free($1);
   }
   ;

PackagedLabel:
   LABEL {
	if(CURRENT_PACKAGE) {
	     char *dotLabel;
	     dotLabel = gstr_cat(".",$1);
	     $$ = gstr_cat(CURRENT_PACKAGE,dotLabel);
	     free(dotLabel);
	     free($1);
	} else {
	     $$=$1;
	}
   }
   | Namespace LABEL {
#ifdef DEBUG
	printf("namespace = %s\n",$1);
#endif
	$$=gstr_catf($1,$2);
   }
   ;

Labels:
   LABEL {
	LIST *labels = list_new();
	list_add(labels,$1);
	$$=labels;
   }
   | Labels ',' LABEL {
	list_add($1,$3);
	$$=$1;
   }
   ;

Label:
   LABEL {
	$$=$1;
   }
   ;

Namespace:
   LABEL '.' {
	$$=gstr_cat($1,".");
	free($1);
   }
   | Namespace LABEL '.' {
        $$=gstr_catf($1,gstr_cat($2,"."));
	free($2);
   }
   ;
%%

int yyerror(char *s) {
     if(includeStackPtr) {
	  fprintf(stderr,"error on line %d of %s: %s\n",g_lineNumber[includeStackPtr]+1,g_fileName[includeStackPtr],s);
     } else {
	  fprintf(stderr,"error on line %d: %s\n",g_lineNumber[0]+1,s);
     }
  return 0;
}
