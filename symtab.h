/* tamaño maximo de la tabla hash */
#define SIZE 211

/* tamaño maximo de los tokens */
#define MAXTOKENLEN 40

/* tipo de referencia */
#define BY_VALUE 1
#define BY_REFER 2

/* Tipo de valores que podemos tener */
typedef union Value{
	int ival;
	double fval;
	char cval;
	char *sval;
}Value;

/* struct de los parametros */
typedef struct Param{
	// Tipo y Nombre
	int par_type;
	char param_name[MAXTOKENLEN];
	
	// Para guardar cualquier tipo de valor que le pasemos a la tabla hash
	Value val;
	int passing; // valor de referencia
}Param;

/* Lista de las lineas de cada variable */
typedef struct RefList{ 
    int lineno;
    struct RefList *next;
}RefList;

// Nodo
typedef struct list_t{
	// Nombre, Tamaño, Alcance y Lineas
	char st_name[MAXTOKENLEN];
    int st_size;
    int scope;
    RefList *lines;
    
	// Para guardar cualquier tipo de valor que le pasemos al nodo
	Value val;
	
	// Tipo "principal"
    int st_type;
    
    // Tipo "secundario"
    // para arrays (info type), apuntadores (pointing type)
	// y funciones (return type)
	int inf_type;
	
	// Arrays
	Value *vals;
	int array_size;
	
	// Parametros
	Param *parameters;
	int num_of_pars;
	
	// Apuntador al siguiente elemento de la lista
	struct list_t *next;
}list_t;

/* Lista de revisitas */
typedef struct revisit_queue{
	// Entrada de la tabla de simbolos
	list_t *entry;
	
	// Identificador
	char *st_name;
	
	// Tipo de revisita
	int revisit_type;
	
	// Parametros de las llamadas a funcion
	int **par_types;
	int *num_of_pars;
	int num_of_calls;
	
	// Asignaciones
	void **nodes;
	int num_of_assigns;
	
	struct revisit_queue *next;
}revisit_queue;

/* Tipos de revisitas */
#define PARAM_CHECK 1  /* Revisar los parametros de una llamada a funcion cunado la funcion se declara */
#define ASSIGN_CHECK 2 /* Revisar asignacion cuando una funcion es parte de la expresion */

/* Listas */
static list_t **hash_table;
static revisit_queue *queue;


// Funciones de la tabla de simbolos
void init_hash_table(); // Inicializar tabla hash
unsigned int hash(char *key); // Funcion hash
void insert(char *name, int len, int type, int lineno); // Agregar entrada
list_t *lookup(char *name); // Buscar una entrada en la tabla hash
void symtab_dump(FILE *of); // Imprimir tabla de simbolos

// Funciones de tipo
void set_type(char *name, int st_type, int inf_type); // Asignar el tipo de la entrada (declaracion)
int get_type(char *name); // Obtener el tipo de una variable

// Funciones de alcance
void hide_scope(); // Ocultar el alcance actual
void incr_scope(); // Incrementar alcance

// Funciones de declaraciones y parametros
Param def_param(int par_type, char *param_name, int passing); // Definir parametros
int func_declare(char *name, int ret_type, int num_of_pars, Param *parameters); // Declarar funciones
int func_param_check(char *name, int num_of_calls, int** par_types, int *num_of_pars); // Revisar parametros

// Funciones de revisitas
void add_to_queue(list_t *entry, char *name, int type); // Añadir a revisitas
revisit_queue *search_queue(char *name); // Buescar en revisitas
revisit_queue *search_prev_queue(char *name); // Buscar entrada previa
int revisit(char *name); // Revisitar
void revisit_dump(FILE *of); // Imprimir lista de revisitas
