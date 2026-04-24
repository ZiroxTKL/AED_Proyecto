#include "Node.h"
#include <string>
#include <vector>

class SparseMatrix {
private:
    Node** rowHeaders;
    Node** colHeaders;
    int maxRows;
    int maxCols;

public:
    SparseMatrix(int rows, int cols);
    ~SparseMatrix();

    void updateCache(int r, int c);

    // Soporte para string, numero y formulas con string
    void set(int r, int c, std::string val); 
    std::string get(int r, int c); 
    double getNumericValue(int r, int c);

    void remove(int r, int c);

    // Guardar y cargar
    void saveToFile(std::string filename);
    void loadFromFile(std::string filename);

    // Añadir y remover
    void addRow(int atPos);
    void addColumn(int atPos);
    void removeRow(int r);
    void removeColumn(int c);
    void removeRange(int r1, int c1, int r2, int c2);

    // Operaciones
    double sumRange(int r1, int c1, int r2, int c2);
    double averageRange(int r1, int c1, int r2, int c2);
    double maxInRange(int r1, int c1, int r2, int c2);
    double minInRange(int r1, int c1, int r2, int c2);

    void display();
};