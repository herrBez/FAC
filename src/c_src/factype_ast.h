/**
 * @file
 * @brief Types for Abstract Syntax Tree - reduced version of extended types
 *	defined in FACTYPE
 *
 *  We need only simple/uniform types  in the AST, i.e. not divided
 *	in different precedence classes. Indeed, the structure of the tree enforces
 *	(by design) precedence between its elements (nodes and leaves)
 * @see factype.h
 * @author <mirko.bez@studenti.unipd.it>
 * @author <stefano.munari.1@studenti.unipd.it>
 */
#ifndef _FACTYPE_AST_H_
#define _FACTYPE_AST_H_
#include "factype.h"
/* Arithmetic OPerator Type */
typedef enum
{
	AST_SUM,
	AST_DIFF,
	AST_MULT,
	AST_DIV
} aop_t;
/* Boolean OPerator Type */
typedef enum
{
	AST_AND,
	AST_OR,
	AST_IMPLY,
	AST_IFF,
	AST_XOR
} bop2_t;
/* RELational OPerator Type */
typedef enum
{
	AST_LT,
	AST_LEQ,
	AST_GEQ,
	AST_GT,
	AST_EQ,
	AST_NEQ
} relop_t;

#define AST_AOP 285
#define AST_BOP2 286
#define AST_RELOP 287

#endif /*_FACTYPE_AST_H_*/
