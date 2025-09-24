#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Tree.h"
#include "Symbol.h"
#include "Stack.h"

int semantic_error = 0;
extern TypeStack typeStack;
extern ScopeStack scopeStack;

// ==================== CREACIÓN DE NODOS ====================
Tree* createNode(typeTree tipo, Symbol *sym, Tree *left, Tree *right) {
    Tree *n = malloc(sizeof(Tree));
    if (!n) { fprintf(stderr,"Error: sin memoria\n"); exit(1); }
    n->tipo = tipo;
    n->sym = sym;
    n->left = left;
    n->right = right;
    return n;
}

// Helpers para crear símbolos constantes
Symbol* makeIntSymbol(int val) {
    Symbol *s = malloc(sizeof(Symbol));
    if(!s){ fprintf(stderr,"Error: sin memoria\n"); exit(1);}
    s->name = NULL;
    s->type = TYPE_INT;
    s->valor.value = val;
    return s;
}

Symbol* makeBoolSymbol(int val) {
    Symbol *s = malloc(sizeof(Symbol));
    if(!s){ fprintf(stderr,"Error: sin memoria\n"); exit(1);}
    s->name = NULL;
    s->type = TYPE_BOOL;
    s->valor.value = val;
    return s;
}

// ==================== IMPRESIÓN DE ÁRBOLES ====================
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
        for(int i=0;i<=level;i++) printf("  ");
        printf("left:\n");
        printTree(n->left, level+2);
    }
    if (n->right) {
        for(int i=0;i<=level;i++) printf("  ");
        printf("right:\n");
        printTree(n->right, level+2);
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
        default:         return "UNKNOWN";
    }
}

// ==================== EVALUACIÓN ====================
int evaluate(Tree *node) {
    if (!node) return 0;

    switch(node->tipo) {
        case NODE_INT: return node->sym->valor.value;
        case NODE_TRUE: return 1;
        case NODE_FALSE: return 0;
        case NODE_ID: {
            Symbol *s = lookupInScopes(&scopeStack, node->sym->name);
            if (!s) { 
                printf("Error: variable %s no declarada\n", node->sym->name);
                semantic_error = 1;
                return 0;
            }
            return s->valor.value;
        }
        case NODE_SUM: return evaluate(node->left) + evaluate(node->right);
        case NODE_RES: return evaluate(node->left) - evaluate(node->right);
        case NODE_MUL: return evaluate(node->left) * evaluate(node->right);
        case NODE_DIV: { int r = evaluate(node->right); return r==0?0:evaluate(node->left)/r; }
        case NODE_MOD: { int r = evaluate(node->right); return r==0?0:evaluate(node->left)%r; }
        case NODE_PARENS: return evaluate(node->left);
        case NODE_OR: return evaluate(node->left)||evaluate(node->right);
        case NODE_AND: return evaluate(node->left)&&evaluate(node->right);
        case NODE_NOT: return !evaluate(node->left);
        case NODE_EQ: return evaluate(node->left)==evaluate(node->right);
        case NODE_NEQ: return evaluate(node->left)!=evaluate(node->right);
        case NODE_LE: return evaluate(node->left)<=evaluate(node->right);
        case NODE_LT: return evaluate(node->left)<evaluate(node->right);
        case NODE_GE: return evaluate(node->left)>=evaluate(node->right);
        case NODE_GT: return evaluate(node->left)>evaluate(node->right);
        default: return 0;
    }
}

// ==================== EJECUCIÓN ====================
void execute(Tree *node) {
    if(!node) return;

    switch(node->tipo) {
        case NODE_ASSIGN: {
            Symbol *s = lookupInScopes(&scopeStack, node->sym->name);
            if(!s) { printf("Error: variable %s no declarada\n", node->sym->name); return; }
            s->valor.value = evaluate(node->left);
            break;
        }

        case NODE_BLOCK:
        case NODE_LIST:
            pushScope(&scopeStack, createTable());
            if(node->left) execute(node->left);
            if(node->right) execute(node->right);
            popScope(&scopeStack);
            break;

        case NODE_METHOD:
            pushScope(&scopeStack, createTable());
            if(node->left) addParamsToScope(node->left);
            if(node->right) execute(node->right);
            popScope(&scopeStack);
            break;

        case NODE_PROGRAM:
            if(node->left) execute(node->left);
            if(node->right) execute(node->right);
            break;

        default:
            if(node->left) execute(node->left);
            if(node->right) execute(node->right);
            break;
    }
}

// ==================== CHECK TYPES ====================
SymbolType check_types(Tree *node) {
    if (!node) return TYPE_VOID;

    switch(node->tipo) {
        case NODE_INT: return TYPE_INT;
        case NODE_TRUE:
        case NODE_FALSE: return TYPE_BOOL;

        case NODE_ID: {
            Symbol *s = lookupInScopes(&scopeStack, node->sym->name);
            if(!s) { 
                printf("Error semántico: variable %s no declarada\n", node->sym->name);
                semantic_error = 1; 
                return TYPE_ERROR;
            }
            return s->type;
        }

        case NODE_ASSIGN: {
            SymbolType t = check_types(node->left);
            Symbol *s = lookupInScopes(&scopeStack, node->sym->name);
            if(!s) { 
                printf("Error semántico: variable %s no declarada\n", node->sym->name);
                semantic_error = 1;
                return TYPE_ERROR;
            }
            if(t != s->type){
                printf("Error semántico: asignación de tipo incorrecto a %s\n", node->sym->name);
                semantic_error=1;
                return TYPE_ERROR;
            }
            return s->type;
        }

        case NODE_DECLARATION:
            if(node->sym) {
                SymbolTable *current = peekScope(&scopeStack);
                addSymbol(current, node->sym);
                printf("[check_types] Declarando %s en scope %d\n", node->sym->name, scopeStack.top-1);
            }
            return node->sym ? node->sym->type : TYPE_ERROR;

        case NODE_METHOD:
            // Primero agregamos el método al scope global
            if(node->sym) {
                addSymbol(scopeStack.arr[0], node->sym); // scope 0 = global
                printf("[check_types] Método %s agregado al global\n", node->sym->name);
            }

            // Abrimos scope propio del método
            pushScope(&scopeStack, createTable());
            if(node->left) addParamsToScope(node->left); // parámetros solo aquí
            if(node->right) check_types(node->right);   // bloque del método
            popScope(&scopeStack);
            return TYPE_VOID;

        case NODE_BLOCK:
        case NODE_LIST:
            pushScope(&scopeStack, createTable());
            if(node->left) check_types(node->left);
            if(node->right) check_types(node->right);
            popScope(&scopeStack);
            return TYPE_VOID;

        case NODE_PROGRAM:
            // El global ya existe en scopeStack.arr[0]
            if(node->left) check_types(node->left);
            if(node->right) check_types(node->right);
            return TYPE_VOID;

        default:
            if(node->left) check_types(node->left);
            if(node->right) check_types(node->right);
            return TYPE_VOID;
    }
}

// ==================== RECORRIDO DE PARÁMETROS ====================
// Agrega parámetros al scope actual (ya debe existir)
void addParamsToScope(Tree *header) {
    if (!header || header->tipo != NODE_METHOD_HEADER) return;
    Tree *args = header->right;
    addParamsRecursive(args);
}

void addParamsRecursive(Tree *params) {
    if (!params) return;

    SymbolTable *currentScope = peekScope(&scopeStack);
    if (!currentScope) {
        printf("Error: no hay scope activo para agregar parámetros\n");
        semantic_error = 1;
        return;
    }

    if (params->tipo == NODE_DECLARATION && params->sym) {
        // Hacemos una copia del símbolo para este scope
        Symbol *s = malloc(sizeof(Symbol));
        *s = *(params->sym);  // copia bit a bit
        if (params->sym->name)
            s->name = strdup(params->sym->name);
        addSymbol(currentScope, s);
        printf("[addParamsToScope] Insertando parámetro %s en scope %d\n",
                s->name, scopeStack.top);
    }

    addParamsRecursive(params->left);
    addParamsRecursive(params->right);
}

// Busca solo en el scope actual
Symbol* lookupCurrentScope(ScopeStack *stack, const char *name) {
    if(stack->top <= 0) return NULL;
    return lookupSymbol(stack->arr[stack->top-1], name);
}