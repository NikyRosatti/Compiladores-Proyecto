# Makefile Temporal (Bison + Flex + GCC)

# Variables
BISON=bison.y
FLEX=flex.l
CC=gcc
TARGET=parser
OBJS=bison.tab.c lex.yy.c tree.c SymbolTable.c Stack.c
FLFLAGS=-lfl
#CFLAGS=-Wall -DMODO=$(MODO)

# Carpetas de resultados
RESULT_DIRS=resultados

# Tests
TESTS=$(wildcard test/Test*.txt)

# Colores
GREEN=\033[0;32m
RED=\033[0;31m
YELLOW=\033[1;33m
NC=\033[0m

.PHONY: all clean compile run_tests

# === Default ===
all: compile run_tests

# === Compilación ===
# $(CC) -o $(TARGET) $(OBJS) $(FLFLAGS) $(CFLAGS) || agregar cuando el interprete este funcionando
compile:
	@echo "${YELLOW}>> Compilando analizador...${NC}"
	bison -d $(BISON) || { echo "${RED}Error en Bison${NC}"; exit 1; }
	flex $(FLEX) || { echo "${RED}Error en Flex${NC}"; exit 1; }
	$(CC) -o $(TARGET) $(OBJS) $(FLFLAGS) || { echo "${RED}Error en compilación${NC}"; exit 1; }
	@echo "${GREEN}>> Compilación exitosa${NC}"
	@mkdir -p $(RESULT_DIRS)

# === Regla genérica para ejecutar tests ===
define run_test_loop
	@mkdir -p resultados/$(1)
	@echo "${YELLOW}>> Ejecutando tests $(1)...${NC}"
	@for t in $($(2)); do \
		base=$$(basename $$t .txt); \
		./$(TARGET) < $$t > resultados/$(1)/$$base.out 2>&1; \
		code=$$?; \
		expected_code=$(3); \
		if [ $$code -eq $$expected_code ]; then \
			echo "${GREEN}✅ $$t OK${NC}"; \
		else \
			echo "${RED}❌ $$t FAIL${NC}"; \
		fi; \
	done
endef

# === Ejecutar todos los tests ===
run_tests: compile
	$(call run_test_loop,correct,TESTS,0)

# === Limpiar binarios y resultados ===
clean:
	@echo "${YELLOW}>> Limpiando...${NC}"
	rm -f $(TARGET) parserbison.tab.c parserbison.tab.h lex.yy.c
	rm -rf resultados
	@echo "${GREEN}>> Limpieza completa${NC}"
