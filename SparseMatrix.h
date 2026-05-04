#pragma once
#include <string>
#include <vector>
#include <tuple>

struct Node {
    int row, col;
    std::string rawValue;
    double cacheValue = 0.0;

    Node* left  = nullptr;
    Node* right = nullptr;
    Node* up    = nullptr;
    Node* down  = nullptr;

    Node(int r, int c, const std::string& v)
        : row(r), col(c), rawValue(v) {}
};

class SparseMatrix {
public:
    SparseMatrix(int rows, int cols);
    ~SparseMatrix();

    // Acceso y modificación
    void        set(int r, int c, std::string val);
    std::string get(int r, int c);
    double      getNumericValue(int r, int c);
    void        remove(int r, int c);

    // Dimensiones
    int getRows() const;
    int getCols() const;

    // Persistencia
    void saveToFile(std::string filename);
    void loadFromFile(std::string filename);

    // Estructura
    void addRow(int atPos);
    void addColumn(int atPos);
    void removeRow(int r);
    void removeColumn(int c);
    void removeRange(int r1, int c1, int r2, int c2);

    // Operaciones de rango
    double sumRange    (int r1, int c1, int r2, int c2);
    double averageRange(int r1, int c1, int r2, int c2);
    double maxInRange  (int r1, int c1, int r2, int c2);
    double minInRange  (int r1, int c1, int r2, int c2);
    Node* getRowHeader(int r) const;

    // Debug
    void display();

private:
    int    maxRows;
    int    maxCols;
    Node** rowHeaders;
    Node** colHeaders;

    void updateCache(int r, int c);
    void recalculateAll();
    bool evalFormula(const std::string& expr, double& result);
};