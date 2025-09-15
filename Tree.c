#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Tree.h"
#include "Symbol.h" 
#include "Stack.h"  

struct Tree;  /* forward declaration */
int semantic_error = 0;
extern TypeStack typeStack;

Tree* createNode(typeTree tipo, Symbol *sym, Tree *left, Tree *right) {
    Tree *n = malloc(sizeof(Tree));
    n->tipo = tipo;
    n->sym = sym;
    n->left = left;
    n->right = right;
    return n;
}

void printTree(Tree *n, int level) {
    if (!n) return;

    for (int i = 0; i < level; i++) printf("  ");

    const char *tipo = tipoToStr(n->tipo);

    if (n->sym) {
        printf("%s(Symbol: %s, type=%d, value=%d)\n",
                tipo,
                n->sym->name ? n->sym->name : "anon",
                n->sym->type,
                n->sym->valor.value);
    } else {
        printf("%s\n", tipo);
    }

    if (n->left) {
        for (int i = 0; i <= level; i++) printf("  ");
        printf("left:\n");
        printTree(n->left, level + 2);
    }
    if (n->right) {
        for (int i = 0; i <= level; i++) printf("  ");
        printf("right:\n");
        printTree(n->right, level + 2);
    }
}

const char* tipoToStr(typeTree t) {
    switch (t) {
        case NODE_CODE: return "CODE";
        case NODE_METHOD: return "METHOD";
        case NODE_DECLARATION: return "DECLARATION";
        case NODE_METHOD_HEADER: return "METHOD_HEADER";
        case NODE_METHOD_CALL: return "METHOD_CALL";
        case NODE_UMINUS: return "UMINUS";
        case NODE_IF: return "IF";
        case NODE_IF_ELSE: return "IF_ELSE";
        case NODE_WHILE: return "WHILE";
        case NODE_ASSIGN: return "=";
        case NODE_ID:    return "ID";
        case NODE_MUL:   return "*";
        case NODE_MOD:   return "%";
        case NODE_RES:   return "-";
        case NODE_DIV:   return "/";
        case NODE_PROGRAM: return "PROGRAM";
        case NODE_ARGS:  return "ARGS";
        case NODE_LIST:  return "LIST";
        case NODE_BLOCK: return "BLOCK";
        case NODE_RETURN: return "RETURN";
        case NODE_TRUE:  return "TRUE";
        case NODE_INT: return "INT";
        case NODE_FALSE: return "FALSE";
        case NODE_T_INT: return "T_INT";
        case NODE_T_BOOL: return "T_BOOL";
        case NODE_T_VOID: return "T_VOID";
        case NODE_OR:    return "OR";
        case NODE_AND:   return "AND";
        case NODE_NOT:   return "¬";
        case NODE_PARENS: return "()";
        case NODE_SUM:   return "+";
        case NODE_EQ:   return "==";
        case NODE_NEQ:   return "!=";
        case NODE_LE:   return "<=";
        case NODE_LT:   return "<";
        case NODE_GE:   return ">=";
        case NODE_GT:   return ">";
        default:         return "STRING";
    }
}

int evaluate(Tree *node) {
    if (!node) return 0;
    switch(node->tipo) {
        case NODE_INT: return node->sym->valor.value;
        case NODE_TRUE: return 1;
        case NODE_FALSE: return 0;

        case NODE_ID: return node->sym->valor.value;
        
        case NODE_SUM: return evaluate(node->left) + evaluate(node->right);
        case NODE_RES: return evaluate(node->left) - evaluate(node->right);
        case NODE_MUL: return evaluate(node->left) * evaluate(node->right);
        case NODE_DIV: 
            if (evaluate(node->right) == 0) {
                printf("Error: División por cero\n");
                return 0;
            } else {
                return evaluate(node->left) / evaluate(node->right);
            }
        case NODE_PARENS: return evaluate(node->left);  

        case NODE_OR:    return evaluate(node->left) || evaluate(node->right);
        case NODE_AND:   return evaluate(node->left) && evaluate(node->right);
        case NODE_NOT:   return !evaluate(node->left);
        case NODE_EQ:    return evaluate(node->left) == evaluate(node->right);
        case NODE_NEQ:   return evaluate(node->left) != evaluate(node->right);
        case NODE_LE:    return evaluate(node->left) <= evaluate(node->right);
        case NODE_LT:    return evaluate(node->left) <  evaluate(node->right);
        case NODE_GE:    return evaluate(node->left) >= evaluate(node->right);
        case NODE_GT:    return evaluate(node->left) >  evaluate(node->right);

        // Agregá más operadores según tu gramática
        default: return 0;
    }
}

void execute(Tree *node) {
    if (!node) return;

    switch(node->tipo) {
        case NODE_ASSIGN:
            node->sym->valor.value = evaluate(node->left);
            break;

        case NODE_LIST:
        case NODE_BLOCK:
        case NODE_PROGRAM:
        case NODE_METHOD:
        case NODE_CODE:
            execute(node->left);
            execute(node->right);
            break;

        default:
            // Otros nodos no hacen nada
            break;
    }
}

SymbolType check_types(Tree *node){
    if (!node) return TYPE_VOID;  // nodo vacío siempre error

    switch(node->tipo) {

        case NODE_INT: return TYPE_INT;
        case NODE_TRUE:
        case NODE_FALSE: return TYPE_BOOL;

        case NODE_ID:{
                if (!node->sym) {
                    semantic_error = 1;
                    return TYPE_ERROR;
                }
                return node->sym->type; 
            }

        case NODE_ASSIGN: {
                SymbolType var_type = node->sym ? node->sym->type : TYPE_ERROR;
                SymbolType expr_type = check_types(node->left);
                if (var_type != expr_type) {
                    printf("Error: asignación incompatible\n");
                    semantic_error = 1;
                    return TYPE_ERROR;
                }
                return var_type;
            }

        case NODE_SUM:
        case NODE_RES:
        case NODE_MUL: {
                SymbolType left = check_types(node->left);
                SymbolType right = check_types(node->right);
                if (left != TYPE_INT || right != TYPE_INT) {
                    printf("Error: operador aritmético espera enteros\n");
                    semantic_error = 1;
                    return TYPE_ERROR;
                }
                return TYPE_INT;
            }
        case NODE_MOD:
        case NODE_DIV: {
                SymbolType left = check_types(node->left);
                SymbolType right = check_types(node->right);
                if (left != TYPE_INT || right != TYPE_INT) {
                    printf("Error: operador aritmético espera enteros\n");
                    semantic_error = 1;
                    return TYPE_ERROR;
                } else if(evaluate(node) == 0) {
                    semantic_error = 1;
                    return TYPE_ERROR;
                }
                return TYPE_INT;
            }

        case NODE_LE:
        case NODE_LT:
        case NODE_GE:
        case NODE_GT: {
                SymbolType left = check_types(node->left);
                SymbolType right = check_types(node->right);
                if (left != TYPE_INT || right != TYPE_INT) {
                    printf("Error: operador relacional espera enteros\n");
                    semantic_error = 1;
                    return TYPE_ERROR;
                }
                return TYPE_BOOL;
            }

        case NODE_EQ:
        case NODE_NEQ: {
            SymbolType left = check_types(node->left);
            SymbolType right = check_types(node->right);
            if (left != right) {
                printf("Error: comparación de tipos incompatibles\n");
                semantic_error = 1;
                return TYPE_ERROR;
            }
            return TYPE_BOOL;
        }

        case NODE_OR:
        case NODE_AND: {
                SymbolType left = check_types(node->left);
                SymbolType right = check_types(node->right);
                if (left != TYPE_BOOL || right != TYPE_BOOL) {
                    printf("Error: operador lógico espera booleanos\n");
                    semantic_error = 1;
                    return TYPE_ERROR;
                }
                return TYPE_BOOL;
            }

        case NODE_NOT: {
                SymbolType left = check_types(node->left);
                if (left != TYPE_BOOL) {
                    printf("Error: operador NOT espera booleano\n");
                    semantic_error = 1;
                    return TYPE_ERROR;
                }
                return TYPE_BOOL;
            }

        case NODE_PARENS:
            return check_types(node->left);

        case NODE_LIST: {
                check_types(node->left);
                check_types(node->right);
                return TYPE_VOID;
            }

        case NODE_BLOCK: {
                check_types(node->left);
                check_types(node->right);
                return TYPE_VOID;
            }

        case NODE_CODE:
            check_types(node->left);
            check_types(node->right);
            return TYPE_VOID;

        case NODE_PROGRAM: {
                // push tipo del programa en la pila
                SymbolType t;
                if (node->left->tipo == NODE_T_INT) t = TYPE_INT;
                else if (node->left->tipo == NODE_T_BOOL) t = TYPE_BOOL;
                else t = TYPE_VOID;

                pushType(&typeStack, t);
                check_types(node->right);
                popType(&typeStack);
                return TYPE_VOID;
            }

        case NODE_RETURN: {
                SymbolType expected = peekType(&typeStack);
                SymbolType got = node->left ? check_types(node->left) : TYPE_VOID;
                if (expected != got) {
                    printf("Error semántico: return de tipo %d, esperado %d\n", got, expected);
                    semantic_error = 1;
                    return TYPE_ERROR;
                }
                return got;
            }

        case NODE_METHOD: {
                // push tipo del método en la pila
                SymbolType t;
                if (node->left->tipo == NODE_T_INT) t = TYPE_INT;
                else if (node->left->tipo == NODE_T_BOOL) t = TYPE_BOOL;
                else t = TYPE_VOID;

                pushType(&typeStack, t);
                check_types(node->right); // cuerpo del método
                popType(&typeStack);
                return TYPE_VOID;
            }   

            case NODE_METHOD_CALL: {
                Symbol *method_sym = node->sym;
                if (!method_sym) {
                    printf("Error: llamada a método no declarado\n");
                    semantic_error = 1;
                    return TYPE_ERROR;
                }
                // chequeo de tipos de argumentos
                struct Tree *ref = node->left->sym->node; //referencia a la decl del metodo

                struct Tree *decl_args = ref->right;
                struct Tree *call_args = node->right;
                
                struct Tree *decl_aux;
                struct Tree *call_aux;
                
                while (1) {
                    decl_aux = decl_args->left;
                    call_aux = call_args->left;

                    if (decl_aux == NULL && call_aux == NULL)
                    {
                        break;
                    }
                    if (decl_aux != NULL && call_aux == NULL || decl_aux == NULL && call_aux != NULL)
                    {
                        printf("Error: Cantidad de parametros distintas\n");
                        semantic_error = 1;
                        return TYPE_ERROR;
                    }
                    if (check_types(call_aux) == check_types(decl_aux))
                    {
                        decl_args = decl_args->right;
                        call_args = call_args->right;
                    } else {
                        printf("Error: Parametros con tipos distintos\n");
                        semantic_error = 1;
                        return TYPE_ERROR;
                    }
                    
                    
                }

                // Aquí podrías agregar chequeo de tipos de argumentos según la firma del método
                return method_sym->type; // tipo de retorno del método
            }

        case NODE_UMINUS: {
                SymbolType expr_type = check_types(node->left);
                if (expr_type != TYPE_INT) {
                    printf("Error: operador unario menos espera entero\n");
                    semantic_error = 1;
                    return TYPE_ERROR;
                }
                return TYPE_INT;
            }

        case NODE_IF: {
                    SymbolType cond_type = check_types(node->left);
                    if (cond_type != TYPE_BOOL) {
                        printf("Error: condición de IF debe ser booleano\n");
                        semantic_error = 1;
                        return TYPE_ERROR;
                    }
                    check_types(node->right); // cuerpo del if
                    return TYPE_VOID;
            }

        case NODE_IF_ELSE: {
                SymbolType cond_type = check_types(node->left);
                if (cond_type != TYPE_BOOL) {
                    printf("Error: condición de IF debe ser booleano\n");
                    semantic_error = 1;
                    return TYPE_ERROR;
                }
                // cuerpo del if
                check_types(node->right->left);
                // cuerpo del else
                check_types(node->right->right);
                return TYPE_VOID;
            }

        case NODE_WHILE: {
                SymbolType cond_type = check_types(node->left);
                if (cond_type != TYPE_BOOL) {
                    printf("Error: condición de WHILE debe ser booleano\n");
                    semantic_error = 1;
                    return TYPE_ERROR;
                }
                check_types(node->right); // cuerpo del while
                return TYPE_VOID;
            }

        case NODE_DECLARATION:
            // chequeo condicional y bloques
            check_types(node->left);  // condición o tipo
            check_types(node->right); // cuerpo o inicialización
            return TYPE_VOID;
        
        case NODE_ARGS:
            // chequeo de argumentos en llamadas
            check_types(node->left);
            check_types(node->right);
            return TYPE_VOID;
        
        case NODE_METHOD_HEADER:
            // chequeo de la cabecera del método
            check_types(node->left);  // tipo de retorno
            check_types(node->right); // parámetros
            return TYPE_VOID;
        
        case NODE_T_INT: return TYPE_INT;
        
        case NODE_T_BOOL: return TYPE_BOOL;
        
        case NODE_T_VOID: return TYPE_VOID;

        default: {
                printf("Error: nodo de tipo desconocido en check_types\n");
                semantic_error = 1;
                return TYPE_ERROR;
            }
    }
}