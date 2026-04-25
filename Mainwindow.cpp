#include "mainwindow.h"

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
    // Paleta de colores general
    QPalette pal = QApplication::palette();
    pal.setColor(QPalette::Window,      QColor("#F5F5F5"));
    pal.setColor(QPalette::Base,        QColor("#FFFFFF"));
    pal.setColor(QPalette::Highlight,   QColor("#1A73E8"));
    QApplication::setPalette(pal);

    // Widget central
    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // ── Barra de fórmulas ──────────────────────────────────
    QWidget *fxBar = new QWidget();
    fxBar->setFixedHeight(34);
    fxBar->setStyleSheet("background:#FFFFFF; border-bottom:1px solid #E0E0E0;");
    QHBoxLayout *fxLayout = new QHBoxLayout(fxBar);
    fxLayout->setContentsMargins(6, 2, 6, 2);
    fxLayout->setSpacing(4);

    cellRefLabel = new QLabel("A1");
    cellRefLabel->setFixedWidth(52);
    cellRefLabel->setAlignment(Qt::AlignCenter);
    cellRefLabel->setStyleSheet(
        "font-weight:bold; font-size:12px; color:#333;"
        "border:1px solid #D0D0D0; border-radius:3px;"
        "background:#F8F8F8; padding:2px 4px;"
    );

    QLabel *fxIcon = new QLabel("ƒx");
    fxIcon->setFixedWidth(22);
    fxIcon->setAlignment(Qt::AlignCenter);
    fxIcon->setStyleSheet("color:#1A73E8; font-weight:bold; font-size:13px;");

    formulaBar = new QLineEdit();
    formulaBar->setPlaceholderText("Ingresa un valor, texto o fórmula (=A1+B1) …");
    formulaBar->setStyleSheet(
        "border:1px solid #D0D0D0; border-radius:3px;"
        "padding:2px 6px; font-size:13px;"
        "background:#FAFAFA;"
        "selection-background-color:#BDD7FF;"
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

    // Tamaño de celdas
    tableWidget->horizontalHeader()->setDefaultSectionSize(90);
    tableWidget->verticalHeader()->setDefaultSectionSize(22);
    tableWidget->horizontalHeader()->setMinimumSectionSize(30);
    tableWidget->verticalHeader()->setMinimumSectionSize(18);

    tableWidget->setStyleSheet(R"(
        QTableWidget {
            gridline-color: #E0E0E0;
            font-size: 13px;
            selection-color: #000;
        }
        QTableWidget::item {
            padding: 0 4px;
        }
        QTableWidget::item:selected {
            background-color: #BDD7FF;
            color: #000;
        }
        QHeaderView::section {
            background-color: #F1F3F4;
            border: 0px;
            border-right: 1px solid #D8D8D8;
            border-bottom: 1px solid #D8D8D8;
            font-size: 12px;
            color: #555;
            padding: 2px;
        }
        QHeaderView::section:hover {
            background-color: #E8EAED;
        }
    )");

    syncTableHeaders();

    connect(tableWidget, &QTableWidget::cellClicked,
            this,        &MainWindow::onCellClicked);
    connect(tableWidget, &QTableWidget::cellChanged,
            this,        &MainWindow::onCellChanged);

    mainLayout->addWidget(fxBar);
    mainLayout->addWidget(tableWidget);

    setStatusBar(new QStatusBar(this));
    setCentralWidget(central);
}

void MainWindow::setupMenuBar() {
    QMenuBar *mb = menuBar();
    mb->setStyleSheet(
        "QMenuBar { background:#F8F8F8; border-bottom:1px solid #E0E0E0; }"
        "QMenuBar::item:selected { background:#E8EAED; border-radius:3px; }"
        "QMenu { background:#FFFFFF; border:1px solid #D0D0D0; }"
        "QMenu::item:selected { background:#1A73E8; color:#FFF; }"
    );

    // Archivo
    QMenu *fileMenu = mb->addMenu("&Archivo");
    fileMenu->addAction(QIcon(), "&Nuevo",  this, &MainWindow::onNew,  QKeySequence::New);
    fileMenu->addAction(QIcon(), "&Abrir…", this, &MainWindow::onLoad, QKeySequence::Open);
    fileMenu->addAction(QIcon(), "&Guardar…",this,&MainWindow::onSave, QKeySequence::Save);
    fileMenu->addSeparator();
    fileMenu->addAction("&Salir", qApp, &QApplication::quit, QKeySequence::Quit);

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
    editMenu->addAction("Eliminar &rango seleccionado", this, &MainWindow::onRemoveRange,
                        QKeySequence::Delete);

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
    tb->setIconSize(QSize(16, 16));
    tb->setStyleSheet(
        "QToolBar { background:#F8F8F8; border-bottom:1px solid #E0E0E0; spacing:2px; }"
        "QToolButton { padding:3px 8px; border-radius:3px; font-size:12px; }"
        "QToolButton:hover { background:#E8EAED; }"
        "QToolButton:pressed { background:#D0D0D0; }"
    );

    auto addBtn = [&](const QString &text, auto slot, const QString &tip = "") {
        QAction *a = tb->addAction(text, this, slot);
        if (!tip.isEmpty()) a->setToolTip(tip);
    };

    addBtn("📄 Nuevo",   &MainWindow::onNew,         "Nueva hoja (Ctrl+N)");
    addBtn("📂 Abrir",   &MainWindow::onLoad,        "Abrir archivo CSV (Ctrl+O)");
    addBtn("💾 Guardar", &MainWindow::onSave,        "Guardar como CSV (Ctrl+S)");
    tb->addSeparator();

    addBtn("➕↑ Fila",  &MainWindow::onAddRowBefore, "Insertar fila antes de la selección");
    addBtn("➕↓ Fila",  &MainWindow::onAddRowAfter,  "Insertar fila después de la selección");
    addBtn("🗑 Fila",   &MainWindow::onRemoveRow,    "Eliminar fila seleccionada");
    tb->addSeparator();

    addBtn("➕← Col",  &MainWindow::onAddColBefore, "Insertar columna a la izquierda");
    addBtn("➕→ Col",  &MainWindow::onAddColAfter,  "Insertar columna a la derecha");
    addBtn("🗑 Col",   &MainWindow::onRemoveCol,    "Eliminar columna seleccionada");
    tb->addSeparator();

    addBtn("🗑 Rango",  &MainWindow::onRemoveRange,  "Eliminar contenido del rango seleccionado");
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
    // Soporta columnas más allá de Z (AA, AB, …)
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
        // Muestra entero si no tiene decimales
        if (val == std::floor(val) && std::abs(val) < 1e14)
            return QString::number(static_cast<long long>(val));
        // Hasta 6 cifras significativas
        std::ostringstream oss;
        oss << val;
        return QString::fromStdString(oss.str());
    }
    return QString::fromStdString(raw);
}

void MainWindow::syncTableHeaders() {
    // Cabeceras de columna (A, B, C …)
    QStringList colHdrs;
    for (int c = 0; c < numCols; ++c) colHdrs << getCellRef(0, c).left(getCellRef(0, c).size() - 1);
    tableWidget->setHorizontalHeaderLabels(colHdrs);

    // Cabeceras de fila (1, 2, 3 …)
    QStringList rowHdrs;
    for (int r = 0; r < numRows; ++r) rowHdrs << QString::number(r + 1);
    tableWidget->setVerticalHeaderLabels(rowHdrs);
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
                // Alineación: texto a la izquierda, números/fórmulas a la derecha
                if (!raw.empty() && (raw[0] == '=' || std::isdigit(raw[0]) || raw[0] == '-'))
                    item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                else
                    item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
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
        if (raw[0] == '=' || std::isdigit(raw[0]) || raw[0] == '-')
            item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        else
            item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
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

    // Actualiza la barra de fórmulas si es la celda actual
    if (row == currentRow && col == currentCol)
        formulaBar->setText(QString::fromStdString(matrix->get(row, col)));

    // Muestra el valor calculado (no el raw) en la celda
    refreshCell(row, col);
}

void MainWindow::onFormulaConfirmed() {
    QString val = formulaBar->text();
    matrix->set(currentRow, currentCol, val.toStdString());
    refreshCell(currentRow, currentCol);
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

    // Eliminar de abajo hacia arriba para preservar índices
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
    showStatus(QString("Columna(s) eliminada(s)."));
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
    refreshTable();
    showStatus(QString("Cargado: %1").arg(path));
}
