%{

#include <stdlib.h>
#include <string.h>

#include "global.h"
#include "dada.tab.h"
#include "gstr.h"

GSTR *string;
GSTR *rx;
GSTR *replace;

%}

%x STR
%x SUB1
%x SUB2
%x COMMENT

white           [ \t\n]+
label           [A-Za-z_][A-Za-z0-9_\-]*
integer         [0-9]+
special         [:();|=\[\]{},.><]
%%

package { return(PACKAGE); }
use { return(USE); }

\/\/ { BEGIN(COMMENT); }
<COMMENT>\n { BEGIN(INITIAL); }
<COMMENT>. { }

\" { string = gstr_new(""); BEGIN(STR); }

<STR>\" {
     BEGIN(INITIAL);
     yylval.str = gstr_detach(string);
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
     yylval.rx.rx = gstr_detach(rx);
     replace = gstr_new("");
     BEGIN(SUB2);
}
<SUB1>. { gstr_append(rx,yytext); }
<SUB2>\/ {
     yylval.rx.rep = gstr_detach(replace);
     BEGIN(INITIAL);
     return(RXSUB);
}
<SUB2>. { gstr_append(replace,yytext); }

{white}         { }

{label} {
     yylval.str = strdup(yytext);
     return(LABEL);
}

{integer} {
     yylval.integer = atoi(yytext);
     return(INTEGER);
}

{special} {
     return(yytext[0]);
}
