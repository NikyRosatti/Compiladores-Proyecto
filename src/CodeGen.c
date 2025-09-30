#include <stdio.h>
#include <stdlib.h>
#include "Tree.h"
#include "Symbol.h"

static int tempCount = 0;
static int labelCount = 0;
static int ifCount = 0;
static int whileCount = 0;
static int typeBlock = 0; // 1-if, 2-while

char* newTemp() {
    char *buf = malloc(16);
    sprintf(buf, "t%d", tempCount++);
    return buf;
}

char* newLabel() {
    char *buf = malloc(16);
    sprintf(buf, "L%d", labelCount++);
    return buf;
}

void printIndent(int level) {
    for(int i=0;i<level;i++) printf("\t");
}

// -----------------------
// Generación de expresiones
// -----------------------
char* genExpr(Tree *node, int level) {
    if (!node) return NULL;

    switch(node->tipo) {
        case NODE_INT: {
            char *t = newTemp();
            printIndent(level);
            printf("movl $%d, %s\n", node->sym->valor.value, t);
            return t;
        }
        case NODE_TRUE:
        case NODE_FALSE: {
            char *t = newTemp();
            int val = (node->tipo == NODE_TRUE) ? 1 : 0;
            printIndent(level);
            printf("movl $%d, %s\n", val, t);
            return t;
        }
        case NODE_ID:
            return node->sym->name;

        case NODE_SUM:
        case NODE_RES:
        case NODE_MUL:
        case NODE_DIV:
        case NODE_MOD: {
            char *lhs = genExpr(node->left, level);
            char *rhs = genExpr(node->right, level);
            char *t = newTemp();
            printIndent(level);
            printf("movl %s, %s\n", lhs, t);
            printIndent(level);
            switch(node->tipo) {
                case NODE_SUM:  printf("add %s, %s\n", rhs, t); break;
                case NODE_RES:  printf("sub %s, %s\n", rhs, t); break;
                case NODE_MUL:  printf("imul %s, %s\n", rhs, t); break;
                case NODE_DIV:  // división: eax / rhs
                    printIndent(level);
                    printf("movl %s, eax\n", lhs);
                    printIndent(level);
                    printf("cltd\n"); // signo extendido a edx
                    printIndent(level);
                    printf("idiv %s\n", rhs);
                    printIndent(level);
                    printf("movl eax, %s\n", t);
                    break;
                case NODE_MOD:
                    printIndent(level);
                    printf("movl %s, eax\n", lhs);
                    printIndent(level);
                    printf("cltd\n");
                    printIndent(level);
                    printf("idiv %s\n", rhs);
                    printIndent(level);
                    printf("movl edx, %s\n", t);
                    break;
                default: break;
            }
            return t;
        }

        case NODE_EQ:
        case NODE_NEQ:
        case NODE_LT:
        case NODE_GT:
        case NODE_LE:
        case NODE_GE: {
            char *lhs = genExpr(node->left, level);
            char *rhs = genExpr(node->right, level);
            char *t = newTemp();        // <-- temporal donde guardamos 0/1
            char *trueLabel = newLabel();
            char *endLabel = newLabel();

            // Inicializamos t = 0
            printIndent(level);
            printf("movl $0, %s\n", t);

            // Comparamos
            printIndent(level);
            printf("cmp %s, %s\n", lhs, rhs);

            // Saltamos si la comparación es verdadera
            printIndent(level);
            switch(node->tipo) {
                case NODE_EQ:  printf("je %s\n", trueLabel); break;
                case NODE_NEQ: printf("jne %s\n", trueLabel); break;
                case NODE_LT:  printf("jl %s\n", trueLabel); break;
                case NODE_LE:  printf("jle %s\n", trueLabel); break;
                case NODE_GT:  printf("jg %s\n", trueLabel); break;
                case NODE_GE:  printf("jge %s\n", trueLabel); break;
                default: break;
            }

            // Fin: t sigue siendo 0 si no se cumple
            printIndent(level);
            printf("jmp %s\n", endLabel);

            // Verdadero: t = 1
            printIndent(level);
            printf("%s:\n", trueLabel);
            printIndent(level);
            printf("movl $1, %s\n", t);

            // End
            printIndent(level);
            printf("%s:\n", endLabel);

            return t;
        }


        case NODE_NOT:
        case NODE_UMINUS: {
            char *expr = genExpr(node->left, level);
            char *t = newTemp();
            printIndent(level);
            printf("movl %s, %s\n", expr, t);
            printIndent(level);
            if (node->tipo == NODE_NOT) printf("not %s\n", t);
            else printf("neg %s\n", t);
            return t;
        }

        case NODE_AND:
        case NODE_OR: {
            char *lhs = genExpr(node->left, level);
            char *rhs = genExpr(node->right, level);
            char *t = newTemp();
            printIndent(level);
            printf("movl %s, %s\n", lhs, t);
            printIndent(level);
            if (node->tipo == NODE_AND) printf("and %s, %s\n", rhs, t);
            else printf("or %s, %s\n", rhs, t);
            return t;
        }

        case NODE_PARENS:
            return genExpr(node->left, level);

        case NODE_METHOD_CALL: {
            Tree *arg = node->right;
            // Apilar argumentos
            while(arg) {
                char *a = genExpr(arg->left, level);
                if (a) {
                    printIndent(level);
                    printf("push %s\n", a);
                }
                arg = arg->right;
            }
            printIndent(level);
            printf("call %s\n", node->left->sym->name);
            char *t = newTemp();
            printIndent(level);
            printf("movl eax, %s\n", t);
            return t;
        }

        case NODE_LIST: {
            genExpr(node->left, level);
            genExpr(node->right, level);
            return NULL;
        }

        default:
            fprintf(stderr, "genExpr: nodo no implementado %d\n", node->tipo);
            return NULL;
    }
}

// -----------------------
// Generación de código general
// -----------------------
void genCode(Tree *node, int level) {
    if (!node) return;

    switch(node->tipo) {

        case NODE_CODE:
        case NODE_LIST:
        case NODE_BLOCK:
        case NODE_PROGRAM:
            genCode(node->left, level);
            genCode(node->right, level);
            break;

        case NODE_METHOD:
            if (node->right == NULL) break; // externa
            printIndent(level);
            printf("%s:\n", node->sym->name);
            genCode(node->right, level+1);
            printIndent(level);
            printf("f%s:\n", node->sym->name);
            break;

        case NODE_DECLARATION:
            printIndent(level);
            printf("resb %s, 4\n", node->sym->name); // se reservan 4 bytes
            if (node->right) {
                char *rhs = genExpr(node->right, level);
                printIndent(level);
                printf("movl %s, %s\n", rhs, node->sym->name);
            }
            break;

        case NODE_ASSIGN: {
            char *rhs = genExpr(node->left, level);
            printIndent(level);
            printf("movl %s, %s\n", rhs, node->sym->name);
            break;
        }

        case NODE_RETURN:
            if (node->left) {
                char *val = genExpr(node->left, level);
                printIndent(level);
                printf("movl %s, eax\n", val);
            }
            printIndent(level);
            printf("ret\n");
            break;

        case NODE_IF: {
            int thisIf = ifCount++;
            char *endLabel = malloc(16);
            sprintf(endLabel, "IF_%d_END", thisIf);

            char *cond = genExpr(node->left, level);
            printIndent(level);
            printf("cmp $0, %s\n", cond);
            printIndent(level);
            printf("je %s\n", endLabel);

            genCode(node->right, level+1);

            printIndent(level);
            printf("%s:\n", endLabel);
            break;
        }

        case NODE_IF_ELSE: {
            int thisIf = ifCount++;
            char *elseLabel = malloc(16);
            char *endLabel = malloc(16);
            sprintf(elseLabel, "IF_%d_ELSE", thisIf);
            sprintf(endLabel, "IF_%d_END", thisIf);

            char *cond = genExpr(node->left, level);
            printIndent(level);
            printf("cmp $0, %s\n", cond);
            printIndent(level);
            printf("je %s\n", elseLabel);

            genCode(node->right->left, level+1);  // then
            printIndent(level);
            printf("jmp %s\n", endLabel);

            printIndent(level);
            printf("%s:\n", elseLabel);
            genCode(node->right->right, level+1); // else

            printIndent(level);
            printf("%s:\n", endLabel);
            break;
        }

        case NODE_WHILE: {
            int thisWhile = whileCount++;
            char *startLabel = malloc(16);
            char *endLabel = malloc(16);
            sprintf(startLabel, "WHILE_%d_START", thisWhile);
            sprintf(endLabel, "WHILE_%d_END", thisWhile);

            printIndent(level);
            printf("%s:\n", startLabel);

            char *cond = genExpr(node->left, level+1);
            printIndent(level);
            printf("cmp $0, %s\n", cond);
            printIndent(level);
            printf("je %s\n", endLabel);

            genCode(node->right, level+1);

            printIndent(level);
            printf("jmp %s\n", startLabel);

            printIndent(level);
            printf("%s:\n", endLabel);
            break;
        }

        case NODE_METHOD_CALL:
            genExpr(node, level);
            break;

        default:
            genCode(node->left, level+1);
            genCode(node->right, level+1);
            break;
    }
}

// declarar métodos externos primero
void declareExternals(Tree *node, int level) {
    if (!node) return;

    switch(node->tipo) {
        case NODE_METHOD:
            if (node->right == NULL) {
                printf(".extern %s\n", node->sym->name);
            }
            break;
        default:
            declareExternals(node->left, level);
            declareExternals(node->right, level);
            break;
    }
}
