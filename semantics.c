#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "semantics.h"

extern int lineno;

// Tipos: INT_TYPE, REAL_TYPE, CHAR_TYPE

int get_result_type(int type_1, int type_2, int op_type){ /* Tipo de resultado */
	switch(op_type){
		case NONE: /* Compatibilidad, '1': compatible */
			// type_1 INT
			if(type_1 == INT_TYPE){
				// type_2 INT or CHAR
				if(type_2 == INT_TYPE || type_2 == CHAR_TYPE){
					return 1;
				}
				else{
					type_error(type_1, type_2, op_type);
				}
			}
			// type_1 REAL
			else if(type_1 == REAL_TYPE){
				// type_2 INT, REAL or CHAR
				if(type_2 == INT_TYPE || type_2 == REAL_TYPE || type_2 == CHAR_TYPE){
					return 1;
				}
				else{
					type_error(type_1, type_2, op_type);
				}
			}
			// type_1 CHAR
			else if(type_1 == CHAR_TYPE){
				// type_2 INT or CHAR
				if(type_2 == INT_TYPE || type_2 == CHAR_TYPE){
					return 1;
				}
				else{
					type_error(type_1, type_2, op_type);
				}
			}
			break;
		/* ---------------------------------------------------------- */
		case ARITHM_OP: /* Operador aritmetico */
			// type_1 INT
			if(type_1 == INT_TYPE){
				// type_2 INT or CHAR
				if(type_2 == INT_TYPE || type_2 == CHAR_TYPE){
					return INT_TYPE;
				}
				// type_2 REAL
				else if(type_2 == REAL_TYPE){
					return REAL_TYPE;
				}
				else{
					type_error(type_1, type_2, op_type);
				}
			}
			// type_1 REAL
			else if(type_1 == REAL_TYPE){
				// type_2 INT, REAL or CHAR
				if(type_2 == INT_TYPE || type_2 == REAL_TYPE || type_2 == CHAR_TYPE){
					return REAL_TYPE;
				}
				else{
					type_error(type_1, type_2, op_type);
				}
			}
			// type_1 CHAR
			else if(type_1 == CHAR_TYPE){
				// type_2 INT or CHAR
				if(type_2 == INT_TYPE || type_2 == CHAR_TYPE){
					return CHAR_TYPE;
				}
				// type_2 REAL
				else if(type_2 == REAL_TYPE){
					return REAL_TYPE;
				}
				else{
					type_error(type_1, type_2, op_type);
				}
			}
			else{
				type_error(type_1, type_2, op_type);
			}
			break;
		/* ---------------------------------------------------------- */
		case INCR_OP: /* Incremento */
			// type INT
			if(type_1 == INT_TYPE){
				return INT_TYPE;
			}
			// type REAL
			else if(type_1 == REAL_TYPE){
				return REAL_TYPE;
			}
			// type CHAR
			else if(type_1 == CHAR_TYPE){
				return CHAR_TYPE;
			}
			else{
				type_error(type_1, type_2, op_type);
			}
			break;
		/* ---------------------------------------------------------- */
		case BOOL_OP: /* Operador booleano */
			// type_1 INT
			if(type_1 == INT_TYPE){
				// type_2 INT or CHAR
				if(type_2 == INT_TYPE || type_2 == CHAR_TYPE){
					return INT_TYPE;
				}
				else{
					type_error(type_1, type_2, op_type);
				}
			}
			// type_1 CHAR
			else if(type_1 == CHAR_TYPE){
				// type_2 INT or CHAR
				if(type_2 == INT_TYPE || type_2 == CHAR_TYPE){
					return CHAR_TYPE;
				}
				else{
					type_error(type_1, type_2, op_type);
				}
			}
			else{
				type_error(type_1, type_2, op_type);
			}
			break;
		/* ---------------------------------------------------------- */
		case NOT_OP: /* Negacion */
			// type INT
			if(type_1 == INT_TYPE){
				return INT_TYPE;
			}
			// type CHAR
			else if(type_1 == CHAR_TYPE){
				return INT_TYPE;
			}
			else{
				type_error(type_1, type_2, op_type);
			}
			break;
		/* ---------------------------------------------------------- */
		case REL_OP: /* Operador relacional */
			// type_1 INT
			if(type_1 == INT_TYPE){
				// type_2 INT, REAL or CHAR
				if(type_2 == INT_TYPE || type_2 == REAL_TYPE || type_2 == CHAR_TYPE){
					return INT_TYPE;
				}
				else{
					type_error(type_1, type_2, op_type);
				}
			}
			else if(type_1 == REAL_TYPE){
				// type_2 INT, REAL or CHAR
				if(type_2 == INT_TYPE || type_2 == REAL_TYPE || type_2 == CHAR_TYPE){
					return INT_TYPE;
				}
				else{
					type_error(type_1, type_2, op_type);
				}
			}
			// type_1 CHAR
			else if(type_1 == CHAR_TYPE){
				// type_2 INT, REAL or CHAR
				if(type_2 == INT_TYPE || type_2 == REAL_TYPE || type_2 == CHAR_TYPE){
					return INT_TYPE;
				}
				else{
					type_error(type_1, type_2, op_type);
				}
			}
			else{
				type_error(type_1, type_2, op_type);
			}
			break;
		/* ---------------------------------------------------------- */
		case EQU_OP: /* Operador de equivalencia */
			// type_1 INT
			if(type_1 == INT_TYPE){
				// type_2 INT or CHAR
				if(type_2 == INT_TYPE || type_2 == CHAR_TYPE){
					return INT_TYPE;
				}
				else{
					type_error(type_1, type_2, op_type);
				}
			}
			else if(type_1 == REAL_TYPE){
				// type_2 REAL
				if(type_2 == REAL_TYPE){
					return INT_TYPE;
				}
				else{
					type_error(type_1, type_2, op_type);
				}
			}
			// type_1 CHAR
			else if(type_1 == CHAR_TYPE){
				// type_2 INT or CHAR
				if(type_2 == INT_TYPE || type_2 == CHAR_TYPE){
					return INT_TYPE;
				}
				else{
					type_error(type_1, type_2, op_type);
				}
			}
			else{
				type_error(type_1, type_2, op_type);
			}
			break;
		/* ---------------------------------------------------------- */
		default: /* Operador desconocido */
			fprintf(stderr, "Operador no reconocido en la linea %d!\n", lineno);
			exit(1);
	}
}

void type_error(int type_1, int type_2, int op_type){ /* print type error */
	fprintf(stderr, "Error Semantico entre ");
	/* type_1 */
	if      (type_1 == INT_TYPE)           fprintf(stderr,"%s","int");
	else if (type_1 == REAL_TYPE)          fprintf(stderr,"%s","real");
	else if (type_1 == CHAR_TYPE)          fprintf(stderr,"%s","char");
	else                                   fprintf(stderr,"%s","other");
	
	fprintf(stderr, " y ");	
	
	/* type_2 */
	if      (type_2 == INT_TYPE)           fprintf(stderr,"%s","int");
	else if (type_2 == REAL_TYPE)          fprintf(stderr,"%s","real");
	else if (type_2 == CHAR_TYPE)          fprintf(stderr,"%s","char");
	else                                   fprintf(stderr,"%s","other");
	
	/* operador */
	fprintf(stderr," usando el operador ");
	switch(op_type){
		case NONE:
			fprintf(stderr,"%s","NONE");
			break;
		case ARITHM_OP:
			fprintf(stderr,"%s","ARITHM_OP");
			break;
		case INCR_OP:
			fprintf(stderr,"%s","INCR_OP");
			break;
		case BOOL_OP:
			fprintf(stderr,"%s","BOOL_OP");
			break;
		case NOT_OP:
			fprintf(stderr,"%s","NOT_OP");
			break;
		case REL_OP:
			fprintf(stderr,"%s","REL_OP");
			break;
		case EQU_OP:
			fprintf(stderr,"%s","EQU_OP");
			break;
		default: 
			fprintf(stderr, "Operador no reconocido en la linea %d!\n", lineno);
			exit(1);	
	}
	
	/* linea */
	fprintf(stderr, " en la linea %d\n", lineno);
	
	exit(1);
}
