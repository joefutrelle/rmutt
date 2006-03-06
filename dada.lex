%{

#include <stdlib.h>
#include <string.h>

#include "global.h"
#include "dada.tab.h"
#include "gstr.h"

GSTR *string;
GSTR *rx;
GSTR *replace;
char *includeFile;

/* this is used to support older versions of flex (e.g., 2.5.4)
   that do not have %option reentrant */
#define YY_DECL int yylex (YYSTYPE *lvalp)

#define MAX_INCLUDE_DEPTH 10
YY_BUFFER_STATE include_stack[MAX_INCLUDE_DEPTH];
int includeStackPtr = 0;

int g_lineNumber[MAX_INCLUDE_DEPTH];
char *g_fileName[MAX_INCLUDE_DEPTH];
char *g_package[MAX_INCLUDE_DEPTH];

%}

%x STR
%x SUB1
%x SUB2
%x COMMENT
%x INCLUDE

white           [ \t]+
newline         [\n]
label           [A-Za-z_][A-Za-z0-9_\-]*
integer         [0-9]+
special         [:();|=\[\]{},.><%&+*?$]
filename        [-A-Za-z0-9_./ ]+
%%

package { return(PACKAGE); }
use { return(USE); }

\/\/ { BEGIN(COMMENT); }
<COMMENT>\n {
     g_lineNumber[includeStackPtr]++;
     BEGIN(INITIAL);
}
<COMMENT>. { }

\" { string = gstr_new(""); BEGIN(STR); }

<STR>\" {
     BEGIN(INITIAL);
     lvalp->str = gstr_detach(string);
     return(LITERAL);
}

<STR>\\n { gstr_appendc(string,'\n'); }
<STR>\\t { gstr_appendc(string,'\t'); }
<STR>\\\" { gstr_appendc(string,'\"'); }
<STR>\\\\ { gstr_appendc(string,'\\'); }

<STR>. {
     gstr_append(string,yytext);
}

\/ { rx = gstr_new(""); BEGIN(SUB1); }
<SUB1>\/ {
     lvalp->rx.rx = gstr_detach(rx);
     replace = gstr_new("");
     BEGIN(SUB2);
}
<SUB1>\\n { gstr_appendc(rx,'\n'); }
<SUB1>\\t { gstr_appendc(rx,'\t'); }
<SUB1>\\\" { gstr_appendc(rx,'\"'); }
<SUB1>\\\\ { gstr_appendc(rx,'\\'); }
<SUB1>. { gstr_append(rx,yytext); }

<SUB2>\/ {
     lvalp->rx.rep = gstr_detach(replace);
     BEGIN(INITIAL);
     return(RXSUB);
}
<SUB2>\\n { gstr_appendc(replace,'\n'); }
<SUB2>\\t { gstr_appendc(replace,'\t'); }
<SUB2>\\\" { gstr_appendc(replace,'\"'); }
<SUB2>\\\\ { gstr_appendc(replace,'\\'); }
<SUB2>. { gstr_append(replace,yytext); }

{white}         { }
{newline} {
     g_lineNumber[includeStackPtr]++;
}

{label} {
     lvalp->str = strdup(yytext);
     return(LABEL);
}

{integer} {
     lvalp->integer = atoi(yytext);
     return(INTEGER);
}

{special} {
     return(yytext[0]);}

\#include[ \t]+\" {
     BEGIN(INCLUDE);
}

<INCLUDE>{filename} {
     includeFile = strdup(yytext);
}

<INCLUDE>\"[ \t]*\n {
     FILE *f;
     char *sharedPath;

     g_lineNumber[includeStackPtr]++; /* the #include line counts as a line in this file */

     if ( includeStackPtr >= MAX_INCLUDE_DEPTH )
     {
	  fprintf( stderr, "Includes nested too deeply" );
	  exit( 1 );
     }

        f = fopen( includeFile, "r" );

	if(!f) { /* not found locally. try shared location */
	     sharedPath = (char *)malloc(strlen(includeFile) + strlen(RMUTT_INCLUDE) + 1);
	     sprintf(sharedPath, "%s/%s", RMUTT_INCLUDE, includeFile);
	     f = fopen(sharedPath, "r");
	     free(sharedPath);
	}

        if ( ! f ) {
	     fprintf(stderr, "warning: include file not found: %s\n", includeFile);
	} else {
	     yyin = f;

	     include_stack[includeStackPtr++] =
		  YY_CURRENT_BUFFER;

	     g_lineNumber[includeStackPtr] = 0;
	     g_fileName[includeStackPtr] = includeFile;

	     yy_switch_to_buffer(
		  yy_create_buffer( yyin, YY_BUF_SIZE ) );
	}

	/* free(includeFile); */

	BEGIN(INITIAL);
}

<<EOF>> {
     if ( !includeStackPtr ) {
	  yyterminate();
     } else {
	  yy_delete_buffer( YY_CURRENT_BUFFER );
	  free(g_fileName[includeStackPtr]);
	  includeStackPtr--;
	  yy_switch_to_buffer(
	       include_stack[includeStackPtr] );
     }
}
