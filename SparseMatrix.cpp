#include "SparseMatrix.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <stdexcept>

using namespace std;

// ─────────────────────────────────────────────────────────────
//  Constructor / Destructor
// ─────────────────────────────────────────────────────────────

SparseMatrix::SparseMatrix(int rows, int cols) : maxRows(rows), maxCols(cols) {
    rowHeaders = new Node*[rows]();   // () = zero-initialise
    colHeaders = new Node*[cols]();
}

SparseMatrix::~SparseMatrix() {
    for (int i = 0; i < maxRows; ++i) {
        Node* curr = rowHeaders[i];
        while (curr) {
            Node* tmp = curr;
            curr = curr->right;
            delete tmp;
        }
    }
    delete[] rowHeaders;
    delete[] colHeaders;
}

// ─────────────────────────────────────────────────────────────
//  Helpers de referencia de celda
// ─────────────────────────────────────────────────────────────

// Convierte "AA" -> 26, "A" -> 0, etc. (inversa de getCellRef)
static int colLettersToIndex(const string& letters) {
    int idx = 0;
    for (char ch : letters) {
        idx = idx * 26 + (toupper(ch) - 'A' + 1);
    }
    return idx - 1;
}

// Parsea una referencia de celda tipo "AB12" y devuelve (fila, col)
// Devuelve false si el formato es inválido
static bool parseCellRef(const string& ref, int& row, int& col) {
    size_t i = 0;
    while (i < ref.size() && isalpha(ref[i])) ++i;
    if (i == 0 || i == ref.size()) return false;
    try {
        col = colLettersToIndex(ref.substr(0, i));
        row = stoi(ref.substr(i)) - 1;
    } catch (...) {
        return false;
    }
    return (row >= 0 && col >= 0);
}

// ─────────────────────────────────────────────────────────────
//  Parser recursivo descendente para fórmulas
// ─────────────────────────────────────────────────────────────

struct ParseState {
    const std::string& expr;
    size_t pos;
    SparseMatrix* matrix;
};

static void skipSpaces(ParseState& s) {
    while (s.pos < s.expr.size() && s.expr[s.pos] == ' ')
        s.pos++;
}

// Declaraciones adelantadas
static bool parseExpr  (ParseState& s, double& result);
static bool parseTerm  (ParseState& s, double& result);
static bool parseFactor(ParseState& s, double& result);

// Factor: número | referencia de celda | (expresión) | -factor
static bool parseFactor(ParseState& s, double& result) {
    skipSpaces(s);
    if (s.pos >= s.expr.size()) return false;

    // Unario negativo
    if (s.expr[s.pos] == '-') {
        s.pos++;
        double val;
        if (!parseFactor(s, val)) return false;
        result = -val;
        return true;
    }

    // Expresión entre paréntesis
    if (s.expr[s.pos] == '(') {
        s.pos++;
        if (!parseExpr(s, result)) return false;
        skipSpaces(s);
        if (s.pos >= s.expr.size() || s.expr[s.pos] != ')') return false;
        s.pos++;
        return true;
    }

    size_t start = s.pos;

    // Referencia de celda: empieza con letra(s) seguidas de dígitos  (ej: A1, BC42)
    if (isalpha(s.expr[s.pos])) {
        while (s.pos < s.expr.size() && isalpha(s.expr[s.pos])) s.pos++;
        if (s.pos < s.expr.size() && isdigit(s.expr[s.pos])) {
            while (s.pos < s.expr.size() && isdigit(s.expr[s.pos])) s.pos++;
            std::string ref = s.expr.substr(start, s.pos - start);
            int r, c;
            if (parseCellRef(ref, r, c)) {
                result = s.matrix->getNumericValue(r, c);
                return true;
            }
        }
        return false; // letras sin número → no es referencia válida
    }

    // Número (entero o decimal)
    if (isdigit(s.expr[s.pos]) || s.expr[s.pos] == '.') {
        while (s.pos < s.expr.size() &&
               (isdigit(s.expr[s.pos]) || s.expr[s.pos] == '.'))
            s.pos++;
        try {
            result = std::stod(s.expr.substr(start, s.pos - start));
            return true;
        } catch (...) { return false; }
    }

    return false;
}

// Término: factor (* factor | / factor)*
static bool parseTerm(ParseState& s, double& result) {
    if (!parseFactor(s, result)) return false;

    while (true) {
        skipSpaces(s);
        if (s.pos >= s.expr.size()) break;
        char op = s.expr[s.pos];
        if (op != '*' && op != '/') break;
        s.pos++;
        double right;
        if (!parseFactor(s, right)) return false;
        if (op == '*') result *= right;
        else {
            if (right == 0.0) return false; // división por cero
            result /= right;
        }
    }
    return true;
}

// Expresión: término (+ término | - término)*
static bool parseExpr(ParseState& s, double& result) {
    if (!parseTerm(s, result)) return false;

    while (true) {
        skipSpaces(s);
        if (s.pos >= s.expr.size()) break;
        char op = s.expr[s.pos];
        if (op != '+' && op != '-') break;
        s.pos++;
        double right;
        if (!parseTerm(s, right)) return false;
        if (op == '+') result += right;
        else           result -= right;
    }
    return true;
}

bool SparseMatrix::evalFormula(const std::string& expr, double& result) {
    ParseState s{expr, 0, this};
    if (!parseExpr(s, result)) return false;
    skipSpaces(s);
    return s.pos == s.expr.size(); // debe consumir TODA la expresión
}

void SparseMatrix::updateCache(int r, int c) {
    Node* curr = rowHeaders[r];
    while (curr && curr->col < c) curr = curr->right;

    if (!curr || curr->col != c) return;
    if (curr->rawValue.empty() || curr->rawValue[0] != '=') return;

    string expr = curr->rawValue.substr(1);  // quitar '='
    double val = 0;
    evalFormula(expr, val);
    curr->cacheValue = val;
}

// ─────────────────────────────────────────────────────────────
//  Operaciones básicas
// ─────────────────────────────────────────────────────────────

void SparseMatrix::set(int r, int c, string val) {
    if (r < 0 || r >= maxRows || c < 0 || c >= maxCols) return;

    // FIX: solo eliminamos si el valor está vacío; "0" es un valor válido
    if (val.empty()) {
        remove(r, c);
        return;
    }

    // Buscar posición en la fila
    Node* lastInRow  = nullptr;
    Node** prevRight = &rowHeaders[r];
    while (*prevRight && (*prevRight)->col < c) {
        lastInRow = *prevRight;
        prevRight = &((*prevRight)->right);
    }

    // Buscar posición en la columna
    Node* lastInCol = nullptr;
    Node** prevDown  = &colHeaders[c];
    while (*prevDown && (*prevDown)->row < r) {
        lastInCol = *prevDown;
        prevDown  = &((*prevDown)->down);
    }

    if (*prevRight && (*prevRight)->col == c) {
        // Nodo ya existe: actualizar valor
        (*prevRight)->rawValue = val;
        if (!val.empty() && val[0] == '=') updateCache(r, c);
    } else {
        // Crear nuevo nodo
        Node* newNode = new Node(r, c, val);

        newNode->right = *prevRight;
        newNode->left  = lastInRow;
        if (*prevRight) (*prevRight)->left = newNode;
        *prevRight = newNode;

        newNode->down = *prevDown;
        newNode->up   = lastInCol;
        if (*prevDown) (*prevDown)->up = newNode;
        *prevDown = newNode;

    }
    recalculateAll();
}

string SparseMatrix::get(int r, int c) {
    if (r < 0 || r >= maxRows || c < 0 || c >= maxCols) return "";
    Node* curr = rowHeaders[r];
    while (curr && curr->col < c) curr = curr->right;
    return (curr && curr->col == c) ? curr->rawValue : "";
}

double SparseMatrix::getNumericValue(int r, int c) {
    if (r < 0 || r >= maxRows || c < 0 || c >= maxCols) return 0;
    Node* curr = rowHeaders[r];
    while (curr && curr->col < c) curr = curr->right;
    if (curr && curr->col == c) {
        if (!curr->rawValue.empty() && curr->rawValue[0] == '=')
            return curr->cacheValue;
        try { return stod(curr->rawValue); }
        catch (...) { return 0; }
    }
    return 0;
}

int SparseMatrix::getRows() const { return maxRows; }
int SparseMatrix::getCols() const { return maxCols; }

void SparseMatrix::expandRows(int newMax) {
    if (newMax <= maxRows) return;
    Node** newHeaders = new Node*[newMax]();
    for (int i = 0; i < maxRows; ++i)
        newHeaders[i] = rowHeaders[i];
    delete[] rowHeaders;
    rowHeaders = newHeaders;
    maxRows = newMax;
}

void SparseMatrix::expandCols(int newMax) {
    if (newMax <= maxCols) return;
    Node** newHeaders = new Node*[newMax]();
    for (int i = 0; i < maxCols; ++i)
        newHeaders[i] = colHeaders[i];
    delete[] colHeaders;
    colHeaders = newHeaders;
    maxCols = newMax;
}

void SparseMatrix::remove(int r, int c) {
    if (r < 0 || r >= maxRows || c < 0 || c >= maxCols) return;

    // Recorrer la fila r hasta encontrar el nodo anterior al de columna c
    Node** prevRight = &rowHeaders[r];
    while (*prevRight && (*prevRight)->col < c)
        prevRight = &((*prevRight)->right);

    Node* target = *prevRight;
    if (!target || target->col != c) return; // el nodo no existe, nada que hacer

    // Recorrer la columna c hasta encontrar el nodo anterior al de fila r
    Node** prevDown = &colHeaders[c];
    while (*prevDown && (*prevDown)->row < r)
        prevDown = &((*prevDown)->down);

    // Desconectar de la fila: el nodo previo en la fila ahora apunta al siguiente de target,
    // y si existe un siguiente, su puntero left se actualiza para saltarse a target
    *prevRight = target->right;
    if (target->right) target->right->left = target->left;

    // Desconectar de la columna: el nodo previo en la columna ahora apunta al siguiente de target,
    // y si existe un siguiente, su puntero up se actualiza para saltarse a target
    *prevDown = target->down;
    if (target->down) target->down->up = target->up;

    delete target; // liberar memoria del nodo eliminado
}

// ─────────────────────────────────────────────────────────────
//  Persistencia
// ─────────────────────────────────────────────────────────────

void SparseMatrix::saveToFile(string filename) {
    ofstream file(filename);
    if (!file.is_open()) return;
    for (int i = 0; i < maxRows; ++i) {
        Node* curr = rowHeaders[i];
        while (curr) {
            // Escapar comas dentro del valor con comillas simples si es necesario
            file << curr->row << "," << curr->col << "," << curr->rawValue << "\n";
            curr = curr->right;
        }
    }
}

void SparseMatrix::loadFromFile(string filename) {
    ifstream file(filename);
    if (!file.is_open()) return;

    // Primera pasada: determinar dimensiones reales del archivo
    int maxR = 0, maxC = 0;
    vector<tuple<int,int,string>> entries;

    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        size_t p1 = line.find(',');
        if (p1 == string::npos) continue;
        size_t p2 = line.find(',', p1 + 1);
        if (p2 == string::npos) continue;
        try {
            int r   = stoi(line.substr(0, p1));
            int c   = stoi(line.substr(p1 + 1, p2 - p1 - 1));
            string v = line.substr(p2 + 1);
            maxR = max(maxR, r + 1);
            maxC = max(maxC, c + 1);
            entries.emplace_back(r, c, v);
        } catch (...) { continue; }
    }

    // Limpiar matriz actual completamente
    for (int i = 0; i < maxRows; ++i) {
        Node* curr = rowHeaders[i];
        while (curr) {
            Node* tmp = curr->right;
            delete curr;
            curr = tmp;
        }
        rowHeaders[i] = nullptr;
    }
    delete[] rowHeaders;
    delete[] colHeaders;

    // Redimensionar para acomodar todos los datos del archivo
    maxRows = max(maxRows, maxR);
    maxCols = max(maxCols, maxC);

    rowHeaders = new Node*[maxRows]();
    colHeaders = new Node*[maxCols]();

    // Segunda pasada: insertar datos
    for (auto& [r, c, v] : entries)
        set(r, c, v);
}

Node* SparseMatrix::getRowHeader(int r) const {
    if (r < 0 || r >= maxRows) return nullptr;
    return rowHeaders[r];
}

// ─────────────────────────────────────────────────────────────
//  Insertar / eliminar filas y columnas
// ─────────────────────────────────────────────────────────────

void SparseMatrix::addRow(int atPos) {
    if (atPos < 0 || atPos > maxRows) return;

    // Incrementar índice de fila en todos los nodos afectados
    for (int i = atPos; i < maxRows; ++i) {
        Node* curr = rowHeaders[i];
        while (curr) { curr->row++; curr = curr->right; }
    }

    Node** newRowHeaders = new Node*[maxRows + 1]();
    for (int i = 0; i < atPos;    ++i) newRowHeaders[i]     = rowHeaders[i];
    newRowHeaders[atPos] = nullptr;
    for (int i = atPos; i < maxRows; ++i) newRowHeaders[i + 1] = rowHeaders[i];

    delete[] rowHeaders;
    rowHeaders = newRowHeaders;
    maxRows++;
}

void SparseMatrix::addColumn(int atPos) {
    if (atPos < 0 || atPos > maxCols) return;

    for (int i = atPos; i < maxCols; ++i) {
        Node* curr = colHeaders[i];
        while (curr) { curr->col++; curr = curr->down; }
    }

    Node** newColHeaders = new Node*[maxCols + 1]();
    for (int i = 0; i < atPos;    ++i) newColHeaders[i]     = colHeaders[i];
    newColHeaders[atPos] = nullptr;
    for (int i = atPos; i < maxCols; ++i) newColHeaders[i + 1] = colHeaders[i];

    delete[] colHeaders;
    colHeaders = newColHeaders;
    maxCols++;
}

void SparseMatrix::removeRow(int r) {
    if (r < 0 || r >= maxRows) return;

    // Eliminar todos los nodos de la fila
    while (rowHeaders[r])
        remove(r, rowHeaders[r]->col);

    // Decrementar índices de filas posteriores
    for (int i = r + 1; i < maxRows; ++i) {
        Node* curr = rowHeaders[i];
        while (curr) { curr->row--; curr = curr->right; }
    }

    // Compactar array de cabeceras
    Node** newRowHeaders = new Node*[maxRows - 1]();
    for (int i = 0; i < r; ++i)           newRowHeaders[i]     = rowHeaders[i];
    for (int i = r + 1; i < maxRows; ++i)  newRowHeaders[i - 1] = rowHeaders[i];

    delete[] rowHeaders;
    rowHeaders = newRowHeaders;
    maxRows--;
}

void SparseMatrix::removeColumn(int c) {
    if (c < 0 || c >= maxCols) return;

    while (colHeaders[c])
        remove(colHeaders[c]->row, c);

    for (int i = c + 1; i < maxCols; ++i) {
        Node* curr = colHeaders[i];
        while (curr) { curr->col--; curr = curr->down; }
    }

    Node** newColHeaders = new Node*[maxCols - 1]();
    for (int i = 0; i < c; ++i)           newColHeaders[i]     = colHeaders[i];
    for (int i = c + 1; i < maxCols; ++i)  newColHeaders[i - 1] = colHeaders[i];

    delete[] colHeaders;
    colHeaders = newColHeaders;
    maxCols--;
}

void SparseMatrix::removeRange(int r1, int c1, int r2, int c2) {
    for (int i = r1; i <= r2; ++i) {
        if (i < 0 || i >= maxRows) continue;
        Node* curr = rowHeaders[i];
        while (curr) {
            Node* next = curr->right;
            if (curr->col >= c1 && curr->col <= c2)
                remove(i, curr->col);
            curr = next;
        }
    }
}

// ─────────────────────────────────────────────────────────────
//  Operaciones de rango
// ─────────────────────────────────────────────────────────────

double SparseMatrix::sumRange(int r1, int c1, int r2, int c2) {
    double total = 0;
    for (int i = r1; i <= r2; ++i) {
        if (i < 0 || i >= maxRows) continue;
        Node* curr = rowHeaders[i];
        while (curr && curr->col <= c2) {
            if (curr->col >= c1)
                total += getNumericValue(i, curr->col);
            curr = curr->right;
        }
    }
    return total;
}

double SparseMatrix::averageRange(int r1, int c1, int r2, int c2) {
    double total = 0;
    int    count = 0;
    for (int i = r1; i <= r2; ++i) {
        if (i < 0 || i >= maxRows) continue;
        Node* curr = rowHeaders[i];
        while (curr && curr->col <= c2) {
            if (curr->col >= c1) {
                // Ignorar celdas de texto puro (no numéricas ni fórmulas)
                const std::string& raw = curr->rawValue;
                bool isFormula = !raw.empty() && raw[0] == '=';
                bool isNumber  = false;
                if (!isFormula) {
                    try { std::stod(raw); isNumber = true; }
                    catch (...) {}
                }
                if (isFormula || isNumber) {
                    total += getNumericValue(i, curr->col);
                    ++count;
                }
            }
            curr = curr->right;
        }
    }
    return (count == 0) ? 0 : total / count;
}

double SparseMatrix::maxInRange(int r1, int c1, int r2, int c2) {
    double maxVal = -1e18;
    bool   found  = false;
    for (int i = r1; i <= r2; ++i) {
        if (i < 0 || i >= maxRows) continue;
        Node* curr = rowHeaders[i];
        while (curr && curr->col <= c2) {
            if (curr->col >= c1) {
                // Ignorar celdas de texto puro (no numéricas ni fórmulas)
                const std::string& raw = curr->rawValue;
                bool isFormula = !raw.empty() && raw[0] == '=';
                bool isNumber  = false;
                if (!isFormula) {
                    try { std::stod(raw); isNumber = true; }
                    catch (...) {}
                }
                if (isFormula || isNumber) {
                    double val = getNumericValue(i, curr->col);
                    if (val > maxVal) maxVal = val;
                    found = true;
                }
            }
            curr = curr->right;
        }
    }
    return found ? maxVal : 0;
}

double SparseMatrix::minInRange(int r1, int c1, int r2, int c2) {
    double minVal = 1e18;
    bool   found  = false;
    for (int i = r1; i <= r2; ++i) {
        if (i < 0 || i >= maxRows) continue;
        Node* curr = rowHeaders[i];
        while (curr && curr->col <= c2) {
            if (curr->col >= c1) {
                // Ignorar celdas de texto puro (no numéricas ni fórmulas)
                const std::string& raw = curr->rawValue;
                bool isFormula = !raw.empty() && raw[0] == '=';
                bool isNumber  = false;
                if (!isFormula) {
                    try { std::stod(raw); isNumber = true; }
                    catch (...) {}
                }
                if (isFormula || isNumber) {
                    double val = getNumericValue(i, curr->col);
                    if (val < minVal) minVal = val;
                    found = true;
                }
            }
            curr = curr->right;
        }
    }
    return found ? minVal : 0;
}

void SparseMatrix::recalculateAll() {
    for (int i = 0; i < maxRows; ++i) {
        Node* curr = rowHeaders[i];
        while (curr) {
            if (!curr->rawValue.empty() && curr->rawValue[0] == '=')
                updateCache(curr->row, curr->col);
            curr = curr->right;
        }
    }
}

// ─────────────────────────────────────────────────────────────
//  Debug
// ─────────────────────────────────────────────────────────────

void SparseMatrix::display() {
    for (int i = 0; i < maxRows; ++i) {
        Node* curr = rowHeaders[i];
        for (int j = 0; j < maxCols; ++j) {
            if (curr && curr->col == j) {
                cout << setw(10) << curr->rawValue;
                curr = curr->right;
            } else {
                cout << setw(10) << ".";
            }
        }
        cout << "\n";
    }
}