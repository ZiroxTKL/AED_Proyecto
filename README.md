# Hoja de Cálculo — Matriz Dispersa

---
Integrantes:

- Leonardo Martinez Aquino - 202410148
- Miguel Luis Paucar Barrios - 202410276
- Jhonatan Eder Ortega Huaman - 202410141

---

**Curso:** Algoritmos y Estructura de Datos  
**Hito 1**  
**Lenguaje:** C++ 17  
**Interfaz:** Qt

---
## Requisitos

- **Qt 6.11.0** o superior
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

## Instalación de CMake

1. Descargar e instalar Cmake de https://cmake.org/download/

---
## Compilación en VS Code

### 1. Agregar Qt y Ninja al PATH

En Variables de entorno del sistema, agregar:

```
C:\Qt\6.x.x\mingw_64\bin
C:\Qt\Tools\mingw1310_64\bin
C:\Qt\Tools\Ninja
```

### 2. Agregar CMake al PATH

```
C:\Program Files\CMake\bin
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

## Compilación en CLion

### 1. Agregar Qt y Ninja al PATH

En Variables de entorno del sistema, agregar:

```
C:\Qt\6.x.x\mingw_64\bin
C:\Qt\Tools\mingw1310_64\bin
C:\Qt\Tools\Ninja
```

### 2. Agregar CMake al PATH

```
C:\Program Files\CMake\bin
```

### 3. Añadir archivos a CLion

Crear un Nuevo Proyecto en CLion
Añadir el codigo fuente .cpp, .h y CMakeList.txt al Proyecto

### 5. Compilar y ejecutar

```
CLion: Build        (Icono de Martillo al lado de Run)
CLion: Run          (Icono Triangular en la barra Superior Derecha)
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

1. Ir al menú **Fórmulas** o usar la barra de herramientas
2. Elegir la operación deseada

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
| División por cero en fórmula | La fórmula muestra #ERROR sin crash |
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
