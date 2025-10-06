#include "Assembler.h"
#include <stdio.h>
#include <stdlib.h>

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
                    printf("  Parámetro %d '%s': offset %d (%s)\n", 
                           param_sym->param_index,
                           param_sym->name, 
                           param_sym->offset,
                           param_sym->param_index < 6 ? "registro" : "stack");
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
                printf("Método '%s': %d parámetros (%d por registro, %d por stack), %d locales, stack space: %d bytes\n",
                       node->sym->name, param_count, 
                       (param_count > 6) ? 6 : param_count,
                       (param_count > 6) ? param_count - 6 : 0,
                       local_count, node->sym->total_stack_space);
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
                    printf("  Variable local '%s': offset %d\n", 
                           node->sym->name, node->sym->offset);
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
        printf("Variable global '%s' (sin offset, usa label)\n", node->sym->name);
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
            printf("  Variable de bloque '%s': offset %d\n", 
                   node->sym->name, node->sym->offset);
            #endif
        }
    }
    
    assign_block_locals(node->left, offset);
    assign_block_locals(node->right, offset);
}

void generate_assembly(Tree *node, FILE *output_file) {
    if (!node || !output_file) return;
    
    // TODO: Implementar generación de assembly
    fprintf(output_file, "# Assembly generation not yet implemented\n");
    fprintf(output_file, ".section .text\n");
    fprintf(output_file, ".globl main\n");
}
