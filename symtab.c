#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "semantics.h"
#include "symtab.h"

/* Alcance */
int cur_scope = 0;

/* Bandera de declaracion de entrada */
int declare = 0; // 1: entrada declarandose

/* Bandera de declaracion de funcion */
int function_decl = 0; // 1: funcion declarandose

// Funciones de la tabla de simbolos

void init_hash_table(){
	int i; 
	hash_table = malloc(SIZE * sizeof(list_t*));
	for(i = 0; i < SIZE; i++) hash_table[i] = NULL;
}

unsigned int hash(char *key){
	unsigned int hashval = 0;
	for(;*key!='\0';key++) hashval += *key;
	hashval += key[0] % 11 + (key[0] << 3) - key[0];
	return hashval % SIZE;
}

void insert(char *name, int len, int type, int lineno){
	unsigned int hashval = hash(name);
	list_t *l = hash_table[hashval];
	
	while ((l != NULL) && (strcmp(name,l->st_name) != 0)) l = l->next;
	
	/* Si la entrada no esta en la tabla */
	if (l == NULL){
		/* Vemos si la entrada se esta declarando */
		if(declare == 1){
			/* Datos de la entrada */
			l = (list_t*) malloc(sizeof(list_t));
			strncpy(l->st_name, name, len);
			l->st_size = len;
			l->st_type = type;
			l->scope = cur_scope;
			l->lines = (RefList*) malloc(sizeof(RefList));
			l->lines->lineno = lineno;
			l->lines->next = NULL;
			
			/* La añadimos a la tabla hash */
			l->next = hash_table[hashval];
			hash_table[hashval] = l; 
			printf("Se agrego %s por primera vez en la linea %d!\n", name, lineno);
		}
		else{
			/* La añadimos a la tabla para revisarla despues */
			l = (list_t*) malloc(sizeof(list_t));
			strncpy(l->st_name, name, len);
			l->st_size = len;
			l->st_type = type;
			l->scope = cur_scope;
			l->lines = (RefList*) malloc(sizeof(RefList));
			l->lines->lineno = lineno;
			l->lines->next = NULL;
			l->next = hash_table[hashval];
			hash_table[hashval] = l;
			printf("Se agrego %s en la linea %d para revisar despues!\n", name, lineno);
			
			/* Agregamos la entrada a revisitas */
			add_to_queue(l, l->st_name, PARAM_CHECK);
		}
	}
	/* Si la entrada esta en la tabla */
	else{
		// Si la entrada no esta declarandose
		// Solo añadimos el nuevo numero de linea a la lista de lineas
		if(declare == 0){
			/* Buscamos la ultima linea de la variable */
			RefList *t = l->lines;
			while (t->next != NULL) t = t->next;
			
			/* Añadimos el numero de linea de esta iteracion a la lista de lineas */
			t->next = (RefList*) malloc(sizeof(RefList));
			t->next->lineno = lineno;
			t->next->next = NULL;
			printf("Se encontro %s de nuevo en la linea %d!\n", name, lineno);
		}
		// Si la entrada esta declarandose
		// Hay que revisar si el alcance es diferente
		else{
			/* Mismo alcance = Error: Declaracion repetida! */
			if(l->scope == cur_scope){
				fprintf(stderr, "La variable %s se volvio a declarar en la linea %d\n!", name, lineno);
 				exit(1);
			}
			/* Distinto alcance, pero es una declaracion de funcion */
			else if(function_decl == 1){
				/* Buscamos la ultima linea de la entrada */
				RefList *t = l->lines;
				while (t->next != NULL) t = t->next;
				
				/* Añadimos el numero de linea de esta iteracion a la lista de lineas */
				t->next = (RefList*) malloc(sizeof(RefList));
				t->next->lineno = lineno;
				t->next->next = NULL;
			}
			/* Distinto alcance = añadir entrada */
			else{
				/* Datos de la variable */
				l = (list_t*) malloc(sizeof(list_t));
				strncpy(l->st_name, name, len);
				l->st_size = len;
				l->st_type = type;
				l->scope = cur_scope;
				l->lines = (RefList*) malloc(sizeof(RefList));
				l->lines->lineno = lineno;
				l->lines->next = NULL;
				
				/* La añadimos a la tabla hash */
				l->next = hash_table[hashval];
				hash_table[hashval] = l; 
				printf("Agregando %s con un nuevo alcance en la linea %d!\n", name, lineno);
			}	
		}		
	}
}

//Buscar la entrada en la tabla hash
list_t *lookup(char *name){ /* Retorna la entrada si lo encuentra o NULL si no */
	unsigned int hashval = hash(name);
	list_t *l = hash_table[hashval];
	while ((l != NULL) && (strcmp(name,l->st_name) != 0)) l = l->next;
	return l;
}

/* Imprimir la tabla de simbolos */ 
void symtab_dump(FILE * of){
  int i;
  fprintf(of,"------------ ------ ------ ------------\n");
  fprintf(of,"Nombre       Tipo   Alc.   Lineas      \n");
  fprintf(of,"------------ ------ ------ ------------\n");
  for (i=0; i < SIZE; ++i){ 
	if (hash_table[i] != NULL){ 
		list_t *l = hash_table[i];
		while (l != NULL){ 
			RefList *t = l->lines;
			fprintf(of,"%-13s",l->st_name);
			if (l->st_type == INT_TYPE)                fprintf(of,"%-15s","int");
			else if (l->st_type == REAL_TYPE)          fprintf(of,"%-15s","real");
			else if (l->st_type == CHAR_TYPE)          fprintf(of,"%-15s","char");
			else if (l->st_type == VOID_TYPE)          fprintf(of,"%-15s","void");
			else if (l->st_type == ARRAY_TYPE){
				fprintf(of,"array de ");
				if (l->inf_type == INT_TYPE) 		   fprintf(of,"%-6s","int");
				else if (l->inf_type  == REAL_TYPE)    fprintf(of,"%-6s","real");
				else if (l->inf_type  == CHAR_TYPE)    fprintf(of,"%-6s","char");
				else fprintf(of,"%-13s","undef");
			}
			else if (l->st_type == POINTER_TYPE){
				fprintf(of,"apuntador a ");
				if (l->inf_type == INT_TYPE) 		   fprintf(of,"%-4s","int");
				else if (l->inf_type  == REAL_TYPE)    fprintf(of,"%-4s","real");
				else if (l->inf_type  == CHAR_TYPE)    fprintf(of,"%-4s","char");
				else if (l->inf_type  == VOID_TYPE)    fprintf(of,"%-4s","void");
				else fprintf(of,"%-4s","undef");
			}
			else if (l->st_type == FUNCTION_TYPE){
				fprintf(of,"func ret ");
				if (l->inf_type == INT_TYPE) 		   fprintf(of,"%-6s","int");
				else if (l->inf_type  == REAL_TYPE)    fprintf(of,"%-6s","real");
				else if (l->inf_type  == CHAR_TYPE)	   fprintf(of,"%-6s","char");
				else if (l->inf_type  == VOID_TYPE)	   fprintf(of,"%-6s","void");
				else fprintf(of,"%-4s","undef");
			}
			else fprintf(of,"%-15s","undef"); // si UNDEF o 0
			fprintf(of,"  %d  ",l->scope);
			while (t != NULL){
				fprintf(of,"%4d ",t->lineno);
			t = t->next;
			}
			fprintf(of,"\n");
			l = l->next;
		}
    }
  }
}

// Funciones de tipo

void set_type(char *name, int st_type, int inf_type){ // Asignar el tipo de la entrada (declaracion)
	/* Buscamos la entrada en la tabla hash*/
	list_t *l = lookup(name);
	
	/* Asignamos tipo "principal" (int, char, etc.) */
	l->st_type = st_type;
	
	/* Asignamos tipo "secundario" (puntero, array, funcion o simple (UNDEF)) */
	if(inf_type != UNDEF){
		l->inf_type = inf_type;
	}
}

int get_type(char *name){ // Obtener el tipo de la entrada
	/* Buscamos la entrada en la tabla hash*/
	list_t *l = lookup(name);
	
	/* Si tipo simple (UNDEF) */
	if(l->st_type == INT_TYPE || l->st_type == REAL_TYPE || l->st_type == CHAR_TYPE){
		return l->st_type;
	}
	/* Si tipo secundario */
	else{
		return l->inf_type;
	}
}

// Funciones de alcance

void hide_scope(){ /* Ocultar el alcance actual */
	list_t *l;
	int i;
	printf("Ocultando alcance \'%d\':\n", cur_scope);
	/* para todas las listas */
	for (i = 0; i < SIZE; i++){
		if(hash_table[i] != NULL){
			l = hash_table[i];
			/* Encontrar la primera variable con otro alcance */
			while(l != NULL && l->scope == cur_scope){
				printf("Ocultando %s..\n", l->st_name);
				l = l->next;
			}
			/* Poner toda la lista con ese alcance */
			hash_table[i] = l;
		}
	}
	cur_scope--;
}

void incr_scope(){ /* Incrementar alcance */
	cur_scope++;
}

// Funciones de declaraciones y parametros

Param def_param(int par_type, char *param_name, int passing){ // Definir parametros
	Param param; /* struct parametros */
	
	/* Datos de los parametros */
	param.par_type = par_type;
	strcpy(param.param_name, param_name);
	param.passing = passing;	
	
	/* Retornar la estructura */
	return param;
}

int func_declare(char *name, int ret_type, int num_of_pars, Param *parameters){ // Funcion de declaracion
	/* Buscamos la entrada en la tabla hash*/
	list_t *l = lookup(name);
	
	if(l != NULL){
		/* Si el tipo de la entrada no esta declarado todavia */
		if(l->st_type == UNDEF){
			/* la variable es del tipo "funcion" */
			l->st_type = FUNCTION_TYPE;
			
			/* return */
			l->inf_type = ret_type;
			
			/* parametros */
			l->num_of_pars = num_of_pars;
			l->parameters = parameters;
			
			return 0;
		}
		/* Funcion ya declarada */
		else{
			fprintf(stderr, "La funcion %s se ha vuelto a declarar!\n", name);
			exit(1);
		}
	}
}

int func_param_check(char *name, int num_of_calls, int** par_types, int *num_of_pars){ // Revisando parametros
	int i, j, type_1, type_2;
	
	/* Buscamos la variable en la tabla hash*/
	list_t *l = lookup(name);
	
	/* Para todas las llamadas a la funcion */
	for(i = 0 ; i < num_of_calls ; i++){
		/* Revisando el numero de parametros */
		if(l->num_of_pars != num_of_pars[i]){
			fprintf(stderr, "La llamada a funcion de %s en la linea %d tiene un numero erroneo de parametros!\n", name, lineno);
			exit(1);
		}
		/* Revisando si los parametros son compatibles */
		for(j = 0; j < num_of_pars[i]; j++){
			/* Tipo de la funcion en su declaracion */
			type_1 = l->parameters[j].par_type; 
			
			/* Tipo de la funcion en su llamada*/
			type_2 = par_types[i][j]; 
			
			/* Revisando compatibilidad */
			get_result_type(type_1, type_2, NONE);
		}
	}
	
	return 0;
}

// Funciones de revisitas

void add_to_queue(list_t *entry, char *name, int type){ /* Agregar a las revisitas */
	revisit_queue *q;
	
	/* Si revisitas vacia */
	if(queue == NULL){
		/* Datos de la entrada */
		q = (revisit_queue*) malloc(sizeof(revisit_queue));
		q->entry = entry;
		q->st_name = name;
		q->revisit_type = type;
		q->next = NULL;
		
		/* parametros o asignaciones */
		if(type == PARAM_CHECK){
			q->num_of_calls = 0;
		}
		else if(type == ASSIGN_CHECK){
			q->num_of_assigns = 0;
		}
		
		/* Agregar entrada al inicio */
		queue = q;
	}
	/* Si revisitas no vacia */
	else{
		/* Buscamos la ultima variable */
		q = queue;
		while(q->next != NULL) q = q->next;
		
		/* Agregar entrada al final */
		q->next = (revisit_queue*) malloc(sizeof(revisit_queue));
		q->next->entry = entry;
		q->next->st_name = name;
		q->next->revisit_type = type;
		q->next->next = NULL;
		
		/* parametros o asignaciones */
		if(type == PARAM_CHECK){
			q->next->num_of_calls = 0;
		}
		else if(type == ASSIGN_CHECK){
			q->next->num_of_assigns = 0;
		}
	}		
}

revisit_queue *search_queue(char *name){ /* Buscar en revisitas */
	revisit_queue *q;
	
	/* Buscando entrada */
	q = queue;
	while( (q != NULL) && (strcmp(q->st_name, name) != 0) ) q = q->next;
	
	return q;
}

revisit_queue *search_prev_queue(char *name){ //Buscar entrada anterior
	revisit_queue *q;
	
	/* Si revisitas vacia */
	if(queue == NULL){
		return NULL;
	}	
	
	/* Buscando en primera posicion */
	if( strcmp(queue->st_name, name) == 0 ){
		return NULL;
	}
	
	/* Buscando en el resto */
	q = queue;
	while( (q != NULL) && (strcmp(q->next->st_name, name) != 0) ) q = q->next;
	
	return q;
}

int revisit(char *name){ /* Revisitando entrada y quitandola */
	int i, type1, type2;

	revisit_queue *q = search_queue(name);
	revisit_queue *q2;
	
	if(q == NULL){
		return -1; // Si no se encuentra
	}
	
	/* Tipo de revisita */
	switch(q->revisit_type){
		case PARAM_CHECK:			
			/* Parametros */
			if(!func_param_check(name, q->num_of_calls, q->par_types, q->num_of_pars)){
				printf("Parametros de %s correctos\n", name);
			}
			
			/* Remover entrada de revisitas haciendo que la entrada previa apunte a la posterior */
			q2 = search_prev_queue(name);
			if(q2 == NULL){ /* Si el anterior elemento es el primero de revisitas */
				queue = queue->next;
			}
			else{
				q2->next = q2->next->next;
			}
			
			break;
		case ASSIGN_CHECK:
			/* Asignaciones */
			type1 = get_type(q->entry->st_name);
			for(i = 0; i < q->num_of_assigns; i++){
				type2 = expression_data_type(q->nodes[i]);
				
				/* Checando compatibilidad de tipos */
				get_result_type(
					type1,   /*  tipo de la entrada  */
					type2,   /* tipo de la expresion */
					NONE  /* Operador de compatibilidad */
				);
			}
		
			/* Remover entrada de revisitas haciendo que la entrada previa apunte a la posterior */
			q2 = search_prev_queue(name);
			if(q2 == NULL){ /* Si el anterior elemento es el primero de revisitas */
				queue = queue->next;
			}
			else{
				q2->next = q2->next->next;
			}
			
			break;
		/* ... */
	}
	
	return 0; // success
}

void revisit_dump(FILE *of){
	int i;
	revisit_queue *q;
	q = queue;
	
	fprintf(of,"------------ -------------\n");
	fprintf(of,"ID           Tipo Revisita\n");
	fprintf(of,"------------ -------------\n");
  	while(q != NULL){
  		fprintf(of, "%-13s", q->st_name);
  		if(q->revisit_type == PARAM_CHECK){
  			fprintf(of,"%s","Parametros");
  			fprintf(of,"for %d function calls",q->num_of_calls);
		}
		else if(q->revisit_type == ASSIGN_CHECK){
  			fprintf(of,"%s","Asignocaiones ");
  			fprintf(of,"for %d assignments",q->num_of_assigns);
		}
		// more later on
		fprintf(of, "\n");
  		q = q->next;	
	}
}
