#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "gstr.h"

#define GROW_BY 128

GSTR *gstr_new(char *d) {
     GSTR *g = (GSTR *)malloc(sizeof(GSTR));
     g->size = 0;
     g->len = 0;
     g->buf = NULL;
     gstr_append(g, d);
     return g;
}

void gstr_appendn(GSTR *g, char *b, size_t len) {
     if(!b) return;
     if(!g->buf) {
	  g->buf = malloc(len);
	  g->size = len;
	  g->len = len;
	  strncpy(g->buf,b,len);
     } else {
	  if(g->size < g->len + len + 1) {
	       g->buf = realloc(g->buf, (g->size = g->len + len + GROW_BY));
	  }
	  strncpy(g->buf+g->len,b,len);
	  g->len += len;
     }
}

void gstr_append(GSTR *g, char *d) {
     size_t len;
     if(!d) return;
     len = strlen(d);
     if(!g->buf) {
	  g->buf = strdup(d);
	  g->size = len+1;
	  g->len = len;
     } else {
	  if(g->size < g->len + len + 1) {
	       g->buf = realloc(g->buf, (g->size = g->len + len + GROW_BY));
	  }
	  strcpy(g->buf+g->len,d);
	  g->len += len;
     }
}

void gstr_appendc(GSTR *g, char c) {
     char cs[2];
     cs[0] = c;
     cs[1] = '\0';
     gstr_append(g,cs);
}

char *gstr_detach(GSTR *g) {
     char *result;
     result = realloc(g->buf,g->len+1);
     free(g);
     return result;
}

char *gstr_cat(char *a, char *b) {
     GSTR *g = gstr_new(a);
     gstr_append(g,b);
     return gstr_detach(g);
}

/* concatenate and free */
char *gstr_catf(char *a, char *b) {
     GSTR *g = gstr_new(a);
     gstr_append(g,b);
     return gstr_detach(g);
     free(a);
     free(b);
}

