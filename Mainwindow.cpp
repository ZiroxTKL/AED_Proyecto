#include "Mainwindow.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QMenu>
#include <QAction>
#include <QFont>
#include <QColor>
#include <QPalette>
#include <QFrame>
#include <QSizePolicy>

#include <sstream>
#include <iomanip>
#include <cmath>

// ─────────────────────────────────────────────────────────────
//  Paleta de colores (estilo Excel 365)
// ─────────────────────────────────────────────────────────────
namespace ExcelTheme {
    // Estructura
    const char* HEADER_BG       = "#F2F2F2";   // cabeceras de fila/col
    const char* HEADER_BORDER   = "#D0D0D0";
    const char* HEADER_TEXT     = "#444444";
    const char* HEADER_HOVER    = "#E6E6E6";
    const char* HEADER_SELECTED = "#107C41";   // verde Excel al seleccionar
    const char* HEADER_SEL_TEXT = "#FFFFFF";

    // Celdas
    const char* CELL_BG         = "#FFFFFF";
    const char* CELL_GRID       = "#D0D0D0";
    const char* CELL_TEXT       = "#212121";
    const char* CELL_SEL_BG     = "#CDE8D8";   // selección verde pálido
    const char* CELL_SEL_TEXT   = "#212121";

    // Celdas con fórmula
    const char* FORMULA_BG      = "#EEF4FF";   // azul muy tenue
    const char* FORMULA_TEXT    = "#0D47A1";   // azul oscuro

    // Barra de fórmulas
    const char* FX_BAR_BG       = "#FFFFFF";
    const char* FX_BAR_BORDER   = "#D0D0D0";
    const char* FX_ICON_COLOR   = "#107C41";   // verde Excel
    const char* FX_REF_BG       = "#F2F2F2";
    const char* FX_REF_BORDER   = "#C0C0C0";

    // Menú / Toolbar
    const char* TOOLBAR_BG      = "#217346";   // verde Excel oscuro
    const char* TOOLBAR_TEXT    = "#FFFFFF";
    const char* TOOLBAR_HOVER   = "#185C37";
    const char* TOOLBAR_PRESSED = "#0E3D25";
    const char* MENU_BG         = "#FFFFFF";
    const char* MENU_BORDER     = "#C8C8C8";
    const char* MENU_SEL_BG     = "#107C41";
    const char* MENU_SEL_TEXT   = "#FFFFFF";
    const char* MENUBAR_BG      = "#217346";
    const char* MENUBAR_TEXT    = "#FFFFFF";
    const char* MENUBAR_HOVER   = "#185C37";

    // Status bar
    const char* STATUS_BG       = "#217346";
    const char* STATUS_TEXT     = "#FFFFFF";
}

// ─────────────────────────────────────────────────────────────
//  Constructor / Destructor
// ─────────────────────────────────────────────────────────────

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , blockTableSignals(false)
    , currentRow(0)
    , currentCol(0)
    , numRows(INIT_ROWS)
    , numCols(INIT_COLS)
{
    matrix = new SparseMatrix(numRows, numCols);

    setupUI();
    setupMenuBar();
    setupToolBar();
    refreshTable();

    setWindowTitle("Hoja de Cálculo — Matriz Dispersa");
    resize(1280, 720);
    showStatus("Listo. Selecciona una celda para comenzar.");
}

MainWindow::~MainWindow() {
    delete matrix;
}

// ─────────────────────────────────────────────────────────────
//  Construcción de la interfaz
// ─────────────────────────────────────────────────────────────

void MainWindow::setupUI() {
    // Paleta global: fondo neutro, texto oscuro
    QPalette pal = QApplication::palette();
    pal.setColor(QPalette::Window,        QColor(ExcelTheme::CELL_BG));
    pal.setColor(QPalette::WindowText,    QColor(ExcelTheme::CELL_TEXT));
    pal.setColor(QPalette::Base,          QColor(ExcelTheme::CELL_BG));
    pal.setColor(QPalette::Text,          QColor(ExcelTheme::CELL_TEXT));
    pal.setColor(QPalette::Highlight,     QColor(ExcelTheme::CELL_SEL_BG));
    pal.setColor(QPalette::HighlightedText, QColor(ExcelTheme::CELL_SEL_TEXT));
    QApplication::setPalette(pal);

    // Fuente base
    QFont appFont("Calibri", 11);
    QApplication::setFont(appFont);

    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // ── Barra de fórmulas ──────────────────────────────────
    QWidget *fxBar = new QWidget();
    fxBar->setFixedHeight(34);
    fxBar->setStyleSheet(
        QString("background:%1; border-bottom:1px solid %2;")
            .arg(ExcelTheme::FX_BAR_BG)
            .arg(ExcelTheme::FX_BAR_BORDER)
    );
    QHBoxLayout *fxLayout = new QHBoxLayout(fxBar);
    fxLayout->setContentsMargins(6, 2, 6, 2);
    fxLayout->setSpacing(4);

    cellRefLabel = new QLabel("A1");
    cellRefLabel->setFixedWidth(56);
    cellRefLabel->setAlignment(Qt::AlignCenter);
    cellRefLabel->setStyleSheet(
        QString("font-weight:bold; font-size:12px; color:%1;"
                "border:1px solid %2; border-radius:2px;"
                "background:%3; padding:2px 4px;")
            .arg(ExcelTheme::CELL_TEXT)
            .arg(ExcelTheme::FX_REF_BORDER)
            .arg(ExcelTheme::FX_REF_BG)
    );

    QLabel *fxIcon = new QLabel("ƒx");
    fxIcon->setFixedWidth(26);
    fxIcon->setAlignment(Qt::AlignCenter);
    fxIcon->setStyleSheet(
        QString("color:%1; font-weight:bold; font-size:14px;")
            .arg(ExcelTheme::FX_ICON_COLOR)
    );

    formulaBar = new QLineEdit();
    formulaBar->setPlaceholderText("Ingresa un valor, texto o fórmula (=A1+B1) …");
    formulaBar->setStyleSheet(
        QString("border:1px solid %1; border-radius:2px;"
                "padding:2px 6px; font-size:12px;"
                "background:%2; color:%3;"
                "selection-background-color:%4; selection-color:%5;")
            .arg(ExcelTheme::FX_REF_BORDER)
            .arg(ExcelTheme::FX_BAR_BG)
            .arg(ExcelTheme::CELL_TEXT)
            .arg(ExcelTheme::CELL_SEL_BG)
            .arg(ExcelTheme::CELL_SEL_TEXT)
    );
    connect(formulaBar, &QLineEdit::returnPressed,
            this,       &MainWindow::onFormulaConfirmed);

    fxLayout->addWidget(cellRefLabel);
    fxLayout->addWidget(fxIcon);
    fxLayout->addWidget(formulaBar);

    // ── Tabla ─────────────────────────────────────────────
    tableWidget = new QTableWidget(numRows, numCols, this);
    tableWidget->setSelectionMode(QAbstractItemView::ContiguousSelection);
    tableWidget->setEditTriggers(QAbstractItemView::DoubleClicked
                               | QAbstractItemView::AnyKeyPressed);

    tableWidget->horizontalHeader()->setDefaultSectionSize(90);
    tableWidget->verticalHeader()->setDefaultSectionSize(22);
    tableWidget->horizontalHeader()->setMinimumSectionSize(30);
    tableWidget->verticalHeader()->setMinimumSectionSize(18);

    // Resaltar la cabecera de la columna/fila seleccionada
    tableWidget->horizontalHeader()->setHighlightSections(true);
    tableWidget->verticalHeader()->setHighlightSections(true);

    tableWidget->setStyleSheet(
        QString(R"(
        QTableWidget {
            gridline-color: %1;
            background-color: %2;
            color: %3;
            font-size: 12px;
            border: none;
        }
        QTableWidget::item {
            padding: 0 4px;
            color: %3;
            background-color: %2;
        }
        QTableWidget::item:selected {
            background-color: %4;
            color: %5;
        }
        QHeaderView::section {
            background-color: %6;
            color: %7;
            border: 0px;
            border-right: 1px solid %8;
            border-bottom: 1px solid %8;
            font-size: 12px;
            font-weight: 500;
            padding: 2px 4px;
        }
        QHeaderView::section:hover {
            background-color: %9;
        }
        QHeaderView::section:checked {
            background-color: %10;
            color: %11;
            font-weight: bold;
        }
        )")
        .arg(ExcelTheme::CELL_GRID)        // %1 gridline
        .arg(ExcelTheme::CELL_BG)          // %2 cell bg
        .arg(ExcelTheme::CELL_TEXT)        // %3 cell text
        .arg(ExcelTheme::CELL_SEL_BG)      // %4 selected bg
        .arg(ExcelTheme::CELL_SEL_TEXT)    // %5 selected text
        .arg(ExcelTheme::HEADER_BG)        // %6 header bg
        .arg(ExcelTheme::HEADER_TEXT)      // %7 header text
        .arg(ExcelTheme::HEADER_BORDER)    // %8 header border
        .arg(ExcelTheme::HEADER_HOVER)     // %9 header hover
        .arg(ExcelTheme::HEADER_SELECTED)  // %10 selected header bg
        .arg(ExcelTheme::HEADER_SEL_TEXT)  // %11 selected header text
    );

    syncTableHeaders();

    connect(tableWidget, &QTableWidget::cellClicked,
            this,        &MainWindow::onCellClicked);
    connect(tableWidget, &QTableWidget::cellChanged,
            this,        &MainWindow::onCellChanged);

    mainLayout->addWidget(fxBar);
    mainLayout->addWidget(tableWidget);

    // Status bar con fondo verde Excel
    QStatusBar *sb = new QStatusBar(this);
    sb->setStyleSheet(
        QString("QStatusBar { background:%1; color:%2; font-size:11px; }"
                "QStatusBar::item { border:none; }")
            .arg(ExcelTheme::STATUS_BG)
            .arg(ExcelTheme::STATUS_TEXT)
    );
    setStatusBar(sb);
    setCentralWidget(central);
}

void MainWindow::setupMenuBar() {
    QMenuBar *mb = menuBar();
    mb->setStyleSheet(
        QString("QMenuBar { background:%1; color:%2; font-size:12px; }"
                "QMenuBar::item { padding:4px 10px; color:%2; background:transparent; }"
                "QMenuBar::item:selected { background:%3; border-radius:3px; }"
                "QMenuBar::item:pressed { background:%3; }"
                "QMenu { background:%4; color:%5; border:1px solid %6; font-size:12px; }"
                "QMenu::item { padding:5px 24px 5px 24px; color:%5; }"
                "QMenu::item:selected { background:%7; color:%8; }"
                "QMenu::separator { height:1px; background:%6; margin:3px 10px; }")
            .arg(ExcelTheme::MENUBAR_BG)       // %1 menubar bg
            .arg(ExcelTheme::MENUBAR_TEXT)     // %2 menubar text
            .arg(ExcelTheme::MENUBAR_HOVER)    // %3 menubar hover
            .arg(ExcelTheme::MENU_BG)          // %4 menu bg
            .arg(ExcelTheme::CELL_TEXT)        // %5 menu text  ← texto OSCURO visible
            .arg(ExcelTheme::MENU_BORDER)      // %6 menu border
            .arg(ExcelTheme::MENU_SEL_BG)      // %7 menu sel bg
            .arg(ExcelTheme::MENU_SEL_TEXT)    // %8 menu sel text
    );

    // Archivo
    QMenu *fileMenu = mb->addMenu("&Archivo");
    fileMenu->addAction("&Nuevo",    QKeySequence::New,  this, &MainWindow::onNew);
    fileMenu->addAction("&Abrir…",   QKeySequence::Open, this, &MainWindow::onLoad);
    fileMenu->addAction("&Guardar…", QKeySequence::Save, this, &MainWindow::onSave);
    fileMenu->addSeparator();
    fileMenu->addAction("&Salir", QKeySequence::Quit, qApp, &QApplication::quit);

    // Editar
    QMenu *editMenu = mb->addMenu("&Editar");
    editMenu->addAction("Insertar fila &antes",   this, &MainWindow::onAddRowBefore);
    editMenu->addAction("Insertar fila &después", this, &MainWindow::onAddRowAfter);
    editMenu->addAction("&Eliminar fila",         this, &MainWindow::onRemoveRow);
    editMenu->addSeparator();
    editMenu->addAction("Insertar columna a la iz&quierda", this, &MainWindow::onAddColBefore);
    editMenu->addAction("Insertar columna a la &derecha",   this, &MainWindow::onAddColAfter);
    editMenu->addAction("E&liminar columna",                this, &MainWindow::onRemoveCol);
    editMenu->addSeparator();
    editMenu->addAction("Eliminar &rango seleccionado",
                        QKeySequence::Delete, this, &MainWindow::onRemoveRange);

    // Fórmulas
    QMenu *fmlMenu = mb->addMenu("&Fórmulas");
    fmlMenu->addAction("∑  &SUMA del rango",     this, &MainWindow::onSumRange);
    fmlMenu->addAction("x̄  &PROMEDIO del rango", this, &MainWindow::onAverageRange);
    fmlMenu->addAction("↑  &MÁXIMO del rango",   this, &MainWindow::onMaxRange);
    fmlMenu->addAction("↓  M&ÍNIMO del rango",   this, &MainWindow::onMinRange);
}

void MainWindow::setupToolBar() {
    QToolBar *tb = addToolBar("Principal");
    tb->setMovable(false);
    tb->setIconSize(QSize(14, 14));
    tb->setStyleSheet(
        QString("QToolBar { background:%1; border:none;"
                " border-bottom:1px solid %2; spacing:1px; padding:2px 4px; }"
                "QToolButton { padding:3px 10px; border-radius:2px; font-size:12px;"
                " color:%3; background:transparent; }"
                "QToolButton:hover { background:%4; }"
                "QToolButton:pressed { background:%5; }"
                "QToolBar::separator { width:1px; background:%6; margin:4px 3px; }")
            .arg(ExcelTheme::TOOLBAR_BG)      // %1 toolbar bg
            .arg(ExcelTheme::TOOLBAR_HOVER)   // %2 bottom border
            .arg(ExcelTheme::TOOLBAR_TEXT)    // %3 button text (blanco — visible sobre verde)
            .arg(ExcelTheme::TOOLBAR_HOVER)   // %4 hover
            .arg(ExcelTheme::TOOLBAR_PRESSED) // %5 pressed
            .arg("#2D9A5D")                   // %6 separator color
    );

    auto addBtn = [&](const QString &text, auto slot, const QString &tip = "") {
        QAction *a = tb->addAction(text, this, slot);
        if (!tip.isEmpty()) a->setToolTip(tip);
    };

    addBtn("📄 Nuevo",   &MainWindow::onNew,         "Nueva hoja (Ctrl+N)");
    addBtn("📂 Abrir",   &MainWindow::onLoad,        "Abrir archivo CSV (Ctrl+O)");
    addBtn("💾 Guardar", &MainWindow::onSave,        "Guardar como CSV (Ctrl+S)");
    tb->addSeparator();

    addBtn("＋↑ Fila",  &MainWindow::onAddRowBefore, "Insertar fila antes de la selección");
    addBtn("＋↓ Fila",  &MainWindow::onAddRowAfter,  "Insertar fila después de la selección");
    addBtn("✕ Fila",    &MainWindow::onRemoveRow,    "Eliminar fila seleccionada");
    tb->addSeparator();

    addBtn("＋← Col",  &MainWindow::onAddColBefore, "Insertar columna a la izquierda");
    addBtn("＋→ Col",  &MainWindow::onAddColAfter,  "Insertar columna a la derecha");
    addBtn("✕ Col",    &MainWindow::onRemoveCol,    "Eliminar columna seleccionada");
    tb->addSeparator();

    addBtn("✕ Rango",   &MainWindow::onRemoveRange,  "Eliminar contenido del rango seleccionado");
    tb->addSeparator();

    addBtn("∑ Suma",    &MainWindow::onSumRange,     "SUMA sobre el rango seleccionado");
    addBtn("x̄ Prom.",   &MainWindow::onAverageRange, "PROMEDIO sobre el rango seleccionado");
    addBtn("↑ Máx",     &MainWindow::onMaxRange,     "MÁXIMO en el rango seleccionado");
    addBtn("↓ Mín",     &MainWindow::onMinRange,     "MÍNIMO en el rango seleccionado");
}

// ─────────────────────────────────────────────────────────────
//  Helpers
// ─────────────────────────────────────────────────────────────

QString MainWindow::getCellRef(int row, int col) {
    QString colStr;
    int tmp = col;
    do {
        colStr.prepend(QChar('A' + (tmp % 26)));
        tmp = tmp / 26 - 1;
    } while (tmp >= 0);
    return colStr + QString::number(row + 1);
}

QString MainWindow::formatDisplayValue(const std::string &raw, int r, int c) {
    if (raw.empty()) return "";
    if (raw[0] == '=') {
        double val = matrix->getNumericValue(r, c);
        if (val == std::floor(val) && std::abs(val) < 1e14)
            return QString::number(static_cast<long long>(val));
        std::ostringstream oss;
        oss << val;
        return QString::fromStdString(oss.str());
    }
    return QString::fromStdString(raw);
}

void MainWindow::syncTableHeaders() {
    QStringList colHdrs;
    for (int c = 0; c < numCols; ++c) {
        QString ref = getCellRef(0, c);
        colHdrs << ref.left(ref.size() - 1);
    }
    tableWidget->setHorizontalHeaderLabels(colHdrs);

    QStringList rowHdrs;
    for (int r = 0; r < numRows; ++r)
        rowHdrs << QString::number(r + 1);
    tableWidget->setVerticalHeaderLabels(rowHdrs);
}

// Aplica color de fórmula o restablece el color normal a una celda
void MainWindow::applyCellStyle(QTableWidgetItem *item, const std::string &raw) {
    if (!item) return;
    bool isFormula = (!raw.empty() && raw[0] == '=');
    bool isNumeric = (!raw.empty() && (std::isdigit(raw[0]) || raw[0] == '-'));

    if (isFormula) {
        item->setBackground(QColor(ExcelTheme::FORMULA_BG));
        item->setForeground(QColor(ExcelTheme::FORMULA_TEXT));
        item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    } else if (isNumeric) {
        item->setBackground(QColor(ExcelTheme::CELL_BG));
        item->setForeground(QColor(ExcelTheme::CELL_TEXT));
        item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    } else {
        item->setBackground(QColor(ExcelTheme::CELL_BG));
        item->setForeground(QColor(ExcelTheme::CELL_TEXT));
        item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    }
}

void MainWindow::refreshTable() {
    blockTableSignals = true;
    tableWidget->clearContents();
    tableWidget->setRowCount(numRows);
    tableWidget->setColumnCount(numCols);
    syncTableHeaders();

    for (int r = 0; r < numRows; ++r) {
        for (int c = 0; c < numCols; ++c) {
            std::string raw = matrix->get(r, c);
            if (!raw.empty()) {
                QTableWidgetItem *item = new QTableWidgetItem(formatDisplayValue(raw, r, c));
                applyCellStyle(item, raw);
                tableWidget->setItem(r, c, item);
            }
        }
    }
    blockTableSignals = false;
}

void MainWindow::refreshCell(int r, int c) {
    blockTableSignals = true;
    std::string raw = matrix->get(r, c);
    if (raw.empty()) {
        delete tableWidget->takeItem(r, c);
    } else {
        QTableWidgetItem *item = tableWidget->item(r, c);
        if (!item) {
            item = new QTableWidgetItem();
            tableWidget->setItem(r, c, item);
        }
        item->setText(formatDisplayValue(raw, r, c));
        applyCellStyle(item, raw);
    }
    blockTableSignals = false;
}

bool MainWindow::getSelectionRange(int &r1, int &c1, int &r2, int &c2) {
    auto ranges = tableWidget->selectedRanges();
    if (ranges.isEmpty()) {
        r1 = r2 = currentRow;
        c1 = c2 = currentCol;
        return true;
    }
    r1 = ranges.first().topRow();
    c1 = ranges.first().leftColumn();
    r2 = ranges.first().bottomRow();
    c2 = ranges.first().rightColumn();
    return true;
}

void MainWindow::showStatus(const QString &msg, int ms) {
    statusBar()->showMessage(msg, ms);
}

// ─────────────────────────────────────────────────────────────
//  Slots — Celda
// ─────────────────────────────────────────────────────────────

void MainWindow::onCellClicked(int row, int col) {
    currentRow = row;
    currentCol = col;
    cellRefLabel->setText(getCellRef(row, col));
    std::string raw = matrix->get(row, col);
    formulaBar->setText(QString::fromStdString(raw));
}

void MainWindow::onCellChanged(int row, int col) {
    if (blockTableSignals) return;
    QTableWidgetItem *item = tableWidget->item(row, col);
    std::string val = item ? item->text().toStdString() : "";
    matrix->set(row, col, val);
    if (row == currentRow && col == currentCol)
        formulaBar->setText(QString::fromStdString(matrix->get(row, col)));
    refreshCell(row, col);
    refreshFormulaCells(); // ← actualiza solo las celdas con fórmulas
}

void MainWindow::onFormulaConfirmed() {
    QString val = formulaBar->text();
    matrix->set(currentRow, currentCol, val.toStdString());
    refreshCell(currentRow, currentCol);
    refreshFormulaCells(); // ← actualiza solo las celdas con fórmulas
    tableWidget->setCurrentCell(currentRow, currentCol);
    showStatus(QString("Celda %1 actualizada.").arg(getCellRef(currentRow, currentCol)));
}
// ─────────────────────────────────────────────────────────────
//  Slots — Filas y Columnas
// ─────────────────────────────────────────────────────────────

void MainWindow::onAddRowBefore() {
    int r1, c1, r2, c2;
    getSelectionRange(r1, c1, r2, c2);
    matrix->addRow(r1);
    numRows++;
    blockTableSignals = true;
    tableWidget->insertRow(r1);
    syncTableHeaders();
    blockTableSignals = false;
    showStatus(QString("Fila insertada antes de la fila %1.").arg(r1 + 1));
}

void MainWindow::onAddRowAfter() {
    int r1, c1, r2, c2;
    getSelectionRange(r1, c1, r2, c2);
    matrix->addRow(r2 + 1);
    numRows++;
    blockTableSignals = true;
    tableWidget->insertRow(r2 + 1);
    syncTableHeaders();
    blockTableSignals = false;
    showStatus(QString("Fila insertada después de la fila %1.").arg(r2 + 1));
}

void MainWindow::onAddColBefore() {
    int r1, c1, r2, c2;
    getSelectionRange(r1, c1, r2, c2);
    matrix->addColumn(c1);
    numCols++;
    blockTableSignals = true;
    tableWidget->insertColumn(c1);
    syncTableHeaders();
    blockTableSignals = false;
    showStatus(QString("Columna insertada antes de %1.").arg(getCellRef(0, c1).left(1)));
}

void MainWindow::onAddColAfter() {
    int r1, c1, r2, c2;
    getSelectionRange(r1, c1, r2, c2);
    matrix->addColumn(c2 + 1);
    numCols++;
    blockTableSignals = true;
    tableWidget->insertColumn(c2 + 1);
    syncTableHeaders();
    blockTableSignals = false;
    showStatus(QString("Columna insertada después de %1.").arg(getCellRef(0, c2).left(1)));
}

void MainWindow::onRemoveRow() {
    int r1, c1, r2, c2;
    getSelectionRange(r1, c1, r2, c2);
    for (int r = r2; r >= r1; --r) {
        matrix->removeRow(r);
        blockTableSignals = true;
        tableWidget->removeRow(r);
        blockTableSignals = false;
        numRows--;
    }
    syncTableHeaders();
    showStatus(QString("Fila(s) %1–%2 eliminada(s).").arg(r1 + 1).arg(r2 + 1));
}

void MainWindow::onRemoveCol() {
    int r1, c1, r2, c2;
    getSelectionRange(r1, c1, r2, c2);
    for (int c = c2; c >= c1; --c) {
        matrix->removeColumn(c);
        blockTableSignals = true;
        tableWidget->removeColumn(c);
        blockTableSignals = false;
        numCols--;
    }
    syncTableHeaders();
    showStatus("Columna(s) eliminada(s).");
}

void MainWindow::onRemoveRange() {
    int r1, c1, r2, c2;
    getSelectionRange(r1, c1, r2, c2);
    matrix->removeRange(r1, c1, r2, c2);
    blockTableSignals = true;
    for (int r = r1; r <= r2; ++r)
        for (int c = c1; c <= c2; ++c)
            delete tableWidget->takeItem(r, c);
    blockTableSignals = false;
    showStatus(QString("Rango %1:%2 borrado.")
        .arg(getCellRef(r1, c1)).arg(getCellRef(r2, c2)));
}

// ─────────────────────────────────────────────────────────────
//  Slots — Operaciones de rango
// ─────────────────────────────────────────────────────────────

void MainWindow::onSumRange() {
    int r1, c1, r2, c2;
    getSelectionRange(r1, c1, r2, c2);
    double res = matrix->sumRange(r1, c1, r2, c2);
    QMessageBox::information(this, "SUMA",
        QString("SUMA(%1:%2)  =  %3")
            .arg(getCellRef(r1,c1)).arg(getCellRef(r2,c2)).arg(res));
}

void MainWindow::onAverageRange() {
    int r1, c1, r2, c2;
    getSelectionRange(r1, c1, r2, c2);
    double res = matrix->averageRange(r1, c1, r2, c2);
    QMessageBox::information(this, "PROMEDIO",
        QString("PROMEDIO(%1:%2)  =  %3")
            .arg(getCellRef(r1,c1)).arg(getCellRef(r2,c2)).arg(res));
}

void MainWindow::onMaxRange() {
    int r1, c1, r2, c2;
    getSelectionRange(r1, c1, r2, c2);
    double res = matrix->maxInRange(r1, c1, r2, c2);
    QMessageBox::information(this, "MÁXIMO",
        QString("MÁX(%1:%2)  =  %3")
            .arg(getCellRef(r1,c1)).arg(getCellRef(r2,c2)).arg(res));
}

void MainWindow::onMinRange() {
    int r1, c1, r2, c2;
    getSelectionRange(r1, c1, r2, c2);
    double res = matrix->minInRange(r1, c1, r2, c2);
    QMessageBox::information(this, "MÍNIMO",
        QString("MÍN(%1:%2)  =  %3")
            .arg(getCellRef(r1,c1)).arg(getCellRef(r2,c2)).arg(res));
}

// ─────────────────────────────────────────────────────────────
//  Slots — Archivo
// ─────────────────────────────────────────────────────────────

void MainWindow::onNew() {
    if (QMessageBox::question(this, "Nueva hoja",
            "¿Deseas descartar la hoja actual y comenzar una nueva?",
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
    {
        delete matrix;
        numRows = INIT_ROWS;
        numCols = INIT_COLS;
        matrix  = new SparseMatrix(numRows, numCols);
        refreshTable();
        showStatus("Nueva hoja creada.");
    }
}

void MainWindow::onSave() {
    QString path = QFileDialog::getSaveFileName(
        this, "Guardar hoja", "hoja.csv",
        "Archivos CSV (*.csv);;Todos los archivos (*.*)"
    );
    if (path.isEmpty()) return;
    matrix->saveToFile(path.toStdString());
    showStatus(QString("Guardado: %1").arg(path));
}

void MainWindow::onLoad() {
    QString path = QFileDialog::getOpenFileName(
        this, "Abrir hoja", "",
        "Archivos CSV (*.csv);;Todos los archivos (*.*)"
    );
    if (path.isEmpty()) return;
    matrix->loadFromFile(path.toStdString());
    numRows = matrix->getRows();
    numCols = matrix->getCols();
    refreshTable();
    showStatus(QString("Cargado: %1").arg(path));
}

void MainWindow::refreshFormulaCells() {
    blockTableSignals = true;
    for (int r = 0; r < matrix->getRows(); ++r) {
        Node* curr = matrix->getRowHeader(r); // solo nodos con contenido
        while (curr) {
            if (!curr->rawValue.empty() && curr->rawValue[0] == '=') {
                QTableWidgetItem *item = tableWidget->item(curr->row, curr->col);
                if (!item) {
                    item = new QTableWidgetItem();
                    tableWidget->setItem(curr->row, curr->col, item);
                }
                item->setText(formatDisplayValue(curr->rawValue, curr->row, curr->col));
            }
            curr = curr->right;
        }
    }
    blockTableSignals = false;
}