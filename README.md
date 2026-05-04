# Hoja de Cálculo — Matriz Dispersa

**Curso:** Algoritmos y Estructura de Datos  
**Proyecto Nro. 1**  
**Lenguaje:** C++17  
**Interfaz:** Qt 6 (Widgets)

---

## Descripción

Aplicación de hoja de cálculo simple que utiliza una **matriz dispersa implementada con listas enlazadas cruzadas** como estructura principal de almacenamiento. Solo las celdas con contenido ocupan memoria; las celdas vacías no crean ningún nodo.

---

## Estructura de datos: Lista enlazada cruzada

Cada celda con contenido es un `Node` con los siguientes campos:

```
┌─────────────────────────────────┐
│  row, col     → coordenadas     │
│  rawValue     → valor o fórmula │
│  cacheValue   → resultado de =  │
│  left / right → lista de fila   │
│  up   / down  → lista de columna│
└─────────────────────────────────┘
```

Se mantienen dos arreglos de punteros cabecera: `rowHeaders[r]` apunta al primer nodo de la fila `r`, y `colHeaders[c]` apunta al primer nodo de la columna `c`. La lista es **doblemente enlazada** en ambas direcciones, lo que permite eliminaciones eficientes sin recorrer desde la cabecera.

### Justificación frente a matriz densa

| Criterio | Matriz densa | Matriz dispersa (este proyecto) |
|---|---|---|
| Memoria | O(n·m) siempre | O(k) donde k = celdas ocupadas |
| Insertar celda | O(1) | O(k_fila + k_col) |
| Eliminar celda | O(1) | O(k_fila + k_col) |
| Eliminar fila | O(m) | O(k_fila) |
| Suma de rango | O((r2−r1)·(c2−c1)) | O(nodos en rango) |

En una hoja de 100×26 = 2600 celdas con solo 50 valores ingresados, la matriz densa asigna memoria para las 2600 posiciones mientras la dispersa solo usa 50 nodos. La ventaja crece con el tamaño de la hoja.

---

## Requisitos

- **Qt 6.x** con módulo `Qt Widgets`  
- **CMake 3.16** o superior  
- **MinGW 13** (Windows) o `g++ 11+` (Linux/macOS)

---

## Instalación de Qt

1. Crear cuenta educativa en [https://www.qt.io](https://www.qt.io)
2. Descargar e instalar Qt Online Installer
3. Seleccionar durante la instalación:
   - `Qt 6.x` → `MinGW 64-bit`
   - `Qt 6.x` → `Qt Widgets`
   - `Developer and Designer Tools` → `MinGW 13.x 64-bit`
   - `Developer and Designer Tools` → `Ninja`

---

## Compilación en VS Code

### 1. Agregar Qt y Ninja al PATH

En Variables de entorno del sistema, agregar:

```
C:\Qt\6.x.x\mingw_64\bin
C:\Qt\Tools\mingw1310_64\bin
C:\Qt\Tools\Ninja
```

### 2. Configurar el kit de CMake

`Ctrl + Shift + P` → `CMake: Edit User-Local CMake Kits`

Reemplazar el contenido con:

```json
[
  {
    "name": "Qt MinGW Manual",
    "compilers": {
      "C":   "C:/Qt/Tools/mingw1310_64/bin/gcc.exe",
      "CXX": "C:/Qt/Tools/mingw1310_64/bin/g++.exe"
    },
    "preferredGenerator": {
      "name": "MinGW Makefiles"
    }
  }
]
```

### 3. Configurar el generador

`Ctrl + ,` → buscar `CMake: Generator` → escribir `MinGW Makefiles`

### 4. Configurar el entorno de CMake

`Ctrl + ,` → buscar `cmake.configureEnvironment` → agregar ítem:

| Clave | Valor |
|---|---|
| `PATH` | `C:\Qt\Tools\mingw1310_64\bin;${env:PATH}` |

### 5. Compilar y ejecutar

```
Ctrl + Shift + P → CMake: Build        (o F7)
Ctrl + Shift + P → CMake: Run          (o Shift + F5)
```

---

## Uso de la aplicación

### Ingresar datos en una celda

1. Hacer clic en una celda
2. Escribir directamente en la celda o en la barra de fórmulas (`ƒx`)
3. Confirmar con `Enter`

Tipos de valor aceptados:
- **Número:** `42`, `3.14`, `-7`
- **Texto:** `Ventas`, `Total`
- **Fórmula:** `=A1+B1`, `=A1*2`, `=(A1+B2)/C3`

### Fórmulas

Las fórmulas comienzan con `=` y soportan:

| Elemento | Ejemplo |
|---|---|
| Operadores | `+`, `-`, `*`, `/` |
| Referencias a celdas | `=A1+B2` |
| Paréntesis | `=(A1+B1)*C1` |
| Números literales | `=A1*2.5` |
| Signo negativo | `=-A1` |

Las celdas con fórmula se muestran en **azul** y se recalculan automáticamente al modificar cualquier celda referenciada.

### Operaciones sobre filas y columnas

Disponibles en el menú **Editar** y en la barra de herramientas:

| Operación | Descripción |
|---|---|
| Insertar fila antes / después | Inserta una fila respecto a la selección actual |
| Eliminar fila | Elimina la fila de la celda seleccionada |
| Insertar columna izquierda / derecha | Inserta una columna respecto a la selección |
| Eliminar columna | Elimina la columna de la celda seleccionada |
| Eliminar rango | Elimina el contenido de todas las celdas seleccionadas |

### Operaciones de agregación

1. **Seleccionar un rango** de celdas (clic y arrastrar)
2. Ir al menú **Fórmulas** o usar la barra de herramientas
3. Elegir la operación deseada

| Operación | Menú / Botón | Comportamiento con texto |
|---|---|---|
| SUMA | `∑ Suma` | Suma solo valores numéricos |
| PROMEDIO | `x̄ Prom.` | Promedio solo sobre celdas numéricas (ignora texto) |
| MÁXIMO | `↑ Máx` | Máximo solo entre valores numéricos (ignora texto) |
| MÍNIMO | `↓ Mín` | Mínimo solo entre valores numéricos (ignora texto) |

El resultado se muestra en un cuadro de diálogo con el rango seleccionado, por ejemplo: `SUMA(A1:C3) = 42`.

### Guardar y abrir archivos

- **Guardar:** `Ctrl + S` o menú Archivo → Guardar. Genera un archivo `.csv`.
- **Abrir:** `Ctrl + O` o menú Archivo → Abrir. Carga un `.csv` previamente guardado.
- **Nueva hoja:** `Ctrl + N` o menú Archivo → Nuevo.

---

## Casos borde manejados

| Caso | Comportamiento |
|---|---|
| Consultar celda inexistente | Retorna vacío, sin error |
| Eliminar celda inexistente | No hace nada, sin crash |
| Eliminar fila o columna vacía | Se ejecuta sin error |
| Agregar texto en una celda | Se almacena como texto, no afecta agregaciones numéricas |
| SUMA / PROMEDIO / MÁX / MÍN sobre rango sin números | Retorna 0 sin error |
| División por cero en fórmula | La fórmula muestra 0 sin crash |
| Referencia a celda vacía en fórmula | Se trata como 0 |

---

## Estructura del proyecto

```
├── main.cpp          → Punto de entrada, inicialización de QApplication
├── SparseMatrix.h    → Definición de Node y SparseMatrix
├── SparseMatrix.cpp  → Implementación de la matriz dispersa y parser de fórmulas
├── Mainwindow.h      → Declaración de la ventana principal
├── Mainwindow.cpp    → Interfaz gráfica Qt y conexión con SparseMatrix
└── CMakeLists.txt    → Configuración de compilación
```

### Separación de responsabilidades

- `SparseMatrix` no depende de Qt; es una clase de estructura de datos pura.
- `MainWindow` no contiene lógica de datos; delega todo a `SparseMatrix`.
- `main.cpp` solo inicializa la aplicación Qt.

---

## Análisis de complejidad

### Operaciones sobre nodos

| Operación | Complejidad temporal | Descripción |
|---|---|---|
| `set(r, c, val)` — insertar | O(k_fila + k_col) | Recorre la fila y la columna para encontrar la posición |
| `set(r, c, val)` — modificar | O(k_fila) | El nodo ya existe, solo actualiza el valor |
| `get(r, c)` | O(k_fila) | Recorre la fila r hasta la columna c |
| `remove(r, c)` | O(k_fila + k_col) | Busca el nodo previo en fila y en columna |
| `removeRow(r)` | O(k_fila · k_col_avg) | Llama a `remove()` por cada nodo de la fila |
| `removeColumn(c)` | O(k_col · k_fila_avg) | Llama a `remove()` por cada nodo de la columna |
| `removeRange(r1,c1,r2,c2)` | O(nodos en rango) | Itera solo los nodos existentes en el rango |

Donde `k` representa el número de elementos en una fila o columna, siempre menor o igual que el total de nodos `n`. En el peor caso (hoja completamente llena) estas operaciones se acercan a O(n), pero en una hoja dispersa típica `k << n`.

### Operaciones de agregación

| Operación | Complejidad temporal |
|---|---|
| `sumRange` | O(nodos en rango) |
| `averageRange` | O(nodos en rango) |
| `maxInRange` / `minInRange` | O(nodos en rango) |

Las cuatro recorren solo los nodos que existen dentro del rango, no las celdas vacías.

### Complejidad espacial

| Estructura | Espacio |
|---|---|
| Nodos de datos | O(k) donde k = celdas con contenido |
| Arreglos de cabeceras | O(n + m) donde n = filas, m = columnas |
| **Total** | **O(k + n + m)** |

Comparado con una matriz densa que requiere O(n·m) independientemente de cuántas celdas estén ocupadas, la estructura dispersa es significativamente más eficiente en memoria para hojas con baja densidad de datos.
