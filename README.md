# Compilador TDS25

Este proyecto implementa un compilador para el lenguaje **TDS25**, un lenguaje imperativo simple
similar a C/Pascal, en el marco de la materia **Taller de Diseño de Software (3306)** de la UNRC.

## 📌 Estructura del Proyecto
- `flex.l` → Analizador léxico (tokens, palabras reservadas, comentarios, identificadores).
- `bison.y` → Analizador sintáctico (gramática del lenguaje TDS25).
- `Makefile` → Script de compilación.
- `tests/` → Casos de prueba.

## ⚙️ Compilación
Para compilar se requiere **flex** y **bison** instalados:

```bash
make
