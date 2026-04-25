//
// Created by Usuario on 24/04/2026.
//

#ifndef AED_PROYECTO_MAINWINDOW_H
#define AED_PROYECTO_MAINWINDOW_H

#pragma once

#include <QMainWindow>
#include <QTableWidget>
#include <QLineEdit>
#include <QLabel>
#include <QStatusBar>
#include <QPushButton>
#include <QToolBar>
#include <QMenuBar>
#include "SparseMatrix.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Celda
    void onCellClicked(int row, int col);
    void onCellChanged(int row, int col);
    void onFormulaConfirmed();

    // Filas y columnas
    void onAddRowBefore();
    void onAddRowAfter();
    void onAddColBefore();
    void onAddColAfter();
    void onRemoveRow();
    void onRemoveCol();
    void onRemoveRange();

    // Operaciones de rango
    void onSumRange();
    void onAverageRange();
    void onMaxRange();
    void onMinRange();

    // Archivo
    void onNew();
    void onSave();
    void onLoad();

private:
    // ── Widgets ──────────────────────────────────────────
    QTableWidget *tableWidget;
    QLineEdit    *formulaBar;
    QLabel       *cellRefLabel;

    // ── Datos ─────────────────────────────────────────────
    SparseMatrix *matrix;
    bool          blockTableSignals; // evita recursión en cellChanged
    int           currentRow;
    int           currentCol;
    int           numRows;           // filas actuales de la matriz
    int           numCols;           // columnas actuales de la matriz

    static const int INIT_ROWS = 50;
    static const int INIT_COLS = 26; // A-Z

    // ── Helpers ───────────────────────────────────────────
    void setupUI();
    void setupMenuBar();
    void setupToolBar();

    void refreshTable();             // redibuja toda la tabla desde la matriz
    void refreshCell(int r, int c);  // redibuja una sola celda

    QString  getCellRef(int row, int col);               // "A1", "B3", etc.
    QString  formatDisplayValue(const std::string &raw, int r, int c);
    bool     getSelectionRange(int &r1, int &c1, int &r2, int &c2);
    void     showStatus(const QString &msg, int ms = 3000);
    void     syncTableHeaders();     // actualiza cabeceras de fila y columna
};


#endif //AED_PROYECTO_MAINWINDOW_H
