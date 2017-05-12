#include "seq_tree.h"
#include <stdlib.h>
#include <stdio.h>



seq_node * newSeqNode(seq_node * left, ast_node * right){
	seq_node * this=calloc(1, sizeof(seq_node));
	this->left=left;
	this->right=right;
	return this;
}

void freeSeqNode(seq_node * this){
	if(this == NULL)
		return;
	//printf("%s freeing \n\n\n", tokenString(this->right->data->token));
	freeSeqNode(this->left);
	freeastNode(this->right);
	free(this);
}

int printSeqNodeRec(seq_node * this, int instruction, int tab){
	if(this == NULL)
		return instruction;
	int i;
	
	instruction = printSeqNodeRec(this->left, instruction, tab);
	printTab(tab);
	printf("### Statement %d ###\n", instruction);
	instruction = printastNodeRec(this->right, instruction+1, tab);
	
	return instruction;
}

int printSeqNode(seq_node * this){
	printSeqNodeRec(this, 1, 0);
}

void printTab(int tab){
	int i;
	for(i = 0; i < tab; i++){
		putchar('\t');
	}
}
