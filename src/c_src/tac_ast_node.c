#include "tac_ast_node.h"
#include "factype_ast.h"
#include "factype_tac.h"
#include "tac.h"
#include <stdio.h>
#include <limits.h>

extern void yyerror(char *);
static
void _init_tac_node(tac_node ** node);
static
void _init_tac_value(tac_node ** node, void * value);
static
void _connect_tac_nodes(tac_node ** predecessor, tac_node ** current);

static
void _tac_fract(tac_node * current, ast_node * node);
static
void _tac_bool(tac_node * current, ast_node * node);
static
void _tac_id(tac_node * current, ast_node * node);
static
void _tac_print(tac_node * current, ast_node * node);


/**
* @brief builds the 3AC list of triples from the last to the first node by
*	traversing the AST bottom-up
* @note opt_t and tac_op have the same value because the enums follow
* 		the same declaration order
*
*/
void tac_ast_node(ast_node * node, tac_node ** predecessor, tac_node * current,
	stack_t * stack){
	printf("----TAC_AST-NODE-----\n");
	if(!node || !node->data)
		return yyerror("TAC - malformed AST, null node found");
	switch(node->data->token){
		/* Internal nodes */
		case AST_AOP1:
		case AST_BOP1:
		{
			printf("AOP1.BOP1\n");
			if(current)
				current=current->next;
			_init_tac_node(&current);
			current->value->op = node->data->op;
			tac_node * aux = current;
			return tac_ast_node(node->ast_children[0], predecessor, aux, stack);
		}
		case AST_AOP2:
		case AST_BOP2:
		case AST_RELOP:
		{
			printf("AOP2.BOP2.RELOP\n");
			if(current)
				current=current->next;
			_init_tac_node(&current);
			current->value->op = node->data->op;
			tac_node * aux = current;
			/* compute right branch */
			tac_ast_node(node->ast_children[1], predecessor, aux, stack);
			tac_entry * instruction1 = aux->value;
			/* compute left branch */
			tac_ast_node(node->ast_children[0], predecessor, aux, stack);
			tac_entry * instruction0 = aux->value;
			if(!current->value->arg0){
				current->value->arg0 = malloc(sizeof(tac_value));
				current->value->arg0->instruction = instruction0;
			}
			if(!current->value->arg1){
				current->value->arg1 = malloc(sizeof(tac_value));
				current->value->arg1->instruction = instruction1;
			}
			return;
		}
		case AST_ASSIGNMENT:
		{
			printf("ASS\n");
			if(current)
				current=current->next;
			/* build this node */
			_init_tac_node(&current);
			current->value->op = TAC_ASSIGNMENT;
			/* left side of assignment */
			current->value->arg0 = malloc(sizeof(tac_value));
			current->value->arg0->address = lookupID(node->ast_children[0]->data->value);
			tac_node * aux = current;
			/* right side of assignment */
			return tac_ast_node(node->ast_children[1], predecessor, aux, stack);
		}
		/* Leaves */
		case AST_FRACT:{
			_tac_fract(current, node);
			return _connect_tac_nodes(predecessor, &current);
		}
		case AST_BOOL:{/*constants*/
			_tac_bool(current, node);
			return _connect_tac_nodes(predecessor, &current);
		}
		case AST_ID:
		{
			_tac_id(current, node);
			return _connect_tac_nodes(predecessor, &current);
		}
		case AST_PRINT:
		{
			printf("PRINT");
			if(current)
				current=current->next;
			_init_tac_node(&current);
			_tac_print(current, node);
			return _connect_tac_nodes(predecessor, &current);
		}
		case AST_SKIP:
		case AST_DECLARATION:{
			printf("DECL.SKIP\n");
			return;
		}
		default:
			yyerror("TAC - token not recognized");
	}
}

/********************************************
			PRIVATE FUNCTIONS
*********************************************/
static
void _init_tac_node(tac_node ** node){
	(*node) = malloc(sizeof(tac_node));
	(*node)->value = malloc(sizeof(tac_entry));
	(*node)->value->arg0 = NULL;
	(*node)->value->arg1 = NULL;
}

static
void _connect_tac_nodes(tac_node ** predecessor, tac_node ** current){
	if((*predecessor))
		(*predecessor)->next = (*current);
	else
		(*predecessor) = (*current);
	(*current)->next=NULL;
	printf("==============predecessor %p\n", (*predecessor));
	printf("==============current %p\n", (*current));
}

static
void _tac_fract(tac_node * current, ast_node * node){
	printf("FRACT\n");
	if(current->value->arg0){
		printf("FRACT-arg1\n");
		current->value->arg1 = malloc(sizeof(tac_value));
		current->value->arg1->fract = (fract_t*) node->data->value;
		return;
	}
	printf("FRACT-arg0\n");
	current->value->arg0 = malloc(sizeof(tac_value));
	current->value->arg0->fract = (fract_t*) node->data->value;
}

static
void _tac_bool(tac_node * current, ast_node * node){
	printf("BOOL\n");
	if(current->value->arg0){
		current->value->arg1 = malloc(sizeof(tac_value));
		current->value->arg1->boolean = node->data->value;
		return;
	}
	current->value->arg0 = malloc(sizeof(tac_value));
	current->value->arg0->boolean = node->data->value;
}

static
void _tac_id(tac_node * current, ast_node * node){
	printf("ID\n");
	if(current->value->arg0){
		printf("ID-arg1\n");
		current->value->arg1 = malloc(sizeof(tac_value));
		current->value->arg1->address = lookupID(node->data->value);
		return;
	}
	printf("ID-arg0\n");
	current->value->arg0 = malloc(sizeof(tac_value));
	current->value->arg0->address = lookupID(node->data->value);
}

static
void _tac_print(tac_node * current, ast_node * node){
	current->value->op = TAC_PRINT;
	current->value->arg0 = malloc(sizeof(tac_value));
	current->value->arg0->address = lookupID(node->ast_children[0]->data->value);
	printf("-ADDRESS =%p\n",current->value->arg0->address);
}