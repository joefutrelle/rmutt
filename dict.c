/*
 * this trie-based dict is optimized for alphabetic lookup.
 * it has exactly the same API as the dict module and can be
 * used in its place
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "dict.h"
#include "mem.h"

/* create a new dictionary entry (key + value) */
DICT_ENTRY *dictEntry_new(char *key, void *value) {
     DICT_ENTRY *nde = (DICT_ENTRY *)malloc(sizeof(DICT_ENTRY));
     nde->key = strdup(key);
     nde->value = value;
     return nde;
}

DICT *dict_new() {
     DICT *nd = calloc(1,sizeof(DICT));
     nd->entries = list_new();
     return nd;
}

void dict_free(DICT *d) {
     /* recursively free a DICT */
     int i;
     for(i = 0; i < 26; i++) {
	  if(d->children[i]) {
	       dict_free(d->children[i]);
	  }
     }
     list_free(d->entries);
     free(d);
}

void dict_freeValues(DICT *d, Destructor destructo) {
     /* recursively free a DICT */
     int i;
     for(i = 0; i < 26; i++) {
	  if(d->children[i]) {
	       dict_freeValues(d->children[i], destructo);
	  }
     }
     list_freeData(d->entries, destructo);
     free(d);
}

void _dict_put(DICT *d, char *key, char *orig_key, void *data) {
     char *kp = key;
     int ix;
#ifdef DEBUG
     printf("putting %s\n", key); /* debug */
#endif
     /* if the key is empty, we've found or created the appropriate
	leaf node and now just need to append an entry to it */
     if(!*kp) {
#ifdef DEBUG
	  printf("adding new entry %s\n", orig_key);
	  fflush(stdout);
#endif
	  list_add(d->entries,dictEntry_new(orig_key,data));
	  return;
     }
     /* otherwise we need to skip non-alpha chars */
     while(!isalpha(*kp)) {
	  kp++;
	  if(!*kp) { /* last char is non-alpha */
#ifdef DEBUG
	       printf("last char of %s is non-alpha\n",orig_key);
#endif
	       /* add to this node and quit */
	       _dict_put(d, kp, orig_key, data);
	       return;
	  }
     }
     ix = tolower(*kp) - 'a';
     /* if the appropriate subtrie does not exist, create it */
     if(!d->children[ix]) {
#ifdef DEBUG
	  printf("creating subtree for %c\n",*kp); /* debug */
#endif
	  d->children[ix] = dict_new();
     }
     /* recurse */
     _dict_put(d->children[ix],kp+1,orig_key,data);
}

void dict_put(DICT *d, char *key, void *data) {
     _dict_put(d, key, key, data);
}

void *_dict_get(DICT *d, char *key, char *orig_key) {
     char *kp = key;
     int ix;
     /* skip nonalphas */
     while(*kp && !isalpha(*kp)) {
	  kp++;
     }
#ifdef DEBUG
     printf("getting %s\n",kp); /* debug */
#endif
     if(!*kp) {
	  /* do a linear search of this node's entries */
	  int i, len;
	  len = list_length(d->entries);
#ifdef DEBUG
	  printf("searching %d entries for %s\n",len,orig_key); /* debug */
#endif
	  /* search from the end, so that new values shadow old */
	  for(i = len-1; i >= 0; i--) {
	       DICT_ENTRY *de = (DICT_ENTRY *)list_get(d->entries,i);
	       if(!strcmp(orig_key, de->key)) {
		    return de->value;
	       }
	  }
	  return NULL;
     }
     ix = tolower(*kp) - 'a';
     if(!d->children[ix])
	  return NULL;
     return _dict_get(d->children[ix], kp+1, orig_key);
}

void *dict_get(DICT *d, char *key) {
     return _dict_get(d, key, key);
}
