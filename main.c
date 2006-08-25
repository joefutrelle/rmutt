#include <time.h>
#include <unistd.h>
#include <string.h>
#include "dict.h"
#include "list.h"
#include "grambit.h"
#include "grammar.h"
#include "choose.h"
#include "gstr.h"
#include "rxutil.h"

#define RMUTT_VERSION "3.0a"

extern int yyparse(void);
extern FILE *yyin;

GRAMMAR *grammar;
char *topRule = NULL;

int maxStackDepth = -1;

int dynamic = 0;

void version() {
     fprintf(stderr,"rmutt version %s\n",RMUTT_VERSION);
}

void usage() {
     version();
     fprintf(stderr,"usage: rmutt -[bdeirsv] grammar\n");
     fprintf(stderr,"       -s number      max stack depth\n");
     fprintf(stderr,"       -r number      random seed\n");
     fprintf(stderr,"       -i number      iteration\n");
     fprintf(stderr,"       -e name        name of rule to expand (default: first)\n");
     fprintf(stderr,"       -d             dynamic variable scope (default: lexical)\n");
     fprintf(stderr,"       -v             print version number and exit\n");
     fprintf(stderr,"       -b name=value  bind name to value in the grammar\n");
}

int main(int argc, char **argv) {
  GRAMBIT *t;
  char *result;
  FILE *in = stdin;
  char *rte = NULL;
  char c;
  LIST *bindingNames = list_new();
  LIST *bindingValues = list_new();
  int i, len; /* FIXME debug */

  srandom(time(NULL));

  while((c = getopt(argc, argv, "vs:r:i:e:db:")) != EOF) {
       int i = 0;
       switch(c) {
       case 'v':
	    version();
	    exit(1);
       case 's':
	    /* maximum stack depth */
	    maxStackDepth = atoi(optarg);
	    break;
       case 'r':
	    srandom(atoi(optarg));
	    break;
       case 'i':
	    choose_setIteration(atol(optarg));
	    break;
       case 'e':
	    rte = strdup(optarg);
	    break;
       case 'b':
	    for(i=0; *(optarg+i) != '='; i++) {
		 if(!*(optarg+i)) {
		      usage();
		      exit(-1);
		 }
	    }
	    list_add(bindingNames, (char *)strndup(optarg,i));
	    if(!*(optarg+i+1)) { usage(); }
	    list_add(bindingValues, (char *)strdup(optarg+i+1));
	    break;
       case 'd':
	    dynamic++;
	    break;
       default:
	    usage();
	    exit(-1);
       }
  }

  if(argc-optind == 1) {
       in = fopen(argv[optind],"r");
       if(!in) {
	    fprintf(stderr,"error: unable to open file %s\n",argv[optind]);
	    exit(-1);
       }
  }
  yyin = in;

#ifdef DEBUG
  fprintf(stderr,"parsing\n");
#endif
  /* call the parser (puts result in "grammar") */
  yyparse();
#ifdef DEBUG
  fprintf(stderr,"done\n");
  fflush(stdout);
#endif

  if(!grammar) {
#ifdef DEBUG
  fprintf(stderr,"no grammar returned from yyparse\n");
  fflush(stdout);
#endif
       exit(-1);
  }

  if(!topRule) {
       fprintf(stderr,"rmutt error: empty grammar\n");
       exit(-1);
  }
  if(rte) {
       t = label_new(rte);
  } else {
       t = label_new(topRule);
  }

  /* add the bindings from the command line */
  len = list_length(bindingValues);
  for(i = 0; i < len; i++) {
       GRAMBIT *assignment;
       LIST *choices = list_new(), *terms = list_new();
       char *name, *value;
       name = list_get(bindingNames,i);
       value = list_get(bindingValues,i);
       list_add(terms,literal_new(value));
       list_add(choices,terms);
       assignment = assignment_new(name,choices,DYNAMIC_SCOPE);
       grammar_add(grammar,assignment);
  }

  result = grammar_produce(grammar,t);
  fputs(result,stdout);
  free(result);
  grambit_free(t);

  return 0;
}
