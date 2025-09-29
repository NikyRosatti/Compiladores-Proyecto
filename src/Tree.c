#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Tree.h"
#include "Symbol.h" 
#include "Stack.h"  


struct Tree;  /* forward declaration */
int semantic_error = 0;
int main_decl = 0; // chequear la existencia del metodo main
extern TypeStack typeStack;
ScopeStack scope_Stack;  // pila global de scopes


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
        case NODE_MOD: {
            int right = evaluate(node->right);
            if (right == 0) {
                printf("Error: Módulo por cero\n");
                return 0;
            }
            return evaluate(node->left) % right;
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

int has_return(Tree *n) {
                    if (!n) return 0;                 // nodo nulo
                    if (n->tipo == NODE_RETURN )       // encontramos un return
                        return 1;
                    // buscar en left y right recursivamente
                    return has_return(n->left) || has_return(n->right);
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
                } else if(evaluate(node->right) == 0) {
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
                SymbolType t = node->sym->type;
                if (strcmp(node->sym->name, "main") == 0) {main_decl = 1;} // Exactamente debe encontrar "main"
                pushType(&typeStack, t);
                check_types(node->right); // cuerpo del método
                popType(&typeStack);
                    
                if (!node->right )
                {
                    return TYPE_VOID;
                }  else {
                    if (!has_return(node)) {
                        printf("Error: el método '%s' debe tener una sentencia return\n", node->sym->name);
                        semantic_error = 1;
                        return TYPE_ERROR;
                    }
                    return TYPE_VOID;
                }
            }   

        case NODE_METHOD_CALL: {
            Symbol *method_sym = node->left->sym;
            if (!method_sym) {
                printf("Error: llamada a método no declarada\n");
                semantic_error = 1;
                return TYPE_ERROR;
            }

            if ( !method_sym->node) {
                printf("Error: símbolo de método '%s' no tiene nodo asociado\n",
                    method_sym ? method_sym->name : "(desconocido)");
                semantic_error = 1;
                return TYPE_ERROR;  // o lo que uses como valor de error
            }


            // Obtengo la referencia a la declaración del método
            Tree *method_decl = method_sym->node->left;         // NODE_METHOD_HEADER
            Tree *decl_args = method_decl->right;         // ARGS -> NODE_LIST encadenado
            Tree *call_args = node->right;                // ARGS pasados -> NODE_LIST encadenado

            Tree *d = decl_args ? decl_args->left : NULL;
            Tree *c = call_args ? call_args->left : NULL;

            // Recorremos las listas de parámetros y argumentos en paralelo
            while (d && c) {
                SymbolType t_decl = d->left->sym->type;
                SymbolType t_call;

                if (c->left->sym == NULL) {
                    // todavía no se resolvió → buscá el tipo con check_types
                    t_call = check_types(c->left);

                    if (t_call == TYPE_ERROR) {
                        printf("Error: expresión inválida en llamada a método\n");
                        semantic_error = 1;
                        return TYPE_ERROR;
                    }
                } else {
                    // ya está resuelto → uso directo
                    t_call = c->left->sym->type;
                }

                if (t_decl != t_call) {
                    printf("Error: parámetros con tipos distintos\n");
                    semantic_error = 1;
                    return TYPE_ERROR;
                }

                d = d->right;
                c = c->right;
            }

            // Si alguna lista todavía tiene elementos -> error de cantidad
            if (d || c) {
                printf("Error: cantidad de parámetros distintas\n");
                semantic_error = 1;
                return TYPE_ERROR;
            }

            return method_sym->type;
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

        case NODE_DECLARATION: {
            SymbolType var_type = node->sym ? node->sym->type : TYPE_ERROR;

            if (node->right) {  // solo chequea si hay inicialización
                SymbolType init_type = check_types(node->right);
                if (var_type != init_type) {
                    printf("Error: declaración con tipo incompatible en variable '%s' (esperado %d, encontrado %d)\n",
                            node->sym ? node->sym->name : "?", var_type, init_type);
                    semantic_error = 1;
                    return TYPE_ERROR;
                }
            }
            // si no hay inicialización, no pasa nada
            return var_type;
        }

        
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



void check_scopes(Tree *node) {
    if (!node) return;

    switch (node->tipo) {
        case NODE_PROGRAM:
            // Scope global
            pushScope(&scope_Stack, createTable());
            check_scopes(node->left);   // tipo o declaraciones globales
            check_scopes(node->right);  // resto del programa
            popScope(&scope_Stack);
            break;

        case NODE_BLOCK:
            pushScope(&scope_Stack, createTable());
            check_scopes(node->left);   // declaraciones / params
            check_scopes(node->right);  // cuerpo
            popScope(&scope_Stack);
            break;
        case NODE_METHOD:
            // Nuevo scope
            SymbolTable *current = peekScope(&scope_Stack);
            Symbol *sym; 
            if (lookupSymbol(current, node->sym->name)) {
                    printf("Error: redeclaración de metodo: '%s'\n", node->sym->name);
                    semantic_error = 1;
                } else {
                    sym = insertSymbol(current, node->sym->name, node->sym->type, node->sym->valor);
                }

            if (sym) {
                sym->node = node;
                node->sym = sym;  // opcional, por si querés que apunten al mismo
            }
            pushScope(&scope_Stack, createTable());
            check_scopes(node->left);   // declaraciones / params
            check_scopes(node->right);  // cuerpo
            popScope(&scope_Stack);
            break;

        case NODE_DECLARATION:
            if (node->sym) {
                SymbolTable *current = peekScope(&scope_Stack);
                if (lookupSymbol(current, node->sym->name)) {
                    printf("Error: redeclaración de '%s'\n", node->sym->name);
                    semantic_error = 1;
                } else {
                    insertSymbol(current, node->sym->name, node->sym->type, node->sym->valor);
                }
            }
            check_scopes(node->left);
            check_scopes(node->right);
            break;

        case NODE_ID:
            if (node->sym) {
                Symbol *s = lookupInScopes(&scope_Stack, node->sym->name);
                if (!s) {
                    printf("Error: variable '%s' no declarada\n", node->sym->name);
                    semantic_error = 1;
                } else {
                    // linkear el símbolo encontrado
                    node->sym = s;
                }
            }
            check_scopes(node->left);
            check_scopes(node->right);
            break;
        
        case NODE_METHOD_CALL:
            if (node->sym) {
                Symbol *s = lookupInScopes(&scope_Stack, node->sym->name);
                if (!s) {
                    printf("Error: llamada a método '%s' no declarado\n", node->sym->name);
                    semantic_error = 1;
                } else {
                    node->sym = s;                // linkear el método
                    node->left->sym = s;          // linkear el ID dentro del call
                }
            }
            check_scopes(node->left);
            check_scopes(node->right);
            break;
        
        case NODE_ASSIGN: 
            if (node->sym) {
                Symbol *s = lookupInScopes(&scope_Stack, node->sym->name);
                if (!s) {
                    printf("Error: llamada a variable '%s' no declarado\n", node->sym->name);
                    semantic_error = 1;
                } else {
                    node->sym = s;                // linkear la variable
                    
                }
            }
            check_scopes(node->left);
            check_scopes(node->right);
            break;



        default:
            check_scopes(node->left);
            check_scopes(node->right);
            break;
    }
}

