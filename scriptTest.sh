#!/bin/bash
TARGET=$1   # scan, parse, codinter, assembly

# Carpetas de tests y resultados
TEST_DIRS=("tests/correct" "tests/syntax_fail" "tests/semantic_fail")
RESULT_DIRS=("resultados/correct" "resultados/syntax" "resultados/semantic")
TEST_LABELS=("CORRECTOS" "FALLO SINTAXIS" "FALLO SEMÁNTICO")

# Colores
GREEN="\033[0;32m"
RED="\033[0;31m"
YELLOW="\033[1;33m"
BLUE="\033[1;34m"
NC="\033[0m"

# Contadores
total=0
passed=0
failed=0

echo -e "${BLUE}==============================================${NC}"
echo -e "${BLUE}🧪 Ejecutando tests para target: $TARGET${NC}"
echo -e "${BLUE}==============================================${NC}"

# Loop por cada tipo de test
for i in "${!TEST_DIRS[@]}"; do
    # Si target=scan, solo procesar tests correctos
    if [ "$TARGET" = "scan" ] && [ $i -eq 2 ]; then
        continue
    fi
    TEST_DIR=${TEST_DIRS[$i]}
    RES_DIR=${RESULT_DIRS[$i]}
    LABEL=${TEST_LABELS[$i]}
    mkdir -p $RES_DIR

    echo -e "${YELLOW}--- ${LABEL} ---${NC}"

    for f in $TEST_DIR/*.ctds; do
        total=$((total+1))
        base=$(basename $f .ctds)

        if [ "$TARGET" = "assembly" ]; then
            ext="s"
        else
            ext="out"
        fi
        
        ./bin/c-tds -t $TARGET $f > $RES_DIR/$base.ext 2>&1

        code=$?

        case $i in
            0) expected_code=0 ;;
            1) expected_code=1 ;;
            2) expected_code=2 ;;
        esac

        if [ $code -eq $expected_code ]; then
            echo -e "${GREEN}✅ $(printf '%-30s' $base) → OK${NC}"
            passed=$((passed+1))
        else
            echo -e "${RED}❌ $(printf '%-30s' $base) → FAIL (got $code, expected $expected_code)${NC}"
            failed=$((failed+1))
        fi
    done
done

# =====================
# Resumen final
# =====================
echo -e "${BLUE}==============================================${NC}"
echo -e "${YELLOW}▶ Resumen final:${NC}"
echo -e "${GREEN}✅ Pasaron: $passed${NC}"
echo -e "${RED}❌ Fallaron: $failed${NC}"
echo -e "${YELLOW}⚡ Total tests: $total${NC}"
echo -e "${BLUE}==============================================${NC}"
