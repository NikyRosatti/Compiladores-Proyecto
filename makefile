# Makefile para TDS25

# =====================
# Variables
# =====================
BISON=bison.y
FLEX=flex.l
CC=gcc
TARGET=c-tds
OBJS=bison.tab.c lex.yy.c main.c tree.c SymbolTable.c Stack.c Symbol.c
FLFLAGS=-lfl
CFLAGS=-Wall -Wextra -g
# Target por defecto si no se pasa
VALID_TARGETS := scan parse codinter assembly


# Carpeta de resultados
RESULT_DIRS=resultados/correct resultados/syntax resultados/semantic

# Colores
GREEN=\033[0;32m
RED=\033[0;31m
YELLOW=\033[1;33m
BLUE=\033[0;34m
NC=\033[0m

.PHONY: all clean compile run_tests

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
	@echo "${YELLOW}>> 🔧 Compilando analizador...${NC}"
	bison -d $(BISON) || { echo "${RED}Error en Bison${NC}"; exit 1; }
	flex $(FLEX) || { echo "${RED}Error en Flex${NC}"; exit 1; }
	$(CC) -o $(TARGET) $(OBJS) $(FLFLAGS) $(CFLAGS) || { echo "${RED}Error en compilación${NC}"; exit 1; }
	@echo "${GREEN}>> ✅ Compilación exitosa${NC}"
	@mkdir -p $(RESULT_DIRS)

# =====================
# Ejecutar todos los tests (depende de compile)
# =====================
run_tests: 	check_target compile
			@./scriptTest.sh $(TEST_TARGET)

# =====================
# Limpiar binarios y resultados
# =====================
clean:
	@echo "${YELLOW}>>🧹 Limpiando...${NC}"
	rm -f $(TARGET) bison.tab.c bison.tab.h lex.yy.c
	rm -rf resultados
	@echo "${GREEN}>> Limpieza completa${NC}"
