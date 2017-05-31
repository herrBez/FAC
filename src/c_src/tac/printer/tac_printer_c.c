#include "tac_printer.h"
#include "../tac_list.h"
#include "../../symbol_table/symbol_table.h"
#include <stdio.h>

static
void print_tac(tac_list *, char *);

const struct _tprinter_vtable C[] = { { print_tac } };

static
void dump_symbol_table(FILE *);

static
void print_tac_entry(FILE *, tac_node *);

static
char * get_operator(tac_op operator);

static
char * getValue(tac_value *, char *);

static
char * getBooleanValue(tac_value *);

char buffer[256];

void print_tac(tac_list * tlist, char * path){
	char * header_name =
		malloc(sizeof(char) * (strlen(path) + 1) + sizeof(char) * (strlen("fvariables.h")));
	char * main_name =
		malloc(sizeof(char) * (strlen(path) + 1) + sizeof(char) * (strlen("main.c")));

	sprintf(main_name, "%s%s", path, "main.c");
	sprintf(header_name, "%s%s", path, "fvariables.h");

	FILE * c_main = fopen(main_name, "a");
	FILE * c_header = fopen(header_name, "a");

	dump_symbol_table(c_header);

	fprintf(c_main, "#include \"fvariables.h\"\n");
	fprintf(c_main, "#include <stdio.h>\n");
	fprintf(c_main, "int MCD(int u, int v) {\n");
	fprintf(c_main, "\treturn (v != 0)?MCD(v, u %c v):u;\n}\n", 37);

	fprintf(c_main, "int main(void){\n");

	int i=0;
	tac_node * iterator = tlist->first;
	while(iterator != NULL){
		print_tac_entry(c_main, iterator);
		++i;
		iterator = iterator->next;
	}
	fprintf(c_main, "return 0;\n");
	fprintf(c_main, "}\n");

	fclose(c_header);
	fclose(c_main);
}

void dump_symbol_table(FILE * c_header){
	symbol_table_entry * iterator;
    symbol_table_entry * table = getTable();

    for(iterator = table; iterator != NULL; iterator = iterator->hh.next){
		switch(iterator->type) {
			case 0:
				fprintf(c_header, "int %s;\n", iterator->id);
				break;
			case 1:
				fprintf(c_header, "int %snum;\n", iterator->id);
				fprintf(c_header, "int %sden;\n", iterator->id);
				break;
			default: yyerror("TAC_Printer - wrong symbol table type"); break;
		}
    }
}

void print_tac_entry(FILE * c_main, tac_node * node){
	tac_entry * entry = node->value;
	if(entry == NULL)
		return;
	switch(entry->op){
		case TAC_ASSIGNMENT:
		{
			char * id = entry->arg0->address->id;
			switch(getType(id)){
				case BOOL_T:
				{
					char * boolean_value = getBooleanValue(entry->arg1);
					fprintf(c_main, "%s = %s;\n", id, boolean_value);
					break;
				}
				case FRACT_T:
				{
					char * fract_val_num = getValue(entry->arg1, "num");
					char * fract_val_den = getValue(entry->arg1, "den");
					fprintf(c_main, "%snum = %s;\n", id, fract_val_num);
					fprintf(c_main, "%sden = %s;\n", id, fract_val_den);
					break;
				}
			}
			break;
		}
		case TAC_PLUS:
		case TAC_MINUS:
		{
			char * numA = getValue(entry->arg0, "num");
			char * denA = getValue(entry->arg0, "den");
			fprintf(c_main, "t%pnum = %s %s;\n", entry, entry->op==TAC_SUM?"+":"-", numA);
			fprintf(c_main, "t%pden = %s %s;\n", entry, entry->op==TAC_SUM?"+":"-", denA);
			break;
		}
		case TAC_SUM:
		case TAC_DIFF:
		{

			char * numA = getValue(entry->arg0, "num");
			char * denA = getValue(entry->arg0, "den");
			char * numB = getValue(entry->arg1, "num");
			char * denB = getValue(entry->arg1, "den");

			fprintf(c_main, "/* SUM */\n");
			fprintf(c_main, "h0 = %s * %s;\n", numA, denB);
			fprintf(c_main, "h1 = %s * %s;\n", numB, denA);
			fprintf(c_main, "h2 = h0 %s h1;\n", entry->op==TAC_SUM?"+":"-");
			fprintf(c_main, "h3 = %s * %s;\n", denA, denB);
			fprintf(c_main, "h4 = MCD(h2, h3);\n");
			fprintf(c_main, "t%pnum = h2 / h5;\n", entry);
			fprintf(c_main, "t%pden = h3 / h5;\n", entry);

			free(denA);
			free(denB);
			free(numA);
			free(numB);

			break;
		}
		case TAC_MULT:
		case TAC_DIV:
		{
			char * numA = getValue(entry->arg0, "num");
			char * denA = getValue(entry->arg0, "den");

			char * numB = getValue(entry->arg1, "num");
			char * denB = getValue(entry->arg1, "den");
			if(entry->op == TAC_DIV){ //Take the inverse for division
				char * tmp = numB;
				numB = denB;
				denB = tmp;
			}

			fprintf(c_main, "/* START %s */\n", entry->op == TAC_MULT?"MULT":"DIV");
			fprintf(c_main, "h0 = %s * %s;\n", numA, numB);
			fprintf(c_main, "h1 = %s * %s;\n", denA, denB);
			fprintf(c_main, "h2 = MCD(h0, h1);\n");
			fprintf(c_main, "t%pnum = h0 / h2;\n", entry);
			fprintf(c_main, "t%pden = h1 / h3;\n", entry);
			fprintf(c_main, "/* END %s */\n", entry->op == TAC_MULT?"MULT":"DIV");
			free(numA);
			free(numB);
			free(denA);
			free(denB);
			break;

		}
		case TAC_PRINT:
		{
			char * id = entry->arg0->address->id;
			switch(getType(id)){
				case BOOL_T: fprintf(c_main, "printf(\"%cd\\n\", %s);\n", 37, id); break;
				case FRACT_T: fprintf(c_main, "printf(\"[%%d|%%d]\\n\", %snum, %sden);\n", id, id); break;
			}
			break;
		}
		case TAC_NOT:
		{
			char * boolean_value = getBooleanValue(entry->arg0);
			fprintf(c_main, "t%p = !%s;\n", entry, boolean_value);
			free(boolean_value);
			break;
		}
		case TAC_AND:
		case TAC_OR:
		{
			char * bool1 = getBooleanValue(entry->arg0);
			char * bool2 = getBooleanValue(entry->arg1);
			fprintf(c_main, "t%p = %s %s %s;\n", entry, bool1,  get_operator(entry->op), bool2);
			break;
		}
		case TAC_XOR:
		case TAC_IFF:
		{
			char * bool1 = getBooleanValue(entry->arg0);
			char * bool2 = getBooleanValue(entry->arg1);
			fprintf(c_main, "t%p = %s %s %s; \n", entry, bool1, get_operator(entry->op), bool2);
			break;
		}
		case TAC_EQ:
		case TAC_NEQ:
		{

			char * numA = getValue(entry->arg0, "num");
			char * denA = getValue(entry->arg0, "den");
			char * numB = getValue(entry->arg1, "num");
			char * denB = getValue(entry->arg1, "den");
			fprintf(c_main, "h0 = %s %s %s;\n", numA, get_operator(entry->op), numB);
			fprintf(c_main, "h1 = %s %s %s;\n", denA, get_operator(entry->op), denB);
			fprintf(c_main, "t%p = h0 && h1;\n", entry);
			break;
		}
		case TAC_LT:
		case TAC_GEQ:
		case TAC_LEQ:
		case TAC_GT:
		{
			char * numA = getValue(entry->arg0, "num");
			char * denA = getValue(entry->arg0, "den");
			char * numB = getValue(entry->arg1, "num");
			char * denB = getValue(entry->arg1, "den");
			fprintf(c_main, "h0 = %s * %s;\n", numA, denB);
			fprintf(c_main, "h1 = %s * %s;\n", denA, numB);
			fprintf(c_main, "t%p = h0 %s h1;\n", entry, get_operator(entry->op));
			break;
		}
		case TAC_LABEL:
		{
			fprintf(c_main, "L%p:\n", entry);
			break;
		}
		case TAC_GOTO:
		{
			if(entry->arg1 != NULL){ //Conditional goto
				fprintf(c_main, "if (t%p) goto L%p;\n", entry->arg0->instruction,
				entry->arg1->instruction);
			} else {
				fprintf(c_main, "goto L%p;\n", entry->arg0->instruction);
			}
			break;
		}
		default: fprintf(c_main, "Not yet implemented\n");
	}
}

static
char * get_operator(tac_op operator){
	switch(operator){
		case TAC_PLUS:
		case TAC_SUM: return "+"; break;
		case TAC_MINUS:
		case TAC_DIFF: return "-"; break;
		case TAC_MULT: return "*"; break;
		case TAC_DIV: return "/"; break;
		case TAC_NOT: return "!"; break;
		case TAC_AND: return "&&"; break;
		case TAC_OR: return "||"; break;
		case TAC_LT: return "<"; break;
		case TAC_GT: return ">"; break;
		case TAC_LEQ: return "<="; break;
		case TAC_GEQ: return ">="; break;
		case TAC_NEQ: return "!="; break;
		case TAC_EQ: return "=="; break;
		case TAC_IFF: return "=="; break;
		case TAC_XOR: return "!="; break;
		case TAC_PRINT: return "printf"; break;
		case TAC_ASSIGNMENT: return "="; break;
		case TAC_GOTO: return "goto"; break;
		case TAC_LABEL: return "label"; break;
		/* the others are all unrecognized operators */
		default: yyerror("TAC_Printer - operator not recognized"); break;
	}
	return "";
}

static
char * getBooleanValue(tac_value * tvalue){
	if(tvalue->address != NULL){
		char * buffer = malloc(sizeof(char) * (strlen(tvalue->address->id) + 1));
		sprintf(buffer, "%s", tvalue->address->id);
		return buffer;
	} else if(tvalue->instruction != NULL){
		char * buffer = malloc(sizeof(char) * 128);
		sprintf(buffer, "t%p", tvalue->instruction);
		return buffer;
	}
	 else {
		char * buffer = malloc(sizeof(char) * 2);
		sprintf(buffer, "%d", tvalue->boolean);
		return buffer;
	}
}

static
char * getValue(tac_value * tvalue, char * num_or_den) {
	if(tvalue == NULL){
		return NULL;
	}
	if(tvalue->address != NULL){
		char * buffer =
			malloc(sizeof(char) * (strlen(tvalue->address->id)
			+ strlen(num_or_den) + 1));
		sprintf(buffer, "%s%s", tvalue->address->id, num_or_den);
		return buffer;
	}
	else if(tvalue->fract != NULL){
		char * buffer = malloc(sizeof(char) * 128);
		if(strcmp(num_or_den, "num") == 0)
			sprintf(buffer, "%d", tvalue->fract->num);
		else
			sprintf(buffer, "%d", tvalue->fract->den);
		return buffer;
	} else if(tvalue->instruction != NULL){
		char * buffer = malloc(sizeof(char) * 128);
		sprintf(buffer, "t%p%s", tvalue->instruction, num_or_den);
		return buffer;
	} else {
		yyerror("Error in fetching numerator %s %d \n", __FILE__, __LINE__);
		return "";
	}
}