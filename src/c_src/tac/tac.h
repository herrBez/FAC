/**
 * @file
 * @brief Three Address Code (TAC) - translate an AST to the corresponding
 *	Three Address Code (TAC) representation using indirect triples. It is a
 * 	linearized representation of the AST which enables code optimization.
 *	Indeed the list of triples is stored as a doubly linked list, so the
 *	the instruction of the program are easily rearrangeable.
 *
 * @author <mirko.bez@studenti.unipd.it>
 * @author <stefano.munari.1@studenti.unipd.it>
 */
#ifndef __TAC_H__
#define __TAC_H__
#include "tac_list.h"
#include "../ast/seq_tree.h"
#include "../symbol_table/symbol_table.h"
#include "../types/factype.h"
#include "../types/factype_tac.h"
#include <stdbool.h>


/**
 * A value of a TAC entry, i.e. one argument of an indirect triple entry
 */
typedef struct tac_value{
	/** boolean constant */
	bool boolean;
	/** fract constant */
	fract_t * fract;
	/** symbol table entry for a variable */
	symbol_table_entry * address;
	/** pointer to another TAC entry */
	struct tac_entry * instruction;
} tac_value;

/**
 * A TAC entry represented as a triple (operator, argument0, argument1)
 */
typedef struct tac_entry{
	/** the three address code operation (+, -, ...) */
	tac_op op;
	/** the first argument */
	tac_value * arg0;
	/** the optional second argument */
	tac_value * arg1;
} tac_entry;

/** Generates a new TAC representation of the given AST
 * @param input a pointer to the head the AST
 * @return a doubly-linked list of indirect triples
 */
struct tac_list * generate_tac(seq_node * input);

/** Frees the list of indirected triples (TAC) given as input
 * @param tlist a pointer to the list of indirected triples (TAC)
 */
void free_tac(struct tac_list * tlist);

#endif /*__TAC_H__*/
