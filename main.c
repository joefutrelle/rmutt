#include <time.h>
#include "dict.h"
#include "list.h"
#include "grambit.h"
#include "grammar.h"

#include "gstr.h"

#include "rxutil.h"

extern int yyparse(void);

DICT *grammar;
char *topRule = NULL;

int main(int argc, char **argv) {
  GRAMBIT *t;
  char *result;

  srand(time(NULL));

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

  return 1;
}
