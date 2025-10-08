#include "Assembler.h"
#include <stdio.h>
#include <stdlib.h>
#include <intermediate.h>
#include <globals.h>

extern SymbolNode *decl_vars;

// Declaración de función helper interna (solo visible en este archivo)
static void calculate_offsets_helper(Tree *node, int *current_offset, Symbol *current_method);


/**
 * Calcula los offsets de parámetros y variables locales para cada método.
 * 
 * VARIABLES GLOBALES:
 * - No tienen offset (se acceden por nombre/label en la sección .data o .bss)
 * - Se marcan con is_global = 1
 * 
 * BLOQUES ANIDADOS (if, while, for):
 * - Las variables declaradas en bloques anidados continúan el offset del scope del método
 * - Comparten el mismo stack frame que el método padre
 * 
 * LAYOUT DEL STACK (para métodos):
 *   16(%rbp)    <- parámetro 7 (si existe)
 *   8(%rbp)     <- return address
 *   0(%rbp)     <- previous %rbp
 *   -8(%rbp)    <- parámetro 1 (guardado desde %rdi)
 *   -16(%rbp)   <- parámetro 2 (guardado desde %rsi)
 *   ...
 *   -X(%rbp)    <- variables locales del método
 *   -Y(%rbp)    <- variables de bloques anidados (if, while, etc.)
 */
void calculate_offsets(Tree *node) {
    calculate_offsets_helper(node, NULL, NULL);
}

/**
 * Helper recursivo que mantiene contexto del método actual y offset actual
 */
static void calculate_offsets_helper(Tree *node, int *current_offset, Symbol *current_method) {
    if (!node) return;

    switch (node->tipo) {
        case NODE_CODE:
            // Marcar declaraciones globales
            mark_globals(node->left);
            // Procesar el resto del código (métodos)
            calculate_offsets_helper(node->left, NULL, NULL);
            calculate_offsets_helper(node->right, NULL, NULL);
            break;

        case NODE_METHOD: {
            int param_count = 0;
            int local_count = 0;
            
            // Obtengo la declaración del método
            Tree *method_decl = node->left;                             // NODE_METHOD_HEADER
            Tree *args_node = method_decl ? method_decl->right : NULL;  // ARGS (puede estar vacío)
            Tree *param_list = (args_node && args_node->left) ? args_node->left : NULL;  // Primer LIST (NULL si no hay parámetros)
            
            // Los primeros 6 parámetros: guardados en stack local (negativos)
            int param_offset = -8;  // Comienza en -8(%rbp)
            int stack_param_offset = 16;  // Para parámetros 7+ en adelante

            while (param_list) {
                // El parámetro actual está en param_list->left
                Tree *param_decl = param_list->left;
                
                if (param_decl && param_decl->sym) {
                    Symbol *param_sym = param_decl->sym;
                    param_sym->is_param = 1;
                    param_sym->is_global = 0;
                    param_sym->param_index = param_count;
                    
                    // Los primeros 6 van a posiciones negativas (desde registros)
                    if (param_count < 6) {
                        param_sym->offset = param_offset;
                        param_offset -= 8;
                    } else {
                        // Del 7mo en adelante ya están en el stack (positivos)
                        param_sym->offset = stack_param_offset;
                        stack_param_offset += 8;
                    }
                    
                    param_count++;
                    
                    #ifdef DEBUG_OFFSETS
                    /*printf("  Parámetro %d '%s': offset %d (%s)\n", 
                           param_sym->param_index,
                           param_sym->name, 
                           param_sym->offset,
                           param_sym->param_index < 6 ? "registro" : "stack");*/
                    #endif
                }
                
                // Siguiente parámetro en la lista
                param_list = param_list->right;
            }
            
            // Inicializar el offset para variables locales
            int method_offset = param_offset;  // Continuar desde donde quedaron los params
            
            // Recorrer el cuerpo del método asignando offsets
            // Pasamos el offset actual y el símbolo del método como contexto
            calculate_offsets_helper(node->right, &method_offset, node->sym);
            
            // Calcular el total de variables locales
            local_count = (param_offset - method_offset) / 8;
            
            // Guardar información en el símbolo del método
            if (node->sym) {
                node->sym->param_count = param_count;
                node->sym->local_count = local_count;
                
                // Espacio para parámetros que vienen por registro (primeros 6) + locales
                int params_from_regs = (param_count > 6) ? 6 : param_count;
                node->sym->total_stack_space = (params_from_regs + local_count) * 8;
                
                #ifdef DEBUG_OFFSETS
                /*printf("Método '%s': %d parámetros (%d por registro, %d por stack), %d locales, stack space: %d bytes\n",
                       node->sym->name, param_count, 
                       (param_count > 6) ? 6 : param_count,
                       (param_count > 6) ? param_count - 6 : 0,
                       local_count, node->sym->total_stack_space);*/
                #endif
            }
            break;
        }

        case NODE_BLOCK: {
            // Los bloques anidados (if, while, for) comparten el stack frame del método
            // Solo pasamos el offset actual para que continúe
            if (current_offset) {
                // Procesar declaraciones del bloque
                assign_block_locals(node->left, current_offset);
            }
            
            // Continuar con el resto del bloque
            calculate_offsets_helper(node->left, current_offset, current_method);
            calculate_offsets_helper(node->right, current_offset, current_method);
            break;
        }

        case NODE_DECLARATION: {
            // Si estamos dentro de un método y es una variable local
            if (current_offset && node->sym && !node->sym->is_param && !node->sym->is_global) {
                if (node->sym->offset == 0) {  // Solo si no tiene offset asignado
                    node->sym->offset = *current_offset;
                    *current_offset -= 8;
                    
                    #ifdef DEBUG_OFFSETS
                    /*printf("  Variable local '%s': offset %d\n", 
                           node->sym->name, node->sym->offset);*/
                    #endif
                }
            }
            
            calculate_offsets_helper(node->left, current_offset, current_method);
            calculate_offsets_helper(node->right, current_offset, current_method);
            break;
        }

        default:
            calculate_offsets_helper(node->left, current_offset, current_method);
            calculate_offsets_helper(node->right, current_offset, current_method);
            break;
    }
}

/**
 * Marca todas las declaraciones globales (fuera de métodos)
 */
void mark_globals(Tree *node) {
    if (!node) return;

    if (node->tipo == NODE_DECLARATION && node->sym) {
        node->sym->is_global = 1;
        node->sym->offset = 0;  // Las globales no usan offset, usan labels
        
        #ifdef DEBUG_OFFSETS
        //printf("Variable global '%s' (sin offset, usa label)\n", node->sym->name);
        #endif
    }
    
    // Solo marcar en el nivel actual, no entrar en métodos
    if (node->tipo != NODE_METHOD) {
        mark_globals(node->left);
        mark_globals(node->right);
    }
}

/**
 * Asigna offsets a variables locales dentro de un bloque
 */
void assign_block_locals(Tree *node, int *offset) {
    if (!node || !offset) return;

    if (node->tipo == NODE_DECLARATION && node->sym) {
        if (!node->sym->is_param && !node->sym->is_global && node->sym->offset == 0) {
            node->sym->offset = *offset;
            *offset -= 8;
            
            #ifdef DEBUG_OFFSETS
            /*printf("  Variable de bloque '%s': offset %d\n", 
                   node->sym->name, node->sym->offset);*/
            #endif
        }
    }
    
    assign_block_locals(node->left, offset);
    assign_block_locals(node->right, offset);
}

// funcion principal
void generateAssembly(IRList *irlist) {


    // primero recorremos variables globales
    collect_globals(irlist);

    // secciones de declaracion e inicializacion de variables
    printf("    .data\n");
    print_globals_data(decl_vars);
    printf("    .bss\n");
    print_globals_bss(decl_vars);

    // seccion text
    printf("    .text\n");
    printf("    .globl main\n");
    printf("    push %%rbp\n");
    printf("    mov %%rsp, %%rbp\n");

    // Iterar sobre todas las instrucciones
    for (int i = 0; i < irlist->size; i++) {
        IRCode *inst = &irlist->codes[i];
        generateInstruction(inst);
    }

    // Reservar espacio local si es necesario (por ahora fijo)
    //printf("    sub $128, %%rsp\n\n");

    // Epílogo
    printf("\n    leave\n");
    printf("    ret\n");
}

// Recorre la lista de IR para recolectar variables globales
void collect_globals(IRList *irlist) {
    if (!irlist) return;

    int in_method = 0;  // 0 = global, 1 = dentro de un método

    for (int i = 0; i < irlist->size; i++) {
        IRCode *inst = &irlist->codes[i];
        if (!inst) continue;

        switch (inst->op) {
            case IR_METHOD:
                in_method = 1;
                break;
            case IR_FMETHOD:
                in_method = 0;
                break;
            default:
                // Solo variables globales fuera de métodos
                if (in_method == 0 && inst->result && inst->result->is_global) {
                    add_decl(&decl_vars, inst->result);
                }
                break;
        }
    }
}



// =============================
// Implementación helpers
// =============================
void generateInstruction(IRCode *inst) {
    switch (inst->op) {
        case IR_LOAD: break;
        case IR_METH_EXT: break;
        case IR_PARAM: break;
        ///// estos de arriba no tienen instrucciones en Assembly
        case IR_STORE: generateAssign(inst); break;
        case IR_ADD: generateBinaryOp(inst, "add"); break;
        case IR_SUB: generateBinaryOp(inst, "sub"); break;
        case IR_MUL: generateBinaryOp(inst, "imul"); break;
        case IR_DIV: generateBinaryOp(inst, "idiv"); break;

        case IR_MOD:break;

        case IR_CALL: generateCall(inst); break;
        // Operadores de comparacion
        case IR_EQ:  generateCompare(inst, "sete");  break;
        case IR_NEQ: generateCompare(inst, "setne"); break;
        case IR_LT:  generateCompare(inst, "setl");  break;
        case IR_LE:  generateCompare(inst, "setle"); break;
        case IR_GT:  generateCompare(inst, "setg");  break;
        case IR_GE:  generateCompare(inst, "setge"); break;

        // Operadores Logicos

        case IR_AND: generateLogicalOp(inst, "and"); break; 
        case IR_OR : generateLogicalOp(inst, "or"); break;
        case IR_NOT : generateLogicalOp(inst, "not"); break;

        case IR_FMETHOD:
        case IR_LABEL: generateLabel(inst); break;
        case IR_METHOD: generateLabel(inst); generateEnter(inst); break;
        case IR_GOTO: generateGoto(inst); break;
        case IR_IF: generateIf(inst); break;
        case IR_RETURN: generateReturn(inst); break;
        default:
            printf("    # [WARN] Operación IR no implementada: %d\n", inst->op);
            break;
    }
}

void generateCall(IRCode *inst) {
    Symbol *a = inst->arg1;   // nombre de la función
    Symbol *r = inst->result;
    // 2. Llamar a la función
    printf("    call %s\n", a->name);
    
    // 3. Guardar el valor de retorno (en %%rax)
    if (r) {
        if (r->is_global)
            printf("    mov %%eax, %s(%%rip)\n", r->name);
        else
            printf("    mov %%eax, %d(%%rbp)\n", r->offset);
    }
}

generateEnter(IRCode *inst) {
    // espacio que necesita cada metodo
    printf("    enter   $(%d), $0\n", inst->result->total_stack_space);
}

// =============================
// Operaciones binarias
// =============================
void generateBinaryOp(IRCode *inst, const char *op) {
    Symbol *a = inst->arg1;
    Symbol *b = inst->arg2;
    Symbol *r = inst->result;


    if (strcmp(op, "mod") == 0) {
        // Cargar arg1 en %rax
        if (a->is_global)
            printf("    mov %s(%%rip), %%rax\n", a->name);
        else
            printf("    mov %d(%%rbp), %%rax\n", a->offset);

        printf("    cqto\n"); // Extiende signo de RAX → RDX:RAX

        // Divisor
        if (b->is_global)
            printf("    idiv %s(%%rip)\n", b->name);
        else
            printf("    idiv %d(%%rbp)\n", b->offset);

        // Guardar el resto (mod) en destino
        if (r->is_global)
            printf("    mov %%rdx, %s(%%rip)\n", r->name);
        else
            printf("    mov %%rdx, %d(%%rbp)\n", r->offset);

        return;
    }

    // Cargar arg1 en %rax
    if (a->is_global)
        printf("    mov %s(%%rip), %%rax\n", a->name);
    else
        printf("    mov %d(%%rbp), %%rax\n", a->offset);

    // Aplicar operación con arg2
    if (b->is_global)
        printf("    %s %s(%%rip), %%rax\n", op, b->name);
    else
        printf("    %s %d(%%rbp), %%rax\n", op, b->offset);

    // Guardar resultado
    if (r->is_global)
        printf("    mov %%rax, %s(%%rip)\n", r->name);
    else
        printf("    mov %%rax, %d(%%rbp)\n", r->offset);
}

void generateLogicalOp(IRCode *inst, const char *op) {
    Symbol *a = inst->arg1;
    Symbol *b = inst->arg2;
    Symbol *r = inst->result;

    // === NOT bit a bit ===
    if (strcmp(op, "not") == 0) {
        // Cargar arg1 en %rax
        if (a->is_global)
            printf("    mov %s(%%rip), %%rax\n", a->name);
        else
            printf("    mov %d(%%rbp), %%rax\n", a->offset);

        // Aplicar NOT bit a bit
        printf("    not %%rax\n");

        // Guardar resultado
        if (r->is_global)
            printf("    mov %%rax, %s(%%rip)\n", r->name);
        else
            printf("    mov %%rax, %d(%%rbp)\n", r->offset);
        return;
    }

    // === AND / OR ===

    // Cargar arg1 en %rax
    if (a->is_global)
        printf("    mov %s(%%rip), %%rax\n", a->name);
    else
        printf("    mov %d(%%rbp), %%rax\n", a->offset);

    // Aplicar operación con arg2
    if (b->is_global)
        printf("    %s %s(%%rip), %%rax\n", op, b->name);
    else
        printf("    %s %d(%%rbp), %%rax\n", op, b->offset);

    // Guardar resultado
    if (r->is_global)
        printf("    mov %%rax, %s(%%rip)\n", r->name);
    else
        printf("    mov %%rax, %d(%%rbp)\n", r->offset);
}

void generateCompare(IRCode *inst, const char *set_op) {
    Symbol *a = inst->arg1;
    Symbol *b = inst->arg2;
    Symbol *r = inst->result;

    // Cargar arg1 en %rax
    if (a->is_global)
        printf("    mov %s(%%rip), %%rax\n", a->name);
    else
        printf("    mov %d(%%rbp), %%rax\n", a->offset);

    // Comparar con arg2
    if (b->is_global)
        printf("    cmp %s(%%rip), %%rax\n", b->name);
    else
        printf("    cmp %d(%%rbp), %%rax\n", b->offset);

    // Guardar resultado (0 o 1)
    printf("    %s %%al\n", set_op);
    printf("    movzbq %%al, %%rax\n");
    if (r->is_global)
        printf("    mov %%rax, %s(%%rip)\n", r->name);
    else
        printf("    mov %%rax, %d(%%rbp)\n", r->offset);
}



// =============================
// Asignaciones simples
// =============================
void generateAssign(IRCode *inst) {
    Symbol *a = inst->arg1;
    Symbol *r = inst->result;

    if (a->is_global)
        printf("    mov %s(%%rip), %%rax\n", a->name);
    else
        printf("    mov %d(%%rbp), %%rax\n", a->offset);

    if (r->is_global)
        printf("    mov %%rax, %s(%%rip)\n", r->name);
    else
        printf("    mov %%rax, %d(%%rbp)\n", r->offset);
}

void data(Symbol *global) {

}

// =============================
// Labels y Goto
// =============================
void generateLabel(IRCode *inst) {
    if (inst->op == IR_FMETHOD) printf("f%s:\n", inst->result->name);
    else printf("%s:\n", inst->result->name);
}

void generateGoto(IRCode *inst) {
    printf("    jmp %s\n", inst->result->name);
}

// =============================
// If (salto condicional simple)
// =============================
void generateIf(IRCode *inst) {
    Symbol *cond = inst->arg1;
    Symbol *label = inst->result;

    // Cargar condición
    if (cond->is_global)
        printf("    mov %s(%%rip), %%rax\n", cond->name);
    else
        printf("    mov %d(%%rbp), %%rax\n", cond->offset);

    printf("    cmp $0, %%rax\n");
    printf("    jne %s\n", label->name); // salta si distinto de 0
}

// =============================
//  Return
// =============================
void generateReturn(IRCode *inst) {
    if (inst->result != NULL) {
        Symbol *res = inst->result;
        if (res->is_global)
            printf("    mov %s(%%rip), %%rax\n", res->name);
        else
            printf("    mov %d(%%rbp), %%rax\n", res->offset);
    }
    printf("    leave\n");
    printf("    ret\n");
}


// no hay instruccion load equivalente sino que se contempla cuando se reserva espacio al inicio del metodo con enter.