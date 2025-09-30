# Makefile para TDS25

# =====================
# Variables
# =====================
SRC_DIR=src
INC_DIR=include
BUILD_DIR=build
BIN_DIR=bin
TARGET=$(BIN_DIR)/c-tds

BISON=$(SRC_DIR)/bison.y
FLEX=$(SRC_DIR)/flex.l

CC=gcc
CFLAGS=-Wall -Wextra -g -I$(INC_DIR)
FLFLAGS=-lfl

OBJS=$(BUILD_DIR)/bison.tab.c \
     $(BUILD_DIR)/lex.yy.c \
     $(SRC_DIR)/main.c \
     $(SRC_DIR)/Tree.c \
     $(SRC_DIR)/SymbolTable.c \
     $(SRC_DIR)/Stack.c \
     $(SRC_DIR)/Symbol.c \
	 $(SRC_DIR)/CodeGen.c

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
compile: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	@echo "${YELLOW}>> 🔧 Linkeando ejecutable...${NC}"
	$(CC) -o $@ $^ $(FLFLAGS) $(CFLAGS) || { echo "${RED}Error en compilación${NC}"; exit 1; }
	@echo "${GREEN}>> ✅ Compilación exitosa${NC}"
	@mkdir -p $(RESULT_DIRS)

# Generar bison
$(BUILD_DIR)/bison.tab.c $(BUILD_DIR)/bison.tab.h: $(BISON)
	@mkdir -p $(BUILD_DIR)
	bison -d -o $(BUILD_DIR)/bison.tab.c $<

# Generar flex
$(BUILD_DIR)/lex.yy.c: $(FLEX)
	@mkdir -p $(BUILD_DIR)
	flex -o $@ $<

# =====================
# Ejecutar todos los tests (depende de compile)
# =====================
run_tests: check_target compile
	@dos2unix scriptTest.sh
	@./scriptTest.sh $(TEST_TARGET)

# =====================
# Limpiar binarios y resultados
# =====================
clean:
	@echo "${YELLOW}>>🧹 Limpiando...${NC}"
	rm -f $(TARGET) $(BUILD_DIR)/*.c $(BUILD_DIR)/*.h
	rm -rf $(RESULT_DIRS)
	@echo "${GREEN}>> Limpieza completa${NC}"
