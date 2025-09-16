# Compilador TDS25

Este proyecto implementa un compilador para el lenguaje **TDS25**, un lenguaje imperativo simple,
similar a C/Pascal, en el marco de la materia **Taller de Diseño de Software (3306)** de la UNRC.

---

## 📌 Estructura del Proyecto
- `flex.l` → Analizador léxico (tokens, palabras reservadas, comentarios, identificadores).
- `bison.y` → Analizador sintáctico (gramática del lenguaje TDS25).
- `main.c` → Programa principal que coordina la ejecución del compilador.
- `Makefile` → Script de compilación y automatización.
- `tests/` → Casos de prueba (`.ctds`).
- `resultados/` → Carpeta donde se guardan los resultados de los tests.
  - `resultados/correct/`
  - `resultados/fail/`

---

## ⚙️ Compilación
Para compilar se requiere tener instalados **flex** y **bison** en el sistema.

Compilación con Make:

```bash
make compile
```

Esto genera el ejecutable `c-tds`.

También puedes compilar manualmente:

```bash
bison -d bison.y
flex flex.l
gcc -o c-tds bison.tab.c lex.yy.c main.c -lfl -Wall -Wextra -g
```

---

## 🧭 Opciones de la línea de comandos

| Opción | Acción |
|--------|--------|
| `-o <salida>` | Renombra el archivo ejecutable a `<salida>` (archivo de salida). |
| `-target <etapa>` | `<etapa>` es una de `scan`, `parse`, `codinter` o `assembly`. La compilación procede hasta la etapa dada. |
| `-opt [optimización]` | Realiza optimizaciones; `all` ejecuta todas las optimizaciones soportadas. |
| `-debug` | Imprime información de debugging. Si la opción **no** es dada, cuando la compilación es exitosa no debería imprimirse ninguna salida. |

> **Table 1:** Argumentos de la línea de comandos del Compilador

---

## 🚀 Ejecución

> **Nota:** el orden recomendado es poner primero las opciones y luego el archivo de entrada (por ejemplo `./c-tds -t scan archivo.ctds`), aunque el `main.c` está preparado para leer `getopt_long`.

### 1. Forma por defecto (escaneo léxico)
Si no se especifica ninguna bandera, se ejecuta en modo **scan**:

```bash
./c-tds archivo.ctds
```

Equivalente a:

```bash
./c-tds -target scan archivo.ctds
```

---

### 2. Selección de etapa con `-target`
El compilador permite elegir la etapa de análisis a ejecutar:

```bash
./c-tds -target <etapa> archivo.ctds
```

Donde `<etapa>` puede ser:

- `scan` → Ejecuta el analizador léxico (tokens).
- `parse` → Ejecuta el análisis sintáctico.
- `codinter` → Genera código intermedio (simulado).
- `assembly` → Genera código ensamblador (simulado).

Ejemplo con debug:

```bash
./c-tds -d -target scan tests/TestCorrect1.ctds
```

---

### 3. Ejecución con Makefile (tests automáticos)
El `Makefile` incluye reglas para ejecutar los tests automáticamente y una variable `TEST_TARGET` para elegir la etapa.

#### Targets disponibles:
- `make compile` → Compila el compilador.
- `make run_tests` → Ejecuta **todos** los tests (correctos y fallidos).
- `make run_tests_correct` → Ejecuta solo los tests correctos.
- `make run_tests_fail` → Ejecuta solo los tests que deben fallar.
- `make clean` → Limpia binarios y resultados.

#### Cambiar el target de prueba:
Por defecto, los tests se corren con `scan`. Para cambiar el target:

```bash
make run_tests TEST_TARGET=parse
```

Ejemplos:

```bash
# Correr todos los tests en etapa parse
make run_tests TEST_TARGET=parse

# Correr solo tests correctos en etapa scan
make run_tests_correct TEST_TARGET=scan

# Correr solo tests fallidos en etapa codinter
make run_tests_fail TEST_TARGET=codinter
```

> El Makefile valida el `TEST_TARGET` antes de ejecutar los tests; si se pasa un valor inválido abortará con un mensaje.

---

## 📂 Resultados
Los resultados de la ejecución de los tests se guardan en:

```
resultados/correct/
resultados/fail/
```

Cada archivo de prueba genera su salida correspondiente (archivo `.out`) en la carpeta según su tipo.

---

## ❌ Errores y códigos de salida
- `0` → Ejecución correcta.
- `1` → Error léxico/sintáctico o target desconocido.

Ejemplo:

```bash
./c-tds -target hola tests/TestCorrect1.ctds
# Salida:
# Target desconocido: hola
# Código de salida = 1
```

---

## 📚 Autores
Rosatti Nicolle, Baldevenito Joaquin, Marcial Valentin — Taller de Diseño de Software (3306), UNRC
