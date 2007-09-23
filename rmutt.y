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
%token <str> LABEL
%token <str> LITERAL
%token <integer> INTEGER
%token <rx> RXSUB
%token <rx> RXMATCH

%type <str> Label
%type <list> Choice
%type <list> Body
%type <list> Arguments
%type <list> Terms
%type <grambit> Term
%type <grambit> QualifiedTerm
%type <grambit> Rule
%type <grambit> ScopedRule
%type <grambit> Statement
%type <dict> Grammar

%start Top

%pure-parser
%glr-parser

%expect 12

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
  | PACKAGE LABEL ';' {
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

ScopedRule:
  Rule {
       $1->scope = LOCAL_SCOPE; /* overrides default LOCAL scope in Rule production */
       $$=$1;
  }
  | '^' Rule {
       $2->scope = NON_LOCAL_SCOPE; /* overrides default LOCAL scope in Rule production */
       $$=$2;
  }
  | '$' Rule {
       $2->scope = GLOBAL_SCOPE; /* overrides default LOCAL scope in Rule production */
       $$=$2;
  }
  ;

Rule: /* A Rule consists of a label and a list of Choices, among other things */
   Label ':' Body {
     GRAMBIT *nr;

     nr = rule_new($1, $3, LOCAL_SCOPE);

     free($1);

     $$=nr;
   }
   | Label '[' Label ']' ':' Body { /* positional argument form */
	LIST *labels;

	labels = list_new();
	list_add(labels,$3);

	$$=rule_newWithArguments($1,labels,$6,LOCAL_SCOPE);
   }
   | Label '[' Label ',' Arguments ']' ':' Body { /* positional argument form */
	list_add($5,$3);

	$$=rule_newWithArguments($1,list_reverse($5),$8,LOCAL_SCOPE);
   }
   | Label '=' Body { 
     GRAMBIT *na;

     na = assignment_new($1, $3, LOCAL_SCOPE);

     free($1);

     $$=na;
   }
   ;

Arguments:
   Label {
	LIST *args = list_new();
	list_add(args,$1);
	$$=args;
   }
   | Label ',' Arguments {
	list_add($3,$1);
	$$=$3;
   }
   ;

Body: /* Body is a list of (at least two) choices, one of which will be chosen */
   Choice {
	LIST *choices=list_new();

	list_add(choices,$1);

	$$=choices;
   }
   |
   Choice INTEGER {
	LIST *choices=list_new();
	{
	     int i;
	     for(i = 0; i < $2; i++) {
		  list_add(choices,$1);
	     }
	}
	$$=choices;
   }
   | Body '|' Choice {
	list_add($1,$3);
	$$=$1;
   }
   | Body '|' Choice INTEGER {
	{
	     int i;
	     for(i = 0; i < $4; i++) {
		  list_add($1,$3);
	     }
	}
	$$=$1;
   }
   | Body ',' Choice {
	list_add($1,$3);
	$$=$1;
   }
   | Body ',' Choice INTEGER {
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
   | QualifiedTerm '{' INTEGER ',' INTEGER '}' {
	$$=$1;
	$1->min_x = $3;
	$1->max_x = $5;
   }
   | QualifiedTerm '{' INTEGER '}' {
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
   Label { 
     $$=label_new($1);
     free($1);
   }
   | Label '[' Body ']' {
	$$=call_new($1,$3);
   }
   | LITERAL {
     $$=literal_new($1);
     free($1);
   }
   | '(' ScopedRule ')' {
     $$=$2;
   }
   | '(' Body ')' {
     $$=choice_new($2);
   }
   | RXMATCH '%' QualifiedTerm {
     $$=rxmatch_new($1.rx,$3);
     free($1.rx);
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

Label:
   LABEL {
        if(CURRENT_PACKAGE) { /* we're in the non-default package */
	     char *fql = NULL; /* fully-qualified label */
	     char *c;
	     for(c=$1; *c; c++ && !fql) {
		  if(*c=='.') { /* it's already fully qualified */
		       fql=strdup($1);
		  }
	     }
	     if(!fql) {
		  fql = gstr_catf(gstr_cat(CURRENT_PACKAGE,"."),$1);
	     }
	     $$=fql;
	} else {
	     $$=$1;
	}
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
