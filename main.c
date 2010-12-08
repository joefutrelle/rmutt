#include <stdio.h>
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

#define RMUTT_VERSION "2.6"

extern int yyparse(void);
extern FILE *yyin;

GRAMMAR *grammar;
char *topRule = NULL;

int maxStackDepth = -1;

int dynamic = 0;

void version() {
    fprintf(stderr, "rmutt version %s\n", RMUTT_VERSION);
}

void usage() {
    version();
    fprintf(stderr, "usage: rmutt -[bdeirsv] grammar\n");
    fprintf(stderr, "       -s number      max stack depth\n");
    fprintf(stderr, "       -r number      random seed\n");
    fprintf(stderr, "       -i number      iteration\n");
    fprintf(stderr, "       -I file        read the iteration from a file\n");
    fprintf(stderr,
            "       -e name        name of rule to expand (default: first)\n");
    fprintf(stderr, "       -b name=value  bind name to value in the grammar\n");
    fprintf(stderr,
            "       -d             dynamic variable scoping (default: lexical)\n");
    fprintf(stderr, "       -v             print version number and exit\n");
}

int main(int argc, char **argv) {
    GRAMBIT *t;
    char *result;
    FILE *in = stdin;
    char *rte = NULL;
    char c;
    LIST *bindings = list_new();
    int i, len;

    srandom(time(NULL));

    while ((c = getopt(argc, argv, "vs:r:i:I:e:db:")) != EOF) {
        i = 0;
        switch (c) {
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
            choose_setIterationString(optarg);
            break;
        case 'I': {
            FILE *of = fopen(optarg, "r");
            if (!of) {
                fprintf(stderr, "error: unable to open iteration file %s\n",
                        optarg);
                exit(-1);
            }
            choose_setIterationFile(of);
        }
            break;
        case 'e':
            rte = strdup(optarg);
            break;
        case 'b': {
            int i;
            char *name, *value;

            for (i = 0; *(optarg + i) != '='; i++) {
                if (!*(optarg + i)) {
                    usage();
                    exit(-1);
                }
            }
            if (!*(optarg + i + 1)) {
                usage();
            }

            name = (char *) calloc(sizeof(char), i + 1);
            strncpy(name, optarg, i);
            value = (char *) strdup(optarg + i + 1);

            list_add(bindings, binding_new(name, literal_new(value),
                    GLOBAL_SCOPE));
        }
            break;
        case 'd':
            dynamic++;
            break;
        default:
            usage();
            exit(-1);
        }
    }

    if (argc - optind == 1) {
        in = fopen(argv[optind], "r");
        if (!in) {
            fprintf(stderr, "error: unable to open file %s\n", argv[optind]);
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

    if (!grammar) {
#ifdef DEBUG
        fprintf(stderr,"no grammar returned from yyparse\n");
        fflush(stdout);
#endif
        exit(-1);
    }

    /* add the bindings from the command line */
    len = list_length(bindings);
    for (i = 0; i < len; i++) {
        grammar_add(grammar, list_get(bindings, i));
    }

    /* if there's no top rule, the grammar is empty */
    if (!topRule) {
        fprintf(stderr, "rmutt error: empty grammar\n");
        exit(-1);
    }

    /* determine which rule to expand */
    if (rte) {
        t = label_new(rte); /* the one from the command line, or */
    } else {
        t = label_new(topRule); /* the (default) top rule */
    }

    /* produce the output */
    result = grammar_produce(grammar, t);

    fputs(result, stdout);

    /*free(result);
     grambit_free(t);*/

    return 0;
}
