#include <time.h>
#include <unistd.h>
#include "dict.h"
#include "list.h"
#include "grambit.h"
#include "grammar.h"

#include "gstr.h"

#include "rxutil.h"

extern int yyparse(void);
extern FILE *yyin;

DICT *grammar;
char *topRule = NULL;

int maxStackDepth = -1;

void usage() {
     fprintf(stderr,"usage: rmutt [-s] [grammar]\n");
     fprintf(stderr,"       -s   max stack depth\n");
}

int main(int argc, char **argv) {
  GRAMBIT *t;
  char *result;
  FILE *in = stdin;
  char c;

  srandom(time(NULL));

  while((c = getopt(argc, argv, "s:")) != EOF) {
       switch(c) {
       case 's':
	    /* maximum stack depth */
	    maxStackDepth = atoi(optarg);
	    break;
       default:
	    usage();
	    exit(-1);
       }
  }

  if(argc-optind == 1) {
       in = fopen(argv[optind],"r");
       if(!in) {
	    fprintf(stderr,"error: unable to open file %s\n",argv[1]);
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
       fprintf(stderr,"dada error: empty grammar\n");
       exit(-1);
  }
  t = label_new(topRule);
  result = grammar_produce(grammar,t);
  printf("%s",result);
  free(result);
  grambit_free(t);

  /*
  for(i = 0; i < list_length(grammar); i++) {
    grambit_print((GRAMBIT *)((DICT_ENTRY *)list_get(grammar,i))->value,stdout);
    printf("\n");
  }
  */

  return 0;
}
