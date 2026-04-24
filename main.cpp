#include <iostream>
#include <string>
#include "SparseMatrix.h"

using namespace std;

int main() {
    // 1. Inicialización de una hoja de 10x10
    cout << "--- 1. Inicializando Matriz Dispersa (10x10) ---" << endl;
    SparseMatrix sheet(10, 10);

    // 2. Probar inserción de valores numéricos y texto
    cout << "\n--- 2. Insertando datos mixtos ---" << endl;
    sheet.set(0, 0, "10.5");  // A1
    sheet.set(0, 1, "20");    // B1
    sheet.set(1, 0, "Texto"); // A2
    sheet.set(1, 1, "30.5");  // B2
    
    sheet.display();

    // 3. Probar obtención de valores
    cout << "\n--- 3. Verificando valores individuales ---" << endl;
    cout << "Celda A1 (num): " << sheet.getNumericValue(0, 0) << endl;
    cout << "Celda A2 (texto): " << sheet.get(1, 0) << endl;

    // 4. Probar Fórmulas (Requisito 12)
    cout << "\n--- 4. Probando Fórmulas (=A1+B1) ---" << endl;
    sheet.set(2, 0, "=A1+B1"); // C1 (en coordenadas es 2,0 si usamos letra-número)
    // Nota: El parser de tu updateCache usa letras para columnas, 
    // asegúrate que coincida con tus índices.
    
    cout << "Resultado fórmula en C1: " << sheet.getNumericValue(2, 0) << endl;

    // 5. Probar Operaciones de Rango
    cout << "\n--- 5. Operaciones de Rango (A1:B2) ---" << endl;
    cout << "Suma del rango (0,0) a (1,1): " << sheet.sumRange(0, 0, 1, 1) << endl;
    cout << "Promedio del rango (0,0) a (1,1): " << sheet.averageRange(0, 0, 1, 1) << endl;
    cout << "Máximo en rango: " << sheet.maxInRange(0, 0, 1, 1) << endl;

    // 6. Probar Modificación Estructural
    cout << "\n--- 6. Insertando Fila en posición 1 ---" << endl;
    sheet.addRow(1); 
    // Ahora lo que estaba en fila 1 (A2, B2) debería estar en fila 2.
    cout << "Nuevo valor en (2,0) [antes era A2]: " << sheet.get(2, 0) << endl;
    sheet.display();

    // 7. Probar Persistencia
    cout << "\n--- 7. Probando Guardado y Carga ---" << endl;
    sheet.saveToFile("hoja67.csv");
    cout << "Archivo 'mi_hoja.csv' guardado." << endl;

    SparseMatrix nuevaSheet(10, 10);
    nuevaSheet.loadFromFile("mi_hoja.csv");
    cout << "Matriz cargada desde archivo:" << endl;
    nuevaSheet.display();

    cout << "\n--- Pruebas finalizadas con éxito ---" << endl;

    return 0;
}