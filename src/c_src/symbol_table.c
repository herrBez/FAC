#include "symbol_table.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

extern symbol_table_entry * symbol_table;
extern void yyerror(char *, ...);


void installID(char* id, type_t type) {
	symbol_table_entry * e;
	HASH_FIND_STR(symbol_table, _id, e);
	if(e != NULL) { /* ID ALREADY INSTALLED -> ERROR */
		yyerror("The id %s is already installed", _id);
	}
	e = (symbol_table_entry*)malloc(sizeof(symbol_table_entry));
	e->id = malloc(sizeof(char) * (strlen(id) + 1)); //strlen does not take into account '\0'
	strcpy(e->id, id);
	e->type = type;
	e->value = NULL;

	HASH_ADD_KEYPTR(hh, symbol_table, e->id, strlen(e->id), e);
}


symbol_table_entry * lookupID(char* id) {
	symbol_table_entry * e;
	HASH_FIND_STR(symbol_table, id, e);
	if(e == NULL){
		yyerror("Failed lookup: The variable %s is not yet installed", _id);
	}
	return e;
}

type_t getType(char * id) {
	symbol_table_entry * e;
	HASH_FIND_STR(symbol_table, id, e);
	if(e == NULL){
		yyerror("Failed lookup: The variable %s is not yet installed", _id);
	}
	return e->type;
}


void setValue(char * id, void * value) {
	symbol_table_entry * e;
	HASH_FIND_STR(symbol_table, id, e);
	if(e == NULL){
		yyerror("The variable cannot be assigned because %s is not yet installed", _id);
	}
}

void freeTable(){
	symbol_table_entry * e, *tmp;
	HASH_ITER(hh, symbol_table, e, tmp) {
		free(e->id);
		if(e->value != NULL)
			free(e->value);
		HASH_DEL(symbol_table, e);
		free(e);
	}
}
