# Makefile para TDS25

# =====================
# Variables
# =====================
BISON=bison.y
FLEX=flex.l
CC=gcc
TARGET=c-tds
OBJS=bison.tab.c lex.yy.c main.c tree.c SymbolTable.c Stack.c
FLFLAGS=-lfl
CFLAGS=-Wall -Wextra -g
# Target por defecto si no se pasa
VALID_TARGETS := scan parse codinter assembly


# Carpeta de resultados
RESULT_DIRS=resultados/correct resultados/syntax resultados/semantic

# Tests
TESTS_CORRECT=$(wildcard tests/TestCorrect*.ctds)
TESTS_FAIL_SYNTAX=$(wildcard tests/TestSyntaxFail*.ctds)
TESTS_FAIL_SEMANTIC=$(wildcard tests/TestSemanticFail*.ctds)

# Colores
GREEN=\033[0;32m
RED=\033[0;31m
YELLOW=\033[1;33m
NC=\033[0m

.PHONY: all clean compile run_tests run_tests_correct run_tests_fail

# =====================
# Chequea target valido
# =====================
check_target:
	@if ! echo "$(VALID_TARGETS)" | grep -qw "$(TEST_TARGET)"; then \
		echo "${RED}❌ Target desconocido: $(TEST_TARGET)${NC}"; \
		exit 1; \
	fi

# =====================
# Default: solo compila
# =====================
all: compile

# =====================
# Compilación
# =====================
compile:
	@echo "${YELLOW}>> Compilando analizador...${NC}"
	bison -d $(BISON) || { echo "${RED}Error en Bison${NC}"; exit 1; }
	flex $(FLEX) || { echo "${RED}Error en Flex${NC}"; exit 1; }
	$(CC) -o $(TARGET) $(OBJS) $(FLFLAGS) $(CFLAGS) || { echo "${RED}Error en compilación${NC}"; exit 1; }
	@echo "${GREEN}>> Compilación exitosa${NC}"
	@mkdir -p $(RESULT_DIRS)


# =====================
# Función para ejecutar tests
# =====================
define run_test_loop
	@mkdir -p resultados/correct resultados/syntax resultados/semantic
	@echo "${YELLOW}>> Ejecutando tests y clasificando resultados con target $(TEST_TARGET)...${NC}"
	@for t in $(2); do \
		base=$$(basename $$t .ctds); \
		./$(TARGET) -t $(TEST_TARGET) $$t > resultados/$(1)/$$base.out 2>&1; \
		code=$$?; \
		exepted_code=$(3); \
		if [ $$exepted_code -eq $$code ]; then \
			echo "${GREEN}✅ $$t CORRECTO${NC}"; \
		else \
			echo "${RED}❌ $$t FAIL ${NC}"; \
		fi; \
	done
endef

# =====================
# Ejecutar todos los tests (depende de compile)
# =====================
run_tests: check_target compile run_tests_correct run_tests_fail_syntax run_tests_fail_semantic

run_tests_correct:
	$(call run_test_loop,correct,$(TESTS_CORRECT),0)

run_tests_fail_syntax:
	$(call run_test_loop,syntax,$(TESTS_FAIL_SYNTAX),1)

run_tests_fail_semantic:
	$(call run_test_loop,semantic,$(TESTS_FAIL_SEMANTIC),2)

# =====================
# Limpiar binarios y resultados
# =====================
clean:
	@echo "${YELLOW}>> Limpiando...${NC}"
	rm -f $(TARGET) bison.tab.c bison.tab.h lex.yy.c
	rm -rf resultados
	@echo "${GREEN}>> Limpieza completa${NC}"
