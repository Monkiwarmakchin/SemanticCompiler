%{
	#include "semantics.c"
	#include "symtab.c"
	#include "ast.h"
	#include "ast.c"
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	extern FILE *yyin;
	extern FILE *yyout;
	extern int lineno;
	extern int yylex();
	void yyerror();
	
	// Para declaraciones
	void add_to_names(list_t *entry);
	list_t **names;
	int nc = 0;
	
	// Para la inicializacion de arrays
	void add_to_vals(Value val);
	Value *vals;
	int vc = 0;
	
	// for else ifs
	void add_elseif(AST_Node *elsif);
	AST_Node **elsifs;
	int elseif_count = 0;
	
	// Para funciones
	AST_Node_Func_Decl *temp_function;
%}

/* YYSTYPE union */
%union{
	// Valores
	Value val;   
	
	// strucs
	list_t* symtab_item;
	AST_Node* node;
	
	// Para declaraciones
	int data_type;
	int const_type;
	
	// Para arrays
	int array_size;
	
	// Para parametros
	Param par;
}

/* definicion de tokens */
%token<val> CHAR INT FLOAT DOUBLE IF ELSE WHILE FOR CONTINUE BREAK VOID RETURN
%token<val> ADDOP MULOP DIVOP INCR OROP ANDOP NOTOP EQUOP RELOP
%token<val> LPAREN RPAREN LBRACK RBRACK LBRACE RBRACE SEMI COMMA ASSIGN REFER
%token <symtab_item> ID
%token <val> 	 ICONST
%token <val>  	 FCONST
%token <val> 	 CCONST
%token <val>     STRING

/* precedencias y asosiasiones */
%left COMMA
%right ASSIGN
%left OROP
%left ANDOP
%left EQUOP
%left RELOP
%left ADDOP
%left MULOP DIVOP
%right NOTOP INCR REFER MINUS
%left LPAREN RPAREN LBRACK RBRACK

/* Definicion de reglas no terminales */
%type <node> program
%type <node> declarations declaration
%type <data_type> type
%type <symtab_item> variable
%type <array_size> array
%type <symtab_item> init var_init array_init
%type <node> constant
%type <node> expression var_ref
%type <node> statement assigment
%type <node> statements tail
%type <node> if_statement else_if optional_else
%type <node> for_statement while_statement
%type <node> functions_optional functions function
%type <node> parameters_optional parameters
%type <par>  parameter
%type <node> return_type
%type <node> function_call call_params call_param

%start program

%%

/* Gramatica */

program: 
	declarations { ast_traversal($1); }
	statements   { ast_traversal($3); }
	RETURN SEMI functions_optional { ast_traversal($7); }
;

/* declarations */
declarations: 
	declarations declaration
	{
		AST_Node_Declarations *temp = (AST_Node_Declarations*) $1;
		$$ = new_declarations_node(temp->declarations, temp->declaration_count, $2);
	}
	| declaration
	{
		$$ = new_declarations_node(NULL, 0, $1);
	}
;

declaration: type { declare = 1; } names { declare = 0; } SEMI
	{
		int i;
		$$ = new_ast_decl_node($1, names, nc);
		nc = 0;
		
		AST_Node_Decl *temp = (AST_Node_Decl*) $$;
		
		// Tipo de declaracion de los nombres
		for(i=0; i < temp->names_count; i++){
			// variable
			if(temp->names[i]->st_type == UNDEF){
				set_type(temp->names[i]->st_name, temp->data_type, UNDEF);
			}
			// apuntador
			else if(temp->names[i]->st_type == POINTER_TYPE){
				set_type(temp->names[i]->st_name, POINTER_TYPE, temp->data_type);
			}
			// array
			else if(temp->names[i]->st_type == ARRAY_TYPE){
				set_type(temp->names[i]->st_name, ARRAY_TYPE, temp->data_type);
			}
		}
	}
;

type: INT  		{ $$ = INT_TYPE;   }
	| CHAR 		{ $$ = CHAR_TYPE;  }
	| FLOAT 	{ $$ = REAL_TYPE;  }
	| DOUBLE 	{ $$ = REAL_TYPE;  }
	| VOID 		{ $$ = VOID_TYPE;  }
;

names: names COMMA variable
	{
		add_to_names($3);
	}
	| names COMMA init
	{
		add_to_names($3);
	}
	| variable
	{
		add_to_names($1);
	}
	| init
	{ 
		add_to_names($1);
	}
;

variable: ID { $$ = $1; }
	| pointer ID
	{
		$2->st_type = POINTER_TYPE;
		$$ = $2;
	}
	| ID array
	{
		$1->st_type = ARRAY_TYPE;
		$1->array_size = $2;
		$$ = $1;
	}
;

pointer: MULOP ; /* Solo apuntadores simples */

array: /* Solo arrays de una dimension */
	LBRACK expression RBRACK /* Solo puede ser usado en expresiones, no en declaraciones */
	{
		// Si el array se estaba declarando
		if(declare == 1){
			fprintf(stderr, "La declaracion del array en la linea %d contiene una expresion!\n", lineno);
			exit(1);
		}
	}
	| LBRACK ICONST RBRACK
	{
		// Seteamos array_size para las declaraciones
		$$ = $2.ival;
	}
;

init:
	var_init { $$ = $1; }
	| array_init { $$ = $1; }
; 

var_init : ID ASSIGN constant
{ 
	AST_Node_Const *temp = (AST_Node_Const*) $$;
	$1->val = temp->val;
	$1->st_type = temp->const_type;
	$$ = $1;
}
;

array_init: ID array ASSIGN LBRACE values RBRACE
{
	if($1->array_size != vc){
	    fprintf(stderr, "El array inicializado en la linea %d no tiene el numero asigndao de valores\n", lineno);
		exit(1);
	}
	$1->vals = vals;
	$1->array_size = $2;
	$$ = $1;
	vc = 0;
}
;

values: values COMMA constant 
	{
		AST_Node_Const *temp = (AST_Node_Const*) $3;
		add_to_vals(temp->val);
	}
	| constant
	{
		AST_Node_Const *temp = (AST_Node_Const*) $1;
		add_to_vals(temp->val);
	}
;

/* statements */
statements:
	statements statement
	{
		AST_Node_Statements *temp = (AST_Node_Statements*) $1;
		$$ = new_statements_node(temp->statements, temp->statement_count, $2);
	}
	| statement
	{
		$$ = new_statements_node(NULL, 0, $1);
	}
;

statement:
	if_statement
	{ 
		$$ = $1; /* solo pasamos la informacion */
	}
	| for_statement
	{ 
		$$ = $1; /* solo pasamos la informacion */
	}
	| while_statement
	{
		$$ = $1; /* solo pasamos la informacion */
	}
	| assigment SEMI
	{
		$$ = $1; /* solo pasamos la informacion */
	}
	| CONTINUE SEMI
	{ 
		$$ = new_ast_simple_node(0);
	}
	| BREAK SEMI
	{ 
		$$ = new_ast_simple_node(1);
	}
	| function_call SEMI
	{ 
		$$ = $1; /* solo pasamos la informacion */
	}
	| ID INCR SEMI
	{
		/* Incremento */
		if($2.ival == INC){
			$$ = new_ast_incr_node($1, 0, 0);
		}
		else{
			$$ = new_ast_incr_node($1, 1, 0);
		}
	}
	| INCR ID SEMI
	{
		/* Incremento */
		if($1.ival == INC){
			$$ = new_ast_incr_node($2, 0, 1);
		}
		else{
			$$ = new_ast_incr_node($2, 1, 1);
		}
	}
;

if_statement:
	IF LPAREN expression RPAREN tail else_if optional_else
	{
		$$ = new_ast_if_node($3, $5, elsifs, elseif_count, $7);
		elseif_count = 0;
		elsifs = NULL;
	}
	| IF LPAREN expression RPAREN tail optional_else
	{
		$$ = new_ast_if_node($3, $5, NULL, 0, $6);
	}
;

else_if:
	else_if ELSE IF LPAREN expression RPAREN tail
	{
		AST_Node *temp = new_ast_elsif_node($5, $7);
		add_elseif(temp);
	}
	| ELSE IF LPAREN expression RPAREN tail
	{
		AST_Node *temp = new_ast_elsif_node($4, $6);
		add_elseif(temp);
	}
;

optional_else:
	ELSE tail
	{
		/* Si "else" existe */
		$$ = $2;
	}
	| /* empty */
	{
		/* Si no hay "else" */
		$$ = NULL;
	}
;

for_statement: FOR LPAREN assigment SEMI expression SEMI ID INCR RPAREN tail
{
	/* Creamos un nodo de incremento */
	AST_Node *incr_node;
	if($8.ival == INC){ /* Incremento */
		incr_node = new_ast_incr_node($7, 0, 0);
	}
	else{
		incr_node = new_ast_incr_node($7, 1, 0);
	}

	$$ = new_ast_for_node($3, $5, incr_node, $10);
	set_loop_counter($$);
}
;

while_statement: WHILE LPAREN expression RPAREN tail
{
	$$ = new_ast_while_node($3, $5);
}
;

tail: LBRACE statements RBRACE
{ 
	$$ = $2; /* solo pasamos la informacion */
}
;

expression:
    expression ADDOP expression
	{ 
	    $$ = new_ast_arithm_node($2.ival, $1, $3);
	}
	| expression MULOP expression
	{
	    $$ = new_ast_arithm_node(MUL, $1, $3);
	}
	| expression DIVOP expression
	{
		$$ = new_ast_arithm_node(DIV, $1, $3);
	}
	| ID INCR
	{
		/* Incremento */
		if($2.ival == INC){
			$$ = new_ast_incr_node($1, 0, 0);
		}
		else{
			$$ = new_ast_incr_node($1, 1, 0);
		}	
	}
	| INCR ID
	{
		/* Incremento */
		if($1.ival == INC){
			$$ = new_ast_incr_node($2, 0, 1);
		}
		else{
			$$ = new_ast_incr_node($2, 1, 1);
		}
	}
	| expression OROP expression
	{
		$$ = new_ast_bool_node(OR, $1, $3);
	}
	| expression ANDOP expression
	{
		$$ = new_ast_bool_node(AND, $1, $3);
	}
	| NOTOP expression
	{
	    $$ = new_ast_bool_node(NOT, $2, NULL);
	}
	| expression EQUOP expression
	{
		$$ = new_ast_equ_node($2.ival, $1, $3);
	}
	| expression RELOP expression
	{
		$$ = new_ast_rel_node($2.ival, $1, $3);
	}
	| LPAREN expression RPAREN
	{
		$$ = $2; /* solo pasamos la informacion */
	}
	| var_ref
	{ 
		$$ = $1; /* solo pasamos la informacion*/
	}
	| constant
	{
		$$ = $1; /* Sin signo */
	}
	| ADDOP constant %prec MINUS
	{
		/* Si ponemos "+" como signo */
		if($1.ival == ADD){
			fprintf(stderr, "Error: Se puso + como signo de una expresion en la linea %d\n", lineno);
			exit(1);
		}
		else{
			AST_Node_Const *temp = (AST_Node_Const*) $2;
		
			/* Invertimos el valor dependiendo del tipo de la constante */
			switch(temp->const_type){
				case INT_TYPE:
					temp->val.ival *= -1;
					break;
				case REAL_TYPE:
					temp->val.fval *= -1;
					break;
				case CHAR_TYPE:
					/* Si hay un signo antes de un char*/
					fprintf(stderr, "Error: Se puso un signo antes de un char contante en la linea %d!\n", lineno);
					exit(1);
					break;
			}
			
			$$ = (AST_Node*) temp;
		}
	}
	| function_call
	{
		$$ = $1; /* solo pasamos la informacion */
	}
;

constant:
	ICONST   { $$ = new_ast_const_node(INT_TYPE, $1);  }
	| FCONST { $$ = new_ast_const_node(REAL_TYPE, $1); }
	| CCONST { $$ = new_ast_const_node(CHAR_TYPE, $1); }
;

assigment: var_ref ASSIGN expression
{
	AST_Node_Ref *temp = (AST_Node_Ref*) $1;
	$$ = new_ast_assign_node(temp->entry, temp->ref, $3);
	
	/* Obteniendo tipos de la asignacion */
	int type1 = get_type(temp->entry->st_name);
	int type2 = expression_data_type($3);
	
	/* La ultima funcion nos dara informacion de las revisitas */
	
	/* Si revisita => Agregar una entrada del tipo asignaciones a revisitas */
	if(cont_revisit == 1){	
		/* Buscar si la entrada existe */
		revisit_queue *q = search_queue(temp->entry->st_name);
		if(q == NULL){
			add_to_queue(temp->entry, temp->entry->st_name, ASSIGN_CHECK);
			q = search_queue(temp->entry->st_name);	
		}
		
		/* Datos de structs */
		if(q->num_of_assigns == 0){ /* first node */
			q->nodes = (void**) malloc(sizeof(void*));
		}
		else{ /* Caso general */
			q->nodes = (void**) realloc(q->nodes, (q->num_of_assigns + 1) * sizeof(void*));
		}
		
		/* Asignaciones */
		q->nodes[q->num_of_assigns] = (void*) $3;
		
		/* Incrementamos el numero de asignaciones */
		q->num_of_assigns++;
		
		/* Reseteamos la bandera de revisitas */
		cont_revisit = 0;
		
		printf("Asignando revisita para %s en la linea %d\n", temp->entry->st_name, lineno);
	}
	else{ /* Si no revisita */
		/* Checamos la compatibilidad de la asignacion */
		get_result_type(
			type1,       /* Tipo de la variable */
			type2,       /* Tipo de la expresion */
			NONE  /* Operador de checar compatibilidad */
		);
	}
}
;

var_ref: variable
	{
		$$ = new_ast_ref_node($1, 0); /* no reference */
	}
	| REFER variable
	{
		$$ = new_ast_ref_node($2, 1); /* reference */
	}
;

function_call: ID LPAREN call_params RPAREN
{
	AST_Node_Call_Params *temp = (AST_Node_Call_Params*) $3;
	$$ = new_ast_func_call_node($1, temp->params, temp->num_of_pars);	
	
	/* Informacion para la entrada a revisitas (si existe) */
	revisit_queue *q = search_queue($1->st_name);
	if(q != NULL){
		/* Datos de stucts */
		if(q->num_of_calls == 0){ /* first call */
			q->par_types = (int**) malloc(sizeof(int*));
			q->num_of_pars = (int*) malloc(sizeof(int));
		}
		else{ /* Caso general*/
			q->par_types = (int**) realloc(q->par_types, (q->num_of_calls + 1) * sizeof(int*));
			q->num_of_pars = (int*) realloc(q->num_of_pars, (q->num_of_calls + 1) * sizeof(int));
		}
		
		/* Llamadas a funcion */
		q->num_of_pars[q->num_of_calls] = temp->num_of_pars;
		q->par_types[q->num_of_calls] = (int*) malloc(temp->num_of_pars * sizeof(int));
		/* get the types of the parameters */
		int i;
		for(i = 0; i < temp->num_of_pars; i++){
			/* Obtener el tipo del parametro/expresion */
			q->par_types[q->num_of_calls][i] = expression_data_type(temp->params[i]);
		}
		
		/* Incrementamos el numero de llamadas */
		q->num_of_calls++;
	}
	else{
		/* Funcion declarada antes de ser llamada */
		if($1->st_type == FUNCTION_TYPE){
			/* Revisando numero de parametros */
			if($1->num_of_pars != temp->num_of_pars){
				fprintf(stderr, "La llamada a funcion de %s, en la linea %d, tiene un numero erroneo de parametros!\n", $1->st_name, lineno);
				exit(1);
			}
			/* Revisando si los parametros son compatibles */
			int i;
			for(i = 0; i < temp->num_of_pars; i++){
				/* Tipo de parametros en la declaracion de la funcion */
				int type_1 = expression_data_type(temp->params[i]);
				
				/* Tipo de parametros en la llamada a funcion */
				int type_2 = $1->parameters[i].par_type;
				
				/* Revisando compatibilidad */
				get_result_type(type_1, type_2, NONE);
			}
		}
	}
}
;

call_params: 
	call_param
	{
		$$ = $1;
	}
	| STRING
	{
		AST_Node *temp = new_ast_const_node(STR_TYPE, $1);
		$$ = new_ast_call_params_node(NULL, 0, temp);
	}
	| /* empty */
	{
		AST_Node_Call_Params *temp = malloc (sizeof (AST_Node_Call_Params));
		temp->type = CALL_PARAMS;
		temp->params = NULL;
		temp->num_of_pars = 0;
		$$ = (AST_Node*)temp;
	}
;

call_param: 
	call_param COMMA expression
	{
		AST_Node_Call_Params *temp = (AST_Node_Call_Params*) $1;
		$$ = new_ast_call_params_node(temp->params, temp->num_of_pars, $3);
	}
	| expression
	{
		$$ = new_ast_call_params_node(NULL, 0, $1);
	}	
;

/* functions */
functions_optional: 
	functions
	{
		$$ = $1;
	}
	| /* empty */
	{
		$$ = NULL;
	}
;

functions: 
	functions function
	{
		AST_Node_Func_Declarations *temp = (AST_Node_Func_Declarations*) $1;
		$$ = new_func_declarations_node(temp->func_declarations, temp->func_declaration_count, $2);
	}
	| function
	{
		$$ = new_func_declarations_node(NULL, 0, $1);
	}
;

function: { incr_scope(); } function_head function_tail
{ 
	/* Revisitar */
	revisit(temp_function->entry->st_name);
	
	hide_scope();
	$$ = (AST_Node *) temp_function;
} 
;

function_head: { function_decl = 1; } return_type ID LPAREN
	{ 
		function_decl = 0;
		
		AST_Node_Ret_Type *temp = (AST_Node_Ret_Type *) $2;
		temp_function = (AST_Node_Func_Decl *) new_ast_func_decl_node(temp->ret_type, temp->pointer, $3);
		temp_function->entry->st_type = FUNCTION_TYPE;
		temp_function->entry->inf_type = temp->ret_type;
	}
	parameters_optional RPAREN
	{
		if($6 != NULL){
			AST_Node_Decl_Params *temp = (AST_Node_Decl_Params *) $6;
			
			temp_function->entry->parameters = temp->parameters;
			temp_function->entry->num_of_pars = temp->num_of_pars;
		}
		else{
			temp_function->entry->parameters = NULL;
			temp_function->entry->num_of_pars = 0;
		}		
	}
;

return_type:
	type
	{
		$$ = new_ast_ret_type_node($1, 0);
	}
	| type pointer
	{
		$$ = new_ast_ret_type_node($1, 1);
	}
;

parameters_optional: 
	parameters
	{
		$$ = $1;
	}
	| /* empty */
	{
		$$ = NULL;
	}
;

parameters: 
	parameters COMMA parameter
	{
		AST_Node_Decl_Params *temp = (AST_Node_Decl_Params *) $1;
		$$ = new_ast_decl_params_node(temp->parameters, temp->num_of_pars, $3);
	}
	| parameter
	{
		$$ = new_ast_decl_params_node(NULL, 0, $1);
	}
;

parameter : { declare = 1; } type variable
{ 
	declare = 0;
	
	// Asignamos el tipo de entrada a la tabla de simbolos
	if($3->st_type == UNDEF){ /* tipo "simple" */
		set_type($3->st_name, $2, UNDEF); 
	}
	else if($3->st_type == POINTER_TYPE){ /* apuntador */
		set_type($3->st_name, POINTER_TYPE, $2);
	}
	else if($3->st_type == ARRAY_TYPE){ /* array  */
		set_type($3->st_name, ARRAY_TYPE, $2);
	}
	
	/* definimos parametros */
	$$ = def_param($2, $3->st_name, 0);
}
;

function_tail: LBRACE declarations_optional statements_optional return_optional RBRACE ;

declarations_optional: 
	declarations 
	{
		temp_function->declarations = $1;
	}
	| /* empty */
	{
		temp_function->declarations = NULL;
	}
;

statements_optional: 
	statements
	{
		temp_function->statements = $1;
	} 
	| /* empty */
	{
		temp_function->statements = NULL;
	}
;

return_optional:
	RETURN expression SEMI
	{
		temp_function->return_node = new_ast_return_node(temp_function->ret_type, $2);
	}
	| /* empty */
	{
		temp_function->return_node = NULL;
	}
;

%%

void yyerror ()
{
  fprintf(stderr, "Error Sintactico en la linea %d\n", lineno);
  exit(1);
}

void add_to_names(list_t *entry){
	// primera entrada
	if(nc == 0){
		nc = 1;
		names = (list_t **) malloc( 1 * sizeof(list_t *));
		names[0] = entry;
	}
	// caso general
	else{
		nc++;
		names = (list_t **) realloc(names, nc * sizeof(list_t *));
		names[nc - 1] = entry;		
	}
}

void add_to_vals(Value val){
	// primera entrada
	if(vc == 0){
		vc = 1;
		vals = (Value *) malloc(1 * sizeof(Value));
		vals[0] = val;
	}
	// cado general
	else{
		vc++;
		vals = (Value *) realloc(vals, vc * sizeof(Value));
		vals[vc - 1] = val;
	}
}

void add_elseif(AST_Node *elsif){
	// primera entrada
	if(elseif_count == 0){
		elseif_count = 1;
		elsifs = (AST_Node **) malloc(1 * sizeof(AST_Node));
		elsifs[0] = elsif;
	}
	// caso general
	else{
		elseif_count++;
		elsifs = (AST_Node **) realloc(elsifs, elseif_count * sizeof(AST_Node));
		elsifs[elseif_count - 1] = elsif;
	}
}

int main (int argc, char *argv[]){
	
	// Inicializar tabla de simbolos
	init_hash_table();
	
	// Inicializar tabla de revisitas
	queue = NULL;
	
	// Analisis Sintactico
	int flag;
	yyin = fopen(argv[1], "r");
	flag = yyparse();
	fclose(yyin);
	
	printf("Analisis Sintactico Completado!\n");
	
	/* Revisitas */
	revisit_queue *q = search_prev_queue("print");
	if(q == NULL){ /* Primera entrada */
		if(queue != NULL){ /* Si revisitas no vacia */
			queue = queue->next;
		}
	}
	else{
		q->next = q->next->next;
	}
	
	/* Hacer las revisitas que faltan */
	if(queue != NULL){
		revisit_queue *cur;
		cur = queue;
		while(cur != NULL){
			if(cur->revisit_type == ASSIGN_CHECK){
				revisit(cur->st_name);
			}
			cur = cur->next;
		}
	}
	
	/* Si revisitas no esta vacio => Warning */
	if(queue != NULL){
		printf("Warning! La lista de revisitas no esta vacia!\n");
	}
	
	/* Funcion print */
	func_declare("print", VOID_TYPE, 1, NULL);
	
	// Imprimiendo tabla de simbolos
	yyout = fopen("symtab_dump.out", "w");
	symtab_dump(yyout);
	fclose(yyout);
	
	// Imprimiendo lista de revisitas
	yyout = fopen("revisit_dump.out", "w");
	revisit_dump(yyout);
	fclose(yyout);
	
	return flag;
}
