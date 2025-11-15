# Compilador TDS25

Este proyecto implementa un compilador para el lenguaje **TDS25**, un lenguaje imperativo simple,
similar a C/Pascal, en el marco de la materia **Taller de Diseño de Software (3306)** de la UNRC.

---

## 📌 Estructura del Proyecto
- `flex.l` → Analizador léxico (tokens, palabras reservadas, comentarios, identificadores).
- `bison.y` → Analizador sintáctico (gramática del lenguaje TDS25).
- `main.c` → Programa principal que coordina la ejecución del compilador.
- `Makefile` → Script de compilación y automatización.
- `scriptTest.sh` → Script para ejecutar tests automáticos.
- `tests/` → Casos de prueba (.ctds), clasificados en subcarpetas:
  - `tests/correct/` → Tests que deben pasar.
  - `tests/syntax_fail/` → Tests con errores de sintaxis.
  - `tests/semantic_fail/` → Tests con errores semánticos.
- `resultados/` → Carpeta donde se guardan los resultados de los tests.
  - `resultados/correct/`
  - `resultados/syntax/`
  - `resultados/semantic/`

---

## 📍 Requisitos previos 
Para normalizar el formato de los saltos de linea en scriptTest.sh
```bash
sudo apt-get install dos2unix
```

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
gcc -o c-tds bison.tab.c lex.yy.c main.c SymbolTable.c Stack.c Symbol.c -lfl -Wall -Wextra -g
```

---

## 🧭 Opciones de la línea de comandos
Pueden usarse sus formas largas como --target, --debug, --opt.

| Opción | Acción |
|--------|--------|
| `-o <salida>` | Renombra el archivo ejecutable a `<salida>` (archivo de salida). |
| `-t <etapa>` | `<etapa>` es una de `scan`, `parse`, `codinter` o `assembly`. La compilación procede hasta la etapa dada. |
| `-opt [optimización]` | Realiza optimizaciones; `all` ejecuta todas las optimizaciones soportadas. |
| `-d` | Imprime información de debugging. Si la opción **no** es dada, cuando la compilación es exitosa no debería imprimirse ninguna salida. |

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
./c-tds --target scan archivo.ctds
```

---

### 2. Selección de etapa con `-target`
El compilador permite elegir la etapa de análisis a ejecutar:

```bash
./c-tds --target <etapa> archivo.ctds
```

Donde `<etapa>` puede ser:

- `scan` → Ejecuta el analizador léxico (tokens).
- `parse` → Ejecuta el análisis sintáctico.
- `codinter` → Genera código intermedio (simulado).
- `assembly` → Genera código ensamblador (simulado).

Ejemplo con debug:

```bash
./c-tds -d --target scan tests/correct/TestCorrect1.ctds
```

---

### 3. Ejecución con Makefile (tests automáticos)
El `Makefile` incluye reglas para ejecutar los tests automáticamente y una variable `TEST_TARGET` para elegir la etapa.

#### Targets disponibles:
- `make compile` → Compila el compilador.
- `make run_tests` → Ejecuta **todos** los tests.
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
```

> El Makefile valida el `TEST_TARGET` antes de ejecutar los tests; si se pasa un valor inválido abortará con un mensaje.

---

### 4\. Compilación y Enlace con Funciones Externas (Runtime)

El compilador `c-tds` sabe cómo manejar las llamadas a funciones externas (como `print_int` o `get_int`), generando las instrucciones `call` correctas en el ensamblador. Sin embargo, no proporciona la *definición* de estas funciones.

Para crear un ejecutable completo, necesitas enlazar (linkear) el código assembly generado con un archivo "runtime" que sí las defina (por ejemplo, un `runtime.c`).

El proceso es el siguiente:

**1. Generar el código Assembly:**
Usa el target `assembly` y redirige la salida a un archivo `.s`.

```bash
./c-tds -t assembly tests/correct/mi_programa.ctds > mi_programa.s
```

**2. Compilar y Enlazar con el Runtime:**
Usa `gcc` para compilar el archivo `.s` generado y enlazarlo con tu archivo `runtime.c` (o cualquier archivo que contenga las definiciones).

```bash
# Asumiendo que tienes un archivo runtime.c que define print_int, get_int, etc.
gcc -o mi_ejecutable mi_programa.s runtime.c
```

**3. Ejecutar el programa final:**
Ahora puedes ejecutar el binario compilado.

```bash
./mi_ejecutable
```

## 📂 Resultados
Los resultados de la ejecución de los tests se guardan en:

```
resultados/correct/
resultados/syntax/
resultados/semantic/
```

Cada archivo de prueba genera su salida correspondiente (archivo `.out`) en la carpeta según su tipo.

#### El script genera un dashboard en colores mostrando:
  - ✅ Tests que pasaron.
  - ❌ Tests que fallaron (con código esperado y obtenido).
  - Resumen final con totales.

---

## ❌ Errores y códigos de salida
- `0` → Ejecución correcta.
- `1` → Error léxico/sintáctico o target desconocido.
- `2` → Error semántico.

Ejemplo:

```bash
./c-tds --target hola tests/correct/TestCorrect1.ctds
# Salida:
# Target desconocido: hola
# Código de salida = 1
```

---

## 📚 Autores
Rosatti Nicolle, Baldevenito Joaquin, Marcial Valentin — Taller de Diseño de Software (3306), UNRC
