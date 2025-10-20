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
    printf(".data\n");
    print_globals_data(decl_vars);
    //no hay bss por la decision
    //printf("    .bss\n");
    //print_globals_bss(decl_vars);

    // seccion text
    printf(".text\n");
    printf(".globl main\n");

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
    
    for (int i = 0; i < irlist->size; i++) {
        IRCode *inst = &irlist->codes[i];
        if (!inst) continue;

        switch (inst->op) {
            case IR_DECL:
            case IR_STORE:
                // Solo variables globales fuera de métodos
                if (inst->result && inst->result->is_global) {
                    add_decl(&decl_vars, inst->result, inst->arg1);
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
        case IR_LOAD: generateLoad(inst); break;
        case IR_METH_EXT: break;
        case IR_PARAM: break;
        ///// estos de arriba no tienen instrucciones en Assembly
        case IR_STORAGE: generateStorage(inst); break;
        case IR_STORE: generateAssign(inst); break;
        case IR_ADD: generateBinaryOp(inst, "addq"); break;
        case IR_SUB: generateBinaryOp(inst, "subq"); break;
        case IR_UMINUS: generateUminus(inst); break;
        case IR_MUL: generateBinaryOp(inst, "imulq"); break;
        case IR_DIV: generateBinaryOp(inst, "idivq"); break;

        case IR_MOD: generateBinaryOp(inst, "modq");break;

        case IR_DECL:break; // implementar para locales

        case IR_CALL: generateCall(inst); break;
        // Operadores de comparacion
        case IR_EQ:  generateCompare(inst, "sete");  break;
        case IR_NEQ: generateCompare(inst, "setne"); break;
        case IR_LT:  generateCompare(inst, "setl");  break;
        case IR_LE:  generateCompare(inst, "setle"); break;
        case IR_GT:  generateCompare(inst, "setg");  break;
        case IR_GE:  generateCompare(inst, "setge"); break;

        // Operadores Logicos

        case IR_AND: generateLogicalOp(inst, "andq"); break; 
        case IR_OR : generateLogicalOp(inst, "orq"); break;
        case IR_NOT : generateLogicalOp(inst, "notq"); break;

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

void generateLoad(IRCode *inst) {
    Symbol *src = inst->arg1;
    Symbol *dst = inst->result;

    printf("    # Carga el valor de la variable '%s' en un temporal\n", src->name);

    // Cargar en %rax
    if (src->is_global)
        printf("    movq %s(%%rip), %%rax\n", src->name);
    else
        printf("    movq %d(%%rbp), %%rax\n", src->offset);

    // Guardar en destino
    if (dst->is_global)
        printf("    movq %%rax, %s(%%rip)\n", dst->name);
    else
        printf("    movq %%rax, %d(%%rbp)\n", dst->offset);

    printf("\n");
}

void generateCall(IRCode *inst) {
    Symbol *a = inst->arg1;   // nombre de la función
    Symbol *r = inst->result;
    // 2. Llamar a la función
    printf("    # Llamada a la función '%s'\n", a->name);
    printf("    call %s\n", a->name);
    
    // 3. Guardar el valor de retorno (en %%rax)
    if (r) {
        printf("    # Guardar el valor de retorno (desde RAX)\n");
        if (r->is_global)
            printf("    movq %%eax, %s(%%rip)\n", r->name);
        else
            printf("    movq %%eax, %d(%%rbp)\n", r->offset);
    }
    printf("\n");
}

void generateEnter(IRCode *inst) {
    int space = inst->result ? inst->result->total_stack_space : 0;
    printf("    # Prólogo del método: crear stack frame y reservar %d bytes\n", space);
    printf("    enter $(%d), $0\n", space);
    printf("\n");
}

// =============================
// Operaciones binarias
// =============================
// Se necesita un contador global o estático para generar etiquetas únicas
static int div_label_count = 0;

void generateBinaryOp(IRCode *inst, const char *op) {
    Symbol *a = inst->arg1;
    Symbol *b = inst->arg2;
    Symbol *r = inst->result;

    // --- MANEJO ESPECIAL PARA DIVISIÓN Y MÓDULO ---
    if (strcmp(op, "idivq") == 0 || strcmp(op, "modq") == 0) {
        int current_label = div_label_count++; // Etiqueta única para este bloque

        printf("    # --- Inicio de bloque de división/módulo ---\n");
        printf("    # Verificar si el divisor es cero\n");

        // 1. Cargar el DIVISOR y compararlo con cero
        if (b->is_global)
            printf("    movq %s(%%rip), %%rcx\n", b->name); // Usamos %rcx como registro temporal
        else
            printf("    movq %d(%%rbp), %%rcx\n", b->offset);
        
        printf("    cmpq $0, %%rcx\n");
        printf("    je _division_by_zero_error_%d\n", current_label); // Si es cero, saltar
        printf("\n");

        printf("    # Realizar la operación de división\n");
        // 2. Si no es cero, proceder con la operación normal
        if (a->is_global)
            printf("    movq %s(%%rip), %%rax\n", a->name);
        else
            printf("    movq %d(%%rbp), %%rax\n", a->offset);

        printf("    cqto\n");
        printf("    idiv %%rcx\n"); // Dividir por el registro %rcx
        printf("\n");
        // 3. Guardar el resultado correcto (cociente o resto)
        const char* result_reg = (strcmp(op, "mod") == 0) ? "%rdx" : "%rax";
        const char* op_name = (strcmp(op, "mod") == 0) ? "Módulo (%)" : "División (/)";
        printf("    # Guardar el resultado de la operación '%s'\n", op_name);
        if (r->is_global)
            printf("    movq %s, %s(%%rip)\n", result_reg, r->name);
        else
            printf("    movq %s, %d(%%rbp)\n", result_reg, r->offset);
        
        printf("    jmp _division_ok_%d\n", current_label);
        printf("\n");
        // 4. Bloque de manejo de error
        printf("_division_by_zero_error_%d:\n", current_label);
        // Aquí podrías llamar a una función que imprima un error.
        // Por ahora, simplemente terminamos el programa.
        printf("    movl $136, %%edi\n"); // Código de error que quieras usar
        printf("    call exit\n");
        printf("\n");
        printf("_division_ok_%d:\n", current_label);
        printf("    # --- Fin de bloque de división/módulo ---\n");
        printf("\n");
        return; // Termina la función aquí
    }

    // --- CÓDIGO ORIGINAL PARA OTRAS OPERACIONES (add, sub, imul) ---
    // (Este código está bien y no necesita cambios)
    printf("    # Operación binaria: %s\n", op);
    if (a->is_global)
        printf("    movq %s(%%rip), %%rax\n", a->name);
    else
        printf("    movq %d(%%rbp), %%rax\n", a->offset);

    if (b->is_global)
        printf("    %s %s(%%rip), %%rax\n", op, b->name);
    else
        printf("    %s %d(%%rbp), %%rax\n", op, b->offset);

    if (r->is_global)
        printf("    movq %%rax, %s(%%rip)\n", r->name);
    else
        printf("    movq %%rax, %d(%%rbp)\n", r->offset);

    printf("\n");
}

/**
 * Genera el código assembly para la operación de menos unario (negación).
 * IR: UMINUS src, NULL, dest  (ej. dest = -src)
 */
void generateUminus(IRCode *inst) {
    Symbol *src = inst->arg1;    // Símbolo de origen (el que se va a negar)
    Symbol *dest = inst->result; // Símbolo de destino (donde se guarda el resultado)
    printf("    # Operación unaria: negación de '%s'\n", src->name);
    // 1. Cargar el valor del operando 'src' en el registro %rax.
    if (src->is_global) {
        printf("    movq %s(%%rip), %%rax\n", src->name);
    } else {
        printf("    movq %d(%%rbp), %%rax\n", src->offset);
    }

    // 2. Aplicar la instrucción NEG a %rax.
    // Esto calcula el complemento a dos del valor en el registro.
    printf("    negq %%rax\n");

    // 3. Guardar el resultado (que ahora está en %rax) en el destino 'dest'.
    if (dest->is_global) {
        printf("    movq %%rax, %s(%%rip)\n", dest->name);
    } else {
        printf("    movq %%rax, %d(%%rbp)\n", dest->offset);
    }

    printf("\n");
}

void generateLogicalOp(IRCode *inst, const char *op) {
    Symbol *a = inst->arg1;
    Symbol *b = inst->arg2;
    Symbol *r = inst->result;

    // === NOT bit a bit ===
    if (strcmp(op, "notq") == 0) {
        printf("    # Operación lógica: NOT '%s'\n", a->name);
        // Cargar arg1 en %rax
        if (a->is_global)
            printf("    movq %s(%%rip), %%rax\n", a->name);
        else
            printf("    movq %d(%%rbp), %%rax\n", a->offset);

        // Aplicar NOT bit a bit
        printf("    notq %%rax\n");

        // Guardar resultado
        if (r->is_global)
            printf("    movq %%rax, %s(%%rip)\n", r->name);
        else
            printf("    movq %%rax, %d(%%rbp)\n", r->offset);
        
        printf("\n");
        return;
    }

    // === AND / OR ===
    printf("    # Operación lógica: %s\n", op);
    // Cargar arg1 en %rax
    if (a->is_global)
        printf("    movq %s(%%rip), %%rax\n", a->name);
    else
        printf("    movq %d(%%rbp), %%rax\n", a->offset);

    // Aplicar operación con arg2
    if (b->is_global)
        printf("    %s %s(%%rip), %%rax\n", op, b->name);
    else
        printf("    %s %d(%%rbp), %%rax\n", op, b->offset);

    // Guardar resultado
    if (r->is_global)
        printf("    movq %%rax, %s(%%rip)\n", r->name);
    else
        printf("    movq %%rax, %d(%%rbp)\n", r->offset);
    
    printf("\n");
}

void generateCompare(IRCode *inst, const char *set_op) {
    Symbol *a = inst->arg1;
    Symbol *b = inst->arg2;
    Symbol *r = inst->result;

    printf("    # Comparación\n");
    // Cargar arg1 en %rax
    if (a->is_global)
        printf("    movq %s(%%rip), %%rax\n", a->name);
    else
        printf("    movq %d(%%rbp), %%rax\n", a->offset);

    // Comparar con arg2
    if (b->is_global)
        printf("    cmpq %s(%%rip), %%rax\n", b->name);
    else
        printf("    cmpq %d(%%rbp), %%rax\n", b->offset);
    printf("\n");
    printf("    # Guardar resultado booleano de la comparación\n");
    // Guardar resultado (0 o 1)
    printf("    %s %%al\n", set_op);
    printf("    movzbq %%al, %%rax\n");
    if (r->is_global)
        printf("    movq %%rax, %s(%%rip)\n", r->name);
    else
        printf("    movq %%rax, %d(%%rbp)\n", r->offset);
    printf("\n");
}


// =============================
// STORAGE: carga un literal o temporal
// =============================
void generateStorage(IRCode *inst) {
    Symbol *literal = inst->arg1; // El símbolo que contiene el valor literal.
    Symbol *dest = inst->result;    // El temporal de destino en la pila.
    printf("    # Almacena el valor literal %d en el temporal '%s'\n", literal->valor.value, dest->name);
    // Genera la instrucción para mover el valor inmediato al offset del temporal.
    printf("    movq $%d, %d(%%rbp)\n", literal->valor.value, dest->offset);
    printf("\n");
}

// =============================
// ASSIGN: asigna valor de una variable o temporal
// =============================
void generateAssign(IRCode *inst) {
    Symbol *a = inst->arg1; // DEBERIA SER RAX
    Symbol *r = inst->result;
    printf("    # Asignación: '%s' = '%s'\n", r->name, a->name);

    if (a->is_global) {
        printf("    movq %s(%%rip), %%rax\n", a->name);
    } else {
        printf("    movq %d(%%rbp), %%rax\n", a->offset);
    }
    // Guardar valor en destino
    if (r->is_global) {
        printf("    movq %%rax, %s(%%rip)\n", r->name);
    } else {
        printf("    movq %%rax, %d(%%rbp)\n", r->offset);
    }
    printf("\n");
}

// =============================
// Labels y Goto
// =============================
void generateLabel(IRCode *inst) {
    if (inst->op == IR_FMETHOD) printf("f%s:\n", inst->result->name);
    else printf("%s:\n", inst->result->name);
    printf("\n");
}

void generateGoto(IRCode *inst) {
    printf("    # Salto incondicional a la etiqueta '%s'\n", inst->result->name);
    printf("    jmp %s\n", inst->result->name);
    printf("\n");
}

// =============================
// If (salto condicional simple)
// =============================
void generateIf(IRCode *inst) {
    Symbol *cond = inst->arg1;
    Symbol *label = inst->result;

    printf("    # Condición del IF\n");
    // Cargar condición
    if (cond->is_global)
        printf("    movq %s(%%rip), %%rax\n", cond->name);
    else
        printf("    movq %d(%%rbp), %%rax\n", cond->offset);

    printf("    cmpq $0, %%rax\n");
    printf("    # Si la condición es verdadera (no es cero), salta a '%s'\n", label->name);
    printf("    jne %s\n", label->name); // salta si distinto de 0
    printf("\n");
}

// =============================
//  Return
// =============================
void generateReturn(IRCode *inst) {
    printf("    # Preparando el retorno de la función\n");
    if (inst->result != NULL) {
        Symbol *res = inst->result;
        if (res->is_global)
            printf("    movq %s(%%rip), %%rax\n", res->name);
        else
            printf("    movq %d(%%rbp), %%rax\n", res->offset);
    }
    printf("    leave\n");
    printf("    ret\n");
    printf("\n");
}


// no hay instruccion load equivalente sino que se contempla cuando se reserva espacio al inicio del metodo con enter.