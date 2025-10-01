#include "Tree.h"
#include "intermediate.h"

// DEBE COINCIDIR EN POSICION CON EL ENUM IRInstr
static const char *ir_names[] = {
    "LOAD","STORE","ADD","SUB","MUL","DIV","MOD",
    "AND","OR","NOT",
    "EQ","NEQ","LT","LE","GT","GE",
    "LABEL","GOTO","IFZ","RETURN","PARAM","CALL", "PRINT"
};

static int tempCount = 0;

char* newTemp() {
    char *buf = malloc(16);
    sprintf(buf, "t%d", tempCount++);
    return buf;
}

char* newLAbel() {
    char *buf = malloc(16);
    sprintf(buf, "L%d", tempCount++);
    return buf;
}

char* gen_code(Tree *node, IRList *list) {
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

        case NODE_INT: {
            char *t = newTemp();
            char buf[16];
            sprintf(buf, "%d", node->sym->valor.value);
            ir_emit(list, IR_LOAD, strdup(buf), NULL, t);
            return t;
        }

        case NODE_TRUE: {
            char *t = newTemp();
            ir_emit(list, IR_LOAD, "TRUE", NULL, t);
            return t;
        }

        case NODE_FALSE: {
            char *t = newTemp();
            ir_emit(list, IR_LOAD, "FALSE", NULL, t);
            return t;
        }

        case NODE_ID: {
            char *t = newTemp();
            ir_emit(list, IR_LOAD, node->sym->name, NULL, t);
            return t;
        }

        case NODE_SUM: {
            char *l = gen_code(node->left, list);
            char *r = gen_code(node->right, list);
            char *t = newTemp();
            ir_emit(list, IR_ADD, l, r, t);
            return t;
        }

        case NODE_RES: {
            char *l = gen_code(node->left, list);
            char *r = gen_code(node->right, list);
            char *t = newTemp();
            ir_emit(list, IR_SUB, l, r, t);
            return t;
        }

        case NODE_DIV: {
            char *l = gen_code(node->left, list);
            char *r = gen_code(node->right, list);
            char *t = newTemp();
            ir_emit(list, IR_DIV, l, r, t);
            return t;
        }

        case NODE_MUL: {
            char *l = gen_code(node->left, list);
            char *r = gen_code(node->right, list);
            char *t = newTemp();
            ir_emit(list, IR_MUL, l, r, t);
            return t;
        }

        case NODE_MOD: {
            char *l = gen_code(node->left, list);
            char *r = gen_code(node->right, list);
            char *t = newTemp();
            ir_emit(list, IR_MOD, l, r, t);
            return t;
        }

        case NODE_NOT: {
            char *l = gen_code(node->left, list);
            char *t = newTemp();
            ir_emit(list, IR_NOT, l, NULL, t);
            return t;
        }

        case NODE_AND: {
            char *l = gen_code(node->left, list);
            char *r = gen_code(node->right, list);
            char *t = newTemp();
            ir_emit(list, IR_AND, l, r, t);
            return t;

        }

        case NODE_OR: {
            char *l = gen_code(node->left, list);
            char *r = gen_code(node->right, list);
            char *t = newTemp();
            ir_emit(list, IR_OR, l, r, t);
            return t;
        }

        case NODE_EQ: {
            char *l = gen_code(node->left, list);
            char *r = gen_code(node->right, list);
            char *t = newTemp();
            ir_emit(list, IR_EQ, l, r, t);
            return t;
        }

        case NODE_NEQ: {
            char *l = gen_code(node->left, list);
            char *r = gen_code(node->right, list);
            char *t = newTemp();
            ir_emit(list, IR_NEQ, l, r, t);
            return t;
        }

        case NODE_LT: {
            char *l = gen_code(node->left, list);
            char *r = gen_code(node->right, list);
            char *t = newTemp();
            ir_emit(list, IR_LT, l, r, t);
            return t;
        }

        case NODE_GT: {
            char *l = gen_code(node->left, list);
            char *r = gen_code(node->right, list);
            char *t = newTemp();
            ir_emit(list, IR_GT, l, r, t);
            return t;
        }

        case NODE_LE: {
            char *l = gen_code(node->left, list);
            char *r = gen_code(node->right, list);
            char *t = newTemp();
            ir_emit(list, IR_LE, l, r, t);
            return t;
        }

        case NODE_GE: {
            char *l = gen_code(node->left, list);
            char *r = gen_code(node->right, list);
            char *t = newTemp();
            ir_emit(list, IR_GE, l, r, t);
            return t;
        }


        case NODE_UMINUS: {
            char *val = gen_code(node->left, list);
            char *t = newTemp();
            ir_emit(list, IR_SUB, "0", val, t);
            return t;
        }

        case NODE_DECLARATION: {
            // Si hay inicialización, generarla
            if (node->right) {
                char *rhs = gen_code(node->right, list);
                ir_emit(list, IR_STORE, rhs, NULL, node->sym->name);
            }
            break;
        }

        case NODE_ASSIGN: {
            char *l = gen_code(node->left, list);
            char *r = gen_code(node->right, list);
            ir_emit(list, IR_STORE, l, r, node->sym->name);
            return node->sym->name;
        }

        case NODE_METHOD_CALL: {
            // Generar args
            char *args = gen_code(node->right, list);
            ir_emit(list, IR_CALL, node->sym->name, args, newTemp());
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
            // Etiqueta para inicio del método
            if (node->sym && node->sym->name) {
                ir_emit(list, IR_LABEL, NULL, NULL, node->sym->name);
            }
            // Cuerpo del método
            gen_code(node->right, list);
            break;
        }

        case NODE_IF: {
            char *cond = gen_code(node->left, list); // condición
            char *label_end = "hola";//newLabel();
            ir_emit(list, IR_IFZ, cond, NULL, label_end);
            gen_code(node->right, list); // cuerpo del if
            ir_emit(list, IR_LABEL, NULL, NULL, label_end);
            break;
        }

        case NODE_RETURN: {
            // no es un return void
            if (node->left != NULL){
                char *l = gen_code(node->left, list);
                ir_emit(list, IR_PRINT, l, NULL, NULL);
                return NULL;
            }
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



void ir_emit(IRList *list, IRInstr op, char *arg1, char *arg2, char *result) {
    // redimensionar si no hay lugar
    if (list->size >= list->capacity) {
        list->capacity = (list->capacity == 0) ? 4 : list->capacity * 2;
        list->codes = realloc(list->codes, list->capacity * sizeof(IRCode));
    }

    // agregar nueva instrucción
    IRCode *code = &list->codes[list->size++];
    code->op = op;
    code->arg1 = arg1 ? strdup(arg1) : NULL;
    code->arg2 = arg2 ? strdup(arg2) : NULL;
    code->result = result ? strdup(result) : NULL;
}



void ir_print(IRList *list) {
    for (int i = 0; i < list->size; i++) {
        IRCode *code = &list->codes[i];
        printf("%s", ir_names[code->op]);

        switch (code->op)
        {
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
            printf(" %s", code->arg1);
            printf(", %s", code->arg2);
            printf(", %s", code->result);
            break;
        case IR_STORE:
        case IR_LOAD:
        case IR_CALL:
        case IR_NOT:
            printf(" %s", code->arg1);
            printf(", %s", code->result);
            break;
        case IR_LABEL:
            printf(" %s", code->result);
            break;
        case IR_GOTO:
        case IR_IFZ:
        case IR_RETURN:
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
    for (int i = 0; i < list->size; i++) {
        free(list->codes[i].arg1);
        free(list->codes[i].arg2);
        free(list->codes[i].result);
    }
    free(list->codes);
    list->codes = NULL;
    list->size = 0;
    list->capacity = 0;
}