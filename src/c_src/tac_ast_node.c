#include "tac_ast_node.h"
#include "factype_ast.h"
#include "factype_tac.h"
#include "tac.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
extern void yyerror(const char *, ...);

static
tac_node * _tac_node();
static
tac_list * _tac_connect(tac_list *, tac_node *);

tac_list * _tac_append(tac_list *, tac_list *);

static
tac_node * _tac_print(ast_node *);
static
tac_value * _tac_leaf(ast_node * node);

static
tac_node * _tac_label();

static
tac_node * _tac_goto_unconditioned(tac_node * destination);
static
tac_node * _tac_goto_conditioned(tac_entry * condition, tac_node * destination);

int check=0;
/**
* @brief builds the 3AC list of triples from the last to the first node by
*	traversing the AST bottom-up
* @note opt_t and tac_op have the same value because the enums follow
* 		the same declaration order
* @note do not free the goto NULL NULL node, reuse it to keep consistency
*/
tac_list * tac_ast_node(ast_node * node){
	if(node == NULL || node->data == NULL){
		yyerror("TAC - malformed AST, null node found");
		exit(EXIT_FAILURE);
	}
	switch(node->data->token){
		/* Internal nodes */
		case AST_AOP1:
		case AST_BOP1:
		{
			tac_list* tlist = calloc(1, sizeof(tac_list));
			
			tac_node* tnode=_tac_node();
			tnode->value->op = node->data->op;
			
			tac_value * left = _tac_leaf(node->ast_children[0]);
			
			if(left != NULL){ /* It is a leaf */
				tnode->value->arg0 = left;
			} else {
				tac_list * left_list = tac_ast_node(node->ast_children[0]);
				tnode->value->arg0= calloc(1, sizeof(tac_value));
				tnode->value->arg0->instruction= left_list->last->value;
				tlist = _tac_append(tlist, left_list);
				
			}
			
			
			/* connect tnode to the current list of triples */
			return _tac_connect(tlist, tnode);
		}
		case AST_AOP2:
		case AST_BOP2:
		case AST_RELOP:
		{
			tac_list * tlist = calloc(1, sizeof(tac_list));
			/* current node */
			tac_node* tnode=_tac_node();
			tnode->value->op = node->data->op;
			
			/* compute right subtree -
			   NOTE: it also connects it to the current list of triples */
			{ 
				tac_value * left = _tac_leaf(node->ast_children[0]);
				
				if(left != NULL){
					tnode->value->arg0 = left;
				} else {
					tac_list* left = tac_ast_node(node->ast_children[0]);
					tnode->value->arg0= calloc(1, sizeof(tac_value));
					tnode->value->arg0->instruction= left->last->value;
					tlist = _tac_append(tlist, left);
				}
			}
			{
				tac_value * right = _tac_leaf(node->ast_children[1]);
				if(right != NULL){
					tnode->value->arg1 = right;
				} else {
					tac_list* right = tac_ast_node(node->ast_children[1]);
					tnode->value->arg1= calloc(1, sizeof(tac_value));
					tnode->value->arg1->instruction= right->last->value;
					tlist = _tac_append(tlist, right);
				}
			}
			return _tac_connect(tlist, tnode);
		}
		case AST_ASSIGNMENT:
		{
			printf("Here we go\n");
			/* create a new tac node with op = TAC_ASSIGNMENT and arg0 = id */
			tac_list * tlist = calloc(1, sizeof(tac_list));
			
			tac_node* tnode=_tac_node();
			tnode->value->op = TAC_ASSIGNMENT;
			tnode->value->arg0 = calloc(1, sizeof(tac_value));
			tnode->value->arg0->address = (symbol_table_entry*)
				lookupID(node->ast_children[0]->data->value);
			
			/* Compute the value of the rhs */
			tac_value * leaf = _tac_leaf(node->ast_children[1]);
			
			if(leaf != NULL){ /* It is leaf */
				tnode->value->arg1 = leaf;
			} else { /* It is a whole list of instructions */
				tac_list* right = tac_ast_node(node->ast_children[1]);
				tnode->value->arg1 = calloc(1, sizeof(tac_value));
				tnode->value->arg1->instruction =  right->last->value;
				tlist = _tac_append(tlist, right);
			}
			/* connect tnode to the current list of triples */
			return _tac_connect(tlist, tnode);
		}
		
		case AST_WHILE:
		{
			tac_list * tlist = calloc(1, sizeof(tac_list));
			
			// Create a label for the bexpr and connect it to the actual tlist
			tac_node * start_bexpr = _tac_label();
			tlist = _tac_connect(tlist, start_bexpr);
			
			
			// Create a label that points to the end of the body
			tac_node * end_while_label = _tac_label();
			
			
			
			
			
			tac_value * bexpr = _tac_leaf(node->ast_children[0]);
			
			
			
			
			if(bexpr != NULL){ //It is a leaf
				
				tac_node * node = _tac_node();
				node->value->op = TAC_NOT;
				node->value->arg0 = bexpr;
				tlist = _tac_connect(tlist, node);
				
			} else {
				tac_list * bexpr = tac_ast_node(node->ast_children[0]);
				tac_node * negateCondition = _tac_node();
				negateCondition->value->op = TAC_NOT;
				negateCondition->value->arg0 = calloc(1, sizeof(tac_value));
				negateCondition->value->arg0->instruction = bexpr->last->value;
				tlist = _tac_append(tlist, bexpr);
				tlist = _tac_connect(tlist, negateCondition);
			}
			
			tac_node * goto_skip_while_body = _tac_goto_conditioned(tlist->last->value, end_while_label);
			tlist = _tac_connect(tlist, goto_skip_while_body);
			
			tac_list * stmt = generate_tac(node->seq_children[0]);
			tlist = _tac_append(tlist, stmt);
						
			// generate goto node that points to the start_bexpr label 
			tac_node * goto_node = _tac_goto_unconditioned(start_bexpr);
			tlist = _tac_connect(tlist, goto_node);
			
			
			tlist = _tac_connect(tlist, end_while_label);
			return tlist;
		}
		/*case AST_IF:
		{
			
			//Calculate list containing tlist extended with bexpr code 
			tlist = tac_ast_node(node->ast_children[0], tlist);
			//adjust the bexpr - if it is a leaf 
			if(!tlist->last->value->arg0){
				tlist->last->value->op=TAC_COND;
				tlist->last->value->arg0=tlist->last->value->arg1;
				tlist->last->value->arg1=NULL;
			}
			// add the code to compute the negation of the condition and add it to the list
			tac_node * last_instruction = tlist->last;
			tac_node * negateCondition = _tac_node();
			negateCondition->value->op = TAC_NOT;
			negateCondition->value->arg0 = malloc(sizeof(tac_value));
			negateCondition->value->arg0->instruction = last_instruction->value;
			tlist = _tac_connect(tlist, negateCondition);
			
			//calculate tlist of the stmt following bexpr 
			tac_list * stmt = generate_tac(node->seq_children[0]);
			
			
			tac_node * end_if_body = _tac_label();
			
			 Create a conditioned goto that onTrue of the negate condition goes to end_while_label 
			tac_node * goto_skip_if_body = _tac_goto_conditioned(negateCondition->value, end_if_body);
			
			
			if(node->number_of_seq_children == 1){
				tlist = _tac_connect(tlist, goto_skip_if_body);
				tlist = _tac_append(tlist, stmt);
				tlist = _tac_connect(tlist, end_if_body);
				return tlist;
			} else if(node->number_of_seq_children == 2) {
				printf("HERE\n");
				tac_node * end_else_body = _tac_label();
				tlist = _tac_connect(tlist, goto_skip_if_body);
				tlist = _tac_append(tlist, stmt);
				// Create a conditioned goto that onTrue of the negate condition goes to end_while_label 
				tac_node * goto_to_end = _tac_goto_unconditioned(end_else_body);
				tlist = _tac_connect(tlist, goto_to_end);
				tlist = _tac_connect(tlist, end_if_body);
				//calculate tlist of the stmt following bexpr 
				tac_list * body_else = generate_tac(node->seq_children[1]);
				tlist = _tac_append(tlist, body_else);
				tlist = _tac_connect(tlist, end_else_body);
				return tlist;
				
			} else {
				char * s = malloc(sizeof(20));
				strcpy(s, __FILE__);
				
				yyerror("%s: IF with more than two children?", s);
			}
			
		}*/
		/* Leaves */
		case AST_PRINT: /* one child subtree */
		{
			tac_list * tlist = calloc(1, sizeof(tlist));
			return _tac_connect(tlist, _tac_print(node));
		}
		case AST_DECLARATION:
			return calloc(1, sizeof(tac_list));
		default:
			yyerror("TAC - token not recognized");
			exit(EXIT_FAILURE);
	}
}

/********************************************
			PRIVATE FUNCTIONS
*********************************************/
static
tac_node* _tac_node(){
	tac_node* node;
	node = malloc(sizeof(tac_node));
	node->value = malloc(sizeof(tac_entry));
	node->value->op = -1;
	node->value->arg0 = NULL;
	node->value->arg1 = NULL;
	node->prev=NULL;
	node->next=NULL;
	return node;
}








tac_list * _tac_connect(tac_list * tlist, tac_node * tnode){
	if(!tlist->last && !tlist->first){
		/* first node of the TAC list - nothing to connect */
		tlist->first=tnode;
		tlist->last=tnode;
	}
	else{
		if(tnode == NULL)
			return tlist;
		printf("CONNECT : %p -> %p\n", tlist->last->value, tnode->value);
		tlist->last->next=tnode;
		tnode->prev=tlist->last;
		tlist->last=tlist->last->next;
	}
	return tlist;
}


tac_list * _tac_append(tac_list * tlist, tac_list * to_append){

	if(!to_append->first && !to_append->last)
		return tlist;
	if(!tlist->last && !tlist->first){
		/* first node of the TAC list - nothing to connect */
		tlist->first=to_append->first;
		tlist->last=to_append->last;
		return tlist;
	}
	tlist->last->next=to_append->first;
	to_append->first->prev=tlist->last;
	tlist->last=to_append->last;
	to_append->first=tlist->first;
	return tlist;
}


static
tac_value * _tac_leaf(ast_node * node){
	tac_value * val = NULL;
	switch(node->data->token){
		case AST_FRACT:
		{
			val = calloc(1,sizeof(tac_value));
			val->fract = (fract_t*) node->data->value;
			break;
		}
		case AST_BOOL:
		{
			val = calloc(1,sizeof(tac_value));
			val->boolean =  *(bool*) node->data->value;
			break;
		}
		case AST_ID:
		{
			val = calloc(1,sizeof(tac_value));
			val->address = (symbol_table_entry *) 
				lookupID(node->data->value);
			break;
		}
		default : break;
	}
	
	return val;
}

static
tac_node * _tac_print(ast_node * node){
	tac_node * tnode=_tac_node();
	tnode->value->op= TAC_PRINT;
	tnode->value->arg0= calloc(1, sizeof(tac_value));
	tnode->value->arg0->address=
		(symbol_table_entry *) lookupID(node->ast_children[0]->data->value);
	return tnode;
}
static
tac_node * _tac_label(){
	tac_node * label = _tac_node();
	label->value->op = TAC_LABEL;
	label->value->arg0=NULL;
	label->value->arg1=NULL;
	label->prev=NULL;
	label->next=NULL;
	return label;
}

static
tac_node * _tac_goto_unconditioned(tac_node * destination){
	tac_node * goto_node=_tac_node();
	goto_node->value->op=TAC_GOTO;
	goto_node->value->arg0=malloc(sizeof(tac_value));
	goto_node->value->arg0->instruction=destination->value;
	return goto_node;
}
static
tac_node * _tac_goto_conditioned(tac_entry * condition, tac_node * destination){
	tac_node * goto_node=_tac_node();
	goto_node->value->op=TAC_GOTO;
	goto_node->value->arg0=malloc(sizeof(tac_value));
	goto_node->value->arg0->instruction=condition;
	goto_node->value->arg1=malloc(sizeof(tac_value));
	goto_node->value->arg1->instruction=destination->value;
	return goto_node;
}
