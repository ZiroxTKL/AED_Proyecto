#include "SparseMatrix.h"
#include <iostream>
#include <iomanip>
#include <fstream> 

using namespace std;

SparseMatrix::SparseMatrix(int rows, int cols) : maxRows(rows), maxCols(cols) {
    rowHeaders = new Node*[rows];
    colHeaders = new Node*[cols];
    for (int i = 0; i < rows; ++i) rowHeaders[i] = nullptr;
    for (int i = 0; i < cols; ++i) colHeaders[i] = nullptr;
}

SparseMatrix::~SparseMatrix() {
    for (int i = 0; i < maxRows; ++i) {
        Node* curr = rowHeaders[i];
        while (curr) {
            Node* temp = curr;
            curr = curr->right;
            delete temp;
        }
    }
    delete[] rowHeaders;
    delete[] colHeaders;
}

void SparseMatrix::updateCache(int r, int c) {
    Node* curr = rowHeaders[r];
    while (curr && curr->col < c) curr = curr->right;
    
    if (!curr || curr->col != c || curr->rawValue.empty() || curr->rawValue[0] != '=') {
        return;
    }

    string formula = curr->rawValue;
    formula = formula.substr(1); 

    size_t plusPos = formula.find('+');
    if (plusPos != string::npos) {
        string celda1 = formula.substr(0, plusPos);
        string celda2 = formula.substr(plusPos + 1);

        int c1_col = celda1[0] - 'A';
        int c1_row = stoi(celda1.substr(1)) - 1;

        int c2_col = celda2[0] - 'A';
        int c2_row = stoi(celda2.substr(1)) - 1;

        double val1 = getNumericValue(c1_row, c1_col);
        double val2 = getNumericValue(c2_row, c2_col);

        curr->cacheValue = val1 + val2;
    }
}

void SparseMatrix::set(int r, int c, string val) {
    if (r < 0 || r >= maxRows || c < 0 || c >= maxCols) return;

    if (val == "" || val == "0") {
        remove(r, c);
        return;
    }

    // Buscar posición en la Fila
    Node* lastInRow = nullptr;
    Node** prevRight = &rowHeaders[r];
    while (*prevRight && (*prevRight)->col < c) {
        lastInRow = *prevRight;
        prevRight = &((*prevRight)->right);
    }

    // Buscar posición en la Columna
    Node* lastInCol = nullptr;
    Node** prevDown = &colHeaders[c];
    while (*prevDown && (*prevDown)->row < r) {
        lastInCol = *prevDown;
        prevDown = &((*prevDown)->down);
    }

    // Verificar Nodo
    if (*prevRight && (*prevRight)->col == c) {
        (*prevRight)->rawValue = val;
        // Si se actualizó a una fórmula, recalculamos
        if (val.length() > 0 && val[0] == '=') updateCache(r, c); 
    } else {
        Node* newNode = new Node(r, c, val);

        // Reorganizar Nodos (Horizontal)
        newNode->right = *prevRight;
        newNode->left = lastInRow;
        if (*prevRight) {
            (*prevRight)->left = newNode;
        }
        *prevRight = newNode;

        // Reorganizar Nodos (Vertical)
        newNode->down = *prevDown;
        newNode->up = lastInCol;
        if (*prevDown) {
            (*prevDown)->up = newNode;
        }
        *prevDown = newNode;
        
        // Si se insertó una fórmula, calculamos su caché
        if (val.length() > 0 && val[0] == '=') updateCache(r, c);
    }
}

string SparseMatrix::get(int r, int c) {
    if (r < 0 || r >= maxRows || c < 0 || c >= maxCols) return "";

    Node* curr = rowHeaders[r];
    while (curr && curr->col < c) curr = curr->right;

    if (curr && curr->col == c) return curr->rawValue;
    return "";
}

double SparseMatrix::getNumericValue(int r, int c) {
    if (r < 0 || r >= maxRows || c < 0 || c >= maxCols) return 0;
    
    Node* curr = rowHeaders[r];
    while (curr && curr->col < c) curr = curr->right;

    if (curr && curr->col == c) {
        if (!curr->rawValue.empty() && curr->rawValue[0] == '=') {
            return curr->cacheValue; 
        }
        try {
            return std::stod(curr->rawValue);
        } catch (...) {
            return 0; 
        }
    }
    return 0;
}

void SparseMatrix::remove(int r, int c) {
    if (r < 0 || r >= maxRows || c < 0 || c >= maxCols) return;

    Node** prevRight = &rowHeaders[r];
    while (*prevRight && (*prevRight)->col < c) {
        prevRight = &((*prevRight)->right);
    }

    Node* target = *prevRight;
    if (!target || target->col != c) return;

    Node** prevDown = &colHeaders[c];
    while (*prevDown && (*prevDown)->row < r) {
        prevDown = &((*prevDown)->down);
    }

    // Desconectar fila
    *prevRight = target->right;
    if (target->right) target->right->left = target->left;

    // Desconectar columna
    *prevDown = target->down;
    if (target->down) target->down->up = target->up;

    delete target;
}

// Guardar y cargar
void SparseMatrix::saveToFile(string filename) {
    ofstream file(filename);
    if (!file.is_open()) return;

    for (int i = 0; i < maxRows; ++i) {
        Node* curr = rowHeaders[i];
        while (curr) {
            file << curr->row << "," << curr->col << "," << curr->rawValue << "\n";
            curr = curr->right;
        }
    }
    file.close();
}

void SparseMatrix::loadFromFile(string filename) {
    ifstream file(filename);
    if (!file.is_open()) return;

    // Limpiar matriz actual antes de cargar
    for (int i = 0; i < maxRows; ++i) removeRow(i);

    string line;
    while (getline(file, line)) {
        size_t p1 = line.find(',');
        size_t p2 = line.find(',', p1 + 1);
        int r = stoi(line.substr(0, p1));
        int c = stoi(line.substr(p1 + 1, p2 - p1 - 1));
        string val = line.substr(p2 + 1);
        set(r, c, val);
    }
}  

// Añadir y remover
void SparseMatrix::addRow(int atPos) {
    if (atPos < 0 || atPos > maxRows) return;

    // Actualizar los índices internos de todos los nodos afectados
    for (int i = atPos; i < maxRows; ++i) {
        Node* curr = rowHeaders[i];
        while (curr) {
            curr->row++;
            curr = curr->right;
        }
    }

    // Reasignar el array de cabeceras de fila
    Node** newRowHeaders = new Node*[maxRows + 1];
    for (int i = 0; i < atPos; ++i) newRowHeaders[i] = rowHeaders[i];
    newRowHeaders[atPos] = nullptr;
    for (int i = atPos; i < maxRows; ++i) newRowHeaders[i + 1] = rowHeaders[i];

    delete[] rowHeaders;
    rowHeaders = newRowHeaders;
    maxRows++;
}

void SparseMatrix::addColumn(int atPos) {
    if (atPos < 0 || atPos > maxCols) return;

    // Actualizar índices de columna en todos los nodos afectados
    for (int i = atPos; i < maxCols; ++i) {
        Node* curr = colHeaders[i];
        while (curr) {
            curr->col++;
            curr = curr->down;
        }
    }

    // Reasignar el array de cabeceras de columna
    Node** newColHeaders = new Node*[maxCols + 1];
    for (int i = 0; i < atPos; ++i) newColHeaders[i] = colHeaders[i];
    newColHeaders[atPos] = nullptr;
    for (int i = atPos; i < maxCols; ++i) newColHeaders[i + 1] = colHeaders[i];

    delete[] colHeaders;
    colHeaders = newColHeaders;
    maxCols++;
}

void SparseMatrix::removeRow(int r) {
    if (r < 0 || r >= maxRows) return;
    while (rowHeaders[r]) {
        remove(r, rowHeaders[r]->col);
    }
}

void SparseMatrix::removeColumn(int c) {
    if (c < 0 || c >= maxCols) return;
    while (colHeaders[c]) {
        remove(colHeaders[c]->row, c);
    }
}

void SparseMatrix::removeRange(int r1, int c1, int r2, int c2) {
    for (int i = r1; i <= r2; ++i) {
        if (i < 0 || i >= maxRows) continue;
        
        Node* curr = rowHeaders[i];
        while (curr) {
             // Guardamos el siguiente antes de borrar
            Node* nextNode = curr->right;
            if (curr->col >= c1 && curr->col <= c2) {
                remove(i, curr->col);
            }
            curr = nextNode;
        }
    }
}

// Operaciones
double SparseMatrix::sumRange(int r1, int c1, int r2, int c2) {
    double total = 0;
    for (int i = r1; i <= r2; ++i) {
        if (i < 0 || i >= maxRows) continue;
        Node* curr = rowHeaders[i];
        while (curr && curr->col <= c2) {
            if (curr->col >= c1) {
                total += getNumericValue(i, curr->col);
            }
            curr = curr->right;
        }
    }
    return total;
}

double SparseMatrix::averageRange(int r1, int c1, int r2, int c2) {
    double total = 0;
    int count = 0;
    for (int i = r1; i <= r2; ++i) {
        if (i < 0 || i >= maxRows) continue;
        Node* curr = rowHeaders[i];
        while (curr && curr->col <= c2) {
            if (curr->col >= c1) {
                total += getNumericValue(i, curr->col);
                count++;
            }
            curr = curr->right;
        }
    }
    return (count == 0) ? 0 : total / count;
}

double SparseMatrix::maxInRange(int r1, int c1, int r2, int c2) {
     // Valor muy pequeño
    double maxVal = -1e18;
    bool found = false;
    for (int i = r1; i <= r2; ++i) {
        if (i < 0 || i >= maxRows) continue;
        Node* curr = rowHeaders[i];
        while (curr && curr->col <= c2) {
            if (curr->col >= c1) {
                double val = getNumericValue(i, curr->col);
                if (val > maxVal) maxVal = val;
                found = true;
            }
            curr = curr->right;
        }
    }
    return found ? maxVal : 0;
}

double SparseMatrix::minInRange(int r1, int c1, int r2, int c2) {
    // Valor muy grande
    double minVal = 1e18; 
    bool found = false;
    for (int i = r1; i <= r2; ++i) {
        if (i < 0 || i >= maxRows) continue;
        Node* curr = rowHeaders[i];
        while (curr && curr->col <= c2) {
            if (curr->col >= c1) {
                double val = getNumericValue(i, curr->col);
                if (val < minVal) minVal = val;
                found = true;
            }
            curr = curr->right;
        }
    }
    return found ? minVal : 0;
}

void SparseMatrix::display() {
    for (int i = 0; i < maxRows; ++i) {
        Node* curr = rowHeaders[i];
        for (int j = 0; j < maxCols; ++j) {
            if (curr && curr->col == j) {
                cout << setw(8) << curr->rawValue << " ";
                curr = curr->right;
            } else {
                cout << setw(8) << "." << " ";
            }
        }
        cout << endl;
    }
}