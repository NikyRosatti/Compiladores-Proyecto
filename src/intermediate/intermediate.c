#include "Tree.h"
#include "Intermediate.h"



// DEBE COINCIDIR EN POSICION CON EL ENUM IRInstr
static const char *ir_names[] = {
    "LOAD","DECL", "STORE","STORAGE", "ADD","SUB", "UMINUS", "MUL","DIV","MOD",
    "AND","OR","NOT",
    "EQ","NEQ","LT","LE","GT","GE",
    "LABEL","GOTO", "RET", "PARAM", "CALL", "METHOD", "F_METHOD", "METH_EXT",
    "PRINT", "SAVE_PARAM"
};

static int tempCount = 0;
static int labelCount = 0;

Symbol* newTempSymbol() {
    Symbol *s = malloc(sizeof(Symbol));
    char buf[16];
    sprintf(buf, "t%d", tempCount++);
    s->name = strdup(buf);
    s->type = TYPE_INT;  
    s->is_global = 0;
    s->is_temp = 1;
    s->offset = 0;
    return s;
}

Symbol* newLabel() {
    Symbol *s = malloc(sizeof(Symbol));
    char buf[16];
    sprintf(buf, "L%d", labelCount++);
    s->name = strdup(buf);
    s->type = TYPE_LABEL;
    return s;
}

int param = 0;
int index_param = 0;


/**
 * @brief Genera el código para los argumentos de un método en orden inverso (R->L)
 * y emite las instrucciones IR_PARAM con el índice correcto (L->R).
 * * @param arg_list_node El nodo 'NODE_LIST' que empieza la lista de argumentos.
 * @param list La lista de IR.
 * @param current_index El índice del parámetro actual (empezando en 0 para el de más a la izquierda).
 */
static void gen_method_args(Tree *arg_list_node, IRList *list, int current_index) {
    if (!arg_list_node) {
        return; // Fin de la lista de argumentos
    }

    // IR AL FONDO DE LA LISTA PRIMERO (DERECHA a IZQUIERDA)
    gen_method_args(arg_list_node->right, list, current_index + 1);

    // PROCESAR EL NODO ACTUAL (IZQUIERDA)
    // Evalúa los argumentos de derecha a izquierda.
    Symbol *arg_value_temp = gen_code(arg_list_node->left, list);

    // Crea un símbolo "dummy" solo para pasar el índice del parámetro
    Symbol *param_index_sym = malloc(sizeof(Symbol));
    param_index_sym->name = NULL;
    param_index_sym->type = TYPE_INT;
    param_index_sym->valor.value = current_index;

    // Emitir la instrucción IR_PARAM
    ir_emit(list, IR_PARAM, arg_value_temp, param_index_sym, NULL);
}


Symbol* gen_code(Tree *node, IRList *list) {
    if (!node) return NULL;

    if (node->sym == NULL && 
        (node->tipo == NODE_ID || node->tipo == NODE_ASSIGN || node->tipo == NODE_METHOD_CALL)) {
        fprintf(stderr, "Error: nodo tipo %d sin símbolo\n", node->tipo);
        return NULL;
    }

    
    switch (node->tipo) {
        case NODE_T_INT:
        case NODE_T_BOOL:
        case NODE_T_VOID:
            return NULL;

        case NODE_INT:
        case NODE_TRUE:
        case NODE_FALSE: {

            
            

            // 1. Crear un nuevo símbolo temporal para guardar el valor del literal.
            Symbol *temp_sym = newTempSymbol();

            // 2. Crear un símbolo simple para encapsular el valor del literal.
            //    Este no es un temporal en la pila, solo un portador del valor.
            Symbol *literal_val_sym = malloc(sizeof(Symbol));
            if (node->tipo == NODE_INT) {
                literal_val_sym->valor.value = node->sym->valor.value;
            } else {
                literal_val_sym->valor.value = (node->tipo == NODE_TRUE) ? 1 : 0;
            }
            literal_val_sym->name = NULL;
            literal_val_sym->is_param = 0; // Importante: Limpiar este flag
            // 3. Emitir una instrucción para ALMACENAR el valor literal en el temporal.
            //    Esta es la clave: le decimos al generador que mueva el número a la pila.
            
            ir_emit(list, IR_STORAGE, literal_val_sym, NULL, temp_sym);

            // 4. Devolver el símbolo temporal, que ahora contiene el valor.
            return temp_sym;
        }

        case NODE_ID: {
            Symbol *t = newTempSymbol();
            ir_emit(list, IR_LOAD, node->sym, NULL, t);
            return t;
        }

        case NODE_SUM: {
            Symbol *l = gen_code(node->left, list);
            Symbol *r = gen_code(node->right, list);
            Symbol *t = newTempSymbol();
            ir_emit(list, IR_ADD, l, r, t);
            return t;
        }

        case NODE_RES: {
            Symbol *l = gen_code(node->left, list);
            Symbol *r = gen_code(node->right, list);
            Symbol *t = newTempSymbol();
            ir_emit(list, IR_SUB, l, r, t);
            return t;
        }

        case NODE_DIV: {
            Symbol *l = gen_code(node->left, list);
            Symbol *r = gen_code(node->right, list);
            Symbol *t = newTempSymbol();
            ir_emit(list, IR_DIV, l, r, t);
            return t;
        }

        case NODE_MUL: {
            Symbol *l = gen_code(node->left, list);
            Symbol *r = gen_code(node->right, list);
            Symbol *t = newTempSymbol();
            ir_emit(list, IR_MUL, l, r, t);
            return t;
        }

        case NODE_MOD: {
            Symbol *l = gen_code(node->left, list);
            Symbol *r = gen_code(node->right, list);
            Symbol *t = newTempSymbol();
            ir_emit(list, IR_MOD, l, r, t);
            return t;
        }

        case NODE_NOT: {
            Symbol *l = gen_code(node->left, list);
            Symbol *t = newTempSymbol();
            ir_emit(list, IR_NOT, l, NULL, t);
            return t;
        }

        case NODE_AND: {
            Symbol *l = gen_code(node->left, list);
            Symbol *r = gen_code(node->right, list);
            Symbol *t = newTempSymbol();
            ir_emit(list, IR_AND, l, r, t);
            return t;

        }

        case NODE_OR: {
            Symbol *l = gen_code(node->left, list);
            Symbol *r = gen_code(node->right, list);
            Symbol *t = newTempSymbol();
            ir_emit(list, IR_OR, l, r, t);
            return t;
        }

        case NODE_EQ: {
            Symbol *l = gen_code(node->left, list);
            Symbol *r = gen_code(node->right, list);
            Symbol *t = newTempSymbol();
            ir_emit(list, IR_EQ, l, r, t);
            return t;
        }

        case NODE_NEQ: {
            Symbol *l = gen_code(node->left, list);
            Symbol *r = gen_code(node->right, list);
            Symbol *t = newTempSymbol();
            ir_emit(list, IR_NEQ, l, r, t);
            return t;
        }

        case NODE_LT: {
            Symbol *l = gen_code(node->left, list);
            Symbol *r = gen_code(node->right, list);
            Symbol *t = newTempSymbol();
            ir_emit(list, IR_LT, l, r, t);
            return t;
        }

        case NODE_GT: {
            Symbol *l = gen_code(node->left, list);
            Symbol *r = gen_code(node->right, list);
            Symbol *t = newTempSymbol();
            ir_emit(list, IR_GT, l, r, t);
            return t;
        }

        case NODE_LE: {
            Symbol *l = gen_code(node->left, list);
            Symbol *r = gen_code(node->right, list);
            Symbol *t = newTempSymbol();
            ir_emit(list, IR_LE, l, r, t);
            return t;
        }

        case NODE_GE: {
            Symbol *l = gen_code(node->left, list);
            Symbol *r = gen_code(node->right, list);
            Symbol *t = newTempSymbol();
            ir_emit(list, IR_GE, l, r, t);
            return t;
        }


        case NODE_UMINUS: {
            Symbol *val = gen_code(node->left, list);
            Symbol *t = newTempSymbol();
            ir_emit(list, IR_UMINUS, val, NULL, t);
            return t;
        }

        case NODE_DECLARATION: {
            // node->sym es la variable que se declara
            // node->right es el inicializador (ej. '100' o 'a+b' o NULL)

            if (!node->right)
            {
                // Caso: Sin inicialización
                
                // Emitir IR_DECL con valor NULL.
                // 'collect_globals' lo verá y 'print_globals_data'
                // lo interpretará como '.quad 0'.
                ir_emit(list, IR_DECL, NULL, NULL, node->sym);
            }
            else
            {
                // Caso: Con inicialización
                // Comprobar si es una inicialización constante
                bool is_static_const = (node->right->tipo == NODE_INT ||
                                        node->right->tipo == NODE_TRUE ||
                                        node->right->tipo == NODE_FALSE);

                // Comprobar si la variable es global
                bool is_global = node->sym->is_global;

                if (is_global && is_static_const)
                {
                    // Inicialización Estática Global

                    // Crear un Símbolo Constante para el valor.
                    Symbol *const_val = malloc(sizeof(Symbol));
                    if (!const_val) {
                        fprintf(stderr, "Error de memoria en DECL\n");
                        break;
                    }

                    const_val->name = NULL; // Es un literal, no tiene nombre
                    const_val->type = (node->right->tipo == NODE_INT) ? TYPE_INT : TYPE_BOOL;
                    const_val->is_param = 0;

                    if (node->right->tipo == NODE_INT)
                    {
                        const_val->valor.value = node->right->sym->valor.value;
                    }
                    else if (node->right->tipo == NODE_TRUE)
                    {
                        const_val->valor.value = 1;
                    }
                    else
                    { // NODE_FALSE
                        const_val->valor.value = 0;
                    }

                    // Emitir IR_DECL con el valor constante (en arg1).
                    //    'collect_globals' usará 'const_val'
                    ir_emit(list, IR_DECL, const_val, NULL, node->sym);
                }
                else
                {
                    // Inicialización Dinámica o Local

                    // Si es una variable global dinámica (ej. g = a+b),
                    //    primero debemos declararla en .data con 0.
                    if (is_global)
                    {
                        // 'collect_globals' lo pondrá en .data como '.quad 0'
                        ir_emit(list, IR_DECL, NULL, NULL, node->sym);
                    }

                    // Generar el código para la expresión
                    Symbol *rhs = gen_code(node->right, list);

                    // Emitir un IR_STORE para asignar el valor.
                    ir_emit(list, IR_STORE, rhs, NULL, node->sym);
                }
            }
            break;
        }

        case NODE_ASSIGN: {
            Symbol *l = gen_code(node->left, list);
            Symbol *r = gen_code(node->right, list);
            ir_emit(list, IR_STORE, l, r, node->sym);
            return node->sym;
        }

        case NODE_METHOD_CALL: {


            // El AST parece ser: node->right (NODE_ARGS) -> left (NODE_LIST)
            Tree *args_node = node->right;
            Tree *arg_list = (args_node && args_node->left) ? args_node->left : NULL;

            // 1. Generar código para todos los argumentos.
            //    Esta función los evaluará de DERECHA a IZQUIERDA
            //    y emitirá las instrucciones IR_PARAM en ese orden.
            gen_method_args(arg_list, list, 0); // Empezar con índice 0

            // Crea un temporal para el valor de retorno de la función
            Symbol *t = newTempSymbol();

            // Emitir la llamada a la función
            ir_emit(list, IR_CALL, node->sym, NULL, t);

            // Devolver el temporal que contendrá el resultado
            return t;
        }

        case NODE_PROGRAM:
        case NODE_CODE:
        case NODE_BLOCK:
        case NODE_LIST:
        case NODE_ARGS: {
            gen_code(node->left, list);
            gen_code(node->right, list);
            break;
        }

        case NODE_METHOD: {


            // ES UN METODO EXTERNO
            if (node->right == NULL) {
                ir_emit(list, IR_METH_EXT, NULL, NULL, node->sym);
            } else {
                // Etiqueta para inicio del método
                if (node->sym) {
                    ir_emit(list, IR_METHOD, NULL, NULL, node->sym);
                }

                Tree *method_decl = node->left;                             // NODE_METHOD_HEADER
                Tree *args_node = method_decl ? method_decl->right : NULL;  // ARGS
                Tree *param_list = (args_node && args_node->left) ? args_node->left : NULL;  // Primer LIST
                
                while (param_list) {
                    Tree *param_decl = param_list->left;
                    if (param_decl && param_decl->sym) {
                        Symbol *param_sym = param_decl->sym;
                        
                        // Solo necesitamos guardar los que vienen por registro (0-5)
                        if (param_sym->is_param && param_sym->param_index < 6) {
                            // Usamos arg1 para pasar el símbolo del parámetro
                            ir_emit(list, IR_SAVE_PARAM, param_sym, NULL, NULL); 
                        }
                    }
                    param_list = param_list->right; // Siguiente parámetro
                }

                // Cuerpo del método
                gen_code(node->right, list);
                ir_emit(list, IR_FMETHOD, NULL, NULL, node->sym);
            }
            break;
        }

        case NODE_IF: {
            Symbol *cond = gen_code(node->left, list); // condición
            Symbol *label_end = newLabel();
            //GOTO SALTA SI ES FALSO, SINO CONTINUA LA EJECUCION SECUENCIAL//
            ir_emit(list, IR_GOTO, cond, NULL, label_end);
            gen_code(node->right, list); // cuerpo del if
            ir_emit(list, IR_LABEL, NULL, NULL, label_end);
            break;
        }

        case NODE_IF_ELSE: {
            Symbol *cond = gen_code(node->left, list); // condición
            Symbol *label_else = newLabel();
            Symbol *label_end = newLabel();
            ir_emit(list, IR_GOTO, cond, NULL, label_else);
            gen_code(node->right->left, list); // cuerpo del if (then)
            ir_emit(list, IR_GOTO, NULL, NULL, label_end);
            ir_emit(list, IR_LABEL, NULL, NULL, label_else);
            gen_code(node->right->right, list); // cuerpo del else
            ir_emit(list, IR_LABEL, NULL, NULL, label_end);
            break;
        }

        case NODE_RETURN: {
            // no es un return void
            if (node->left != NULL){
                Symbol *l = gen_code(node->left, list);
                ir_emit(list, IR_RETURN, l, NULL, NULL);
                return NULL;
            }
            ir_emit(list, IR_RETURN, NULL, NULL, NULL);
            break;
        }

        case NODE_WHILE: {
            Symbol *label_start = newLabel();
            Symbol *label_end = newLabel();
            ir_emit(list, IR_LABEL, NULL, NULL, label_start);
            Symbol *cond = gen_code(node->left, list);
            ir_emit(list, IR_GOTO, cond, NULL, label_end);
            gen_code(node->right, list);
            ir_emit(list, IR_GOTO, NULL, NULL, label_start);
            ir_emit(list, IR_LABEL, NULL, NULL, label_end);
            break;
        }

        case NODE_PARENS: return gen_code(node->left, list);


        case NODE_METHOD_HEADER: break;

        default:
            // Para depuración: nodo no manejado
            fprintf(stderr, "Nodo no soportado en gen_code: %d\n", node->tipo);
            break;

        
    }
    return NULL;
}



void ir_init(IRList *list) {
    list->codes = NULL;
    list->size = 0;
    list->capacity = 0;
}



void ir_emit(IRList *list, IRInstr op, Symbol *arg1, Symbol *arg2, Symbol *result) {
    // redimensionar si no hay lugar
    if (list->size >= list->capacity) {
        list->capacity = (list->capacity == 0) ? 4 : list->capacity * 2;
        list->codes = realloc(list->codes, list->capacity * sizeof(IRCode));
    }

    // agregar nueva instrucción
    IRCode *code = &list->codes[list->size++];
    code->op = op;
    code->arg1 = arg1;
    code->arg2 = arg2;
    code->result = result;
}



void ir_print(IRList *list) {
    for (int i = 0; i < list->size; i++) {
        IRCode *code = &list->codes[i];
        if (strcmp(ir_names[code->op], "METH_EXT") == 0)
        {
            printf("EXTERN");    
        } else {
            printf("%s", ir_names[code->op]);
        }

        switch (code->op){
            
            case IR_ADD:
            case IR_SUB:
            case IR_MUL:
            case IR_DIV:
            case IR_MOD:
            case IR_AND:
            case IR_OR:
            case IR_EQ:
            case IR_NEQ:
            case IR_LT:
            case IR_LE:
            case IR_GT:
            case IR_GE:
                if (code->arg1) {
                    if (code->arg1->name)
                        printf(" %s", code->arg1->name);
                    else
                        printf(" %d", code->arg1->valor.value);
                }
                if (code->arg2) {
                    if (code->arg2->name)
                        printf(", %s", code->arg2->name);
                    else
                        printf(", %d", code->arg2->valor.value);
                }
                if (code->result) {
                    if (code->result->name)
                        printf(", %s", code->result->name);
                    else
                        printf(", %d", code->result->valor.value);
                }
                break;
            case IR_STORE:
            case IR_STORAGE:
            case IR_SAVE_PARAM:
            case IR_LOAD:
            case IR_CALL:
            case IR_NOT:
            case IR_UMINUS:
            case IR_GOTO:
                if (code->arg1) {
                    if (code->arg1->name)
                        printf(" %s", code->arg1->name);
                    else
                        printf(" %d", code->arg1->valor.value);
                }
                if (code->arg2) {
                    if (code->arg2->name)
                        printf(", %s", code->arg2->name);
                    else
                        printf(", %d", code->arg2->valor.value);
                }
                if (code->result) {
                    if (code->result->name)
                        printf(", %s", code->result->name);
                    else
                        printf(", %d", code->result->valor.value);
                }
                break;
            case IR_LABEL:
            case IR_METH_EXT:
            case IR_DECL:
                printf(" %s", code->result->name);
                break;

            case IR_METHOD:
            case IR_FMETHOD:
                printf(": %s", code->result->name);
                break;

            case IR_RETURN:
                if (code->arg1) {
                    printf(" %s", code->arg1->name);
                }
                break;
            
            case IR_PARAM:
            case IR_PRINT:


                /* code */
                break;
            
            default:
                printf("CASO DEFAULT");
                break;
        }
        printf("\n");
    }
}

void ir_free(IRList *list) {
    free(list->codes);
    list->codes = NULL;
    list->size = 0;
    list->capacity = 0;
}

void offset_temps(IRList *list) {
    int temp_offset = 0;            // Offset para temporales (negativo)
    Symbol *current_method = NULL;

    for (int i = 0; i < list->size; i++) {
        IRCode *code = &list->codes[i];

        if (code->op == IR_METHOD) {
            // Entramos a un método
            current_method = code->result;
            if (current_method) {
                // Empieza el offset de temporales justo después de los locals
                temp_offset = (-current_method->total_stack_space) - 8;
            }
            continue;
        }

        if (code->op == IR_FMETHOD) {
            // Guardar total de stack incluyendo temporales
            if (current_method)
                current_method->total_stack_space = -temp_offset - 8;
            current_method = NULL;
            continue;
        }

        // Asignar offset a los temporales dentro del método
        if (current_method && code->result && code->result->is_temp && code->result->offset == 0) {
            code->result->offset = temp_offset;
            temp_offset -= 8;
        }
    }
}
