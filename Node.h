#include <iostream>

struct Node {
    std::string rawValue; // Guardar texto, numero o formula
    double cacheValue;    // Guardar resultaldo de la formula
    int row, col;
    Node *up, *down, *left, *right;

    Node(int r, int c, std::string val) 
        : row(r), col(c), rawValue(val), cacheValue(0),
          up(nullptr), down(nullptr), left(nullptr), right(nullptr) {}
};