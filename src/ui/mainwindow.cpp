#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui/puzzle/puzzle_tab.h"
#include "ui/analysis/analysis_tab.h"

#include <QFileDialog>
#include <QMessageBox>
#include <fstream>
#include <sstream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_ui(new Ui::MainWindow)
{
    m_ui->setupUi(this);
    this->showMaximized();

    connect(m_ui->action_load_board, &QAction::triggered, this, &MainWindow::action_load_board_triggered);
    connect(m_ui->action_save_analysis, &QAction::triggered, this, &MainWindow::action_save_analysis_triggered);
    connect(m_ui->action_quit, &QAction::triggered, this, &MainWindow::action_quit_triggered);
}

MainWindow::~MainWindow() {
    delete m_ui;
}

PuzzleTab* MainWindow::get_active_puzzle_tab() const {
    if (auto *tab = qobject_cast<PuzzleTab*>(m_ui->tabWidget->currentWidget())) {
        return tab;
    }
    
    for (int i = 0; i < m_ui->tabWidget->count(); ++i) {
        if (auto *tab = qobject_cast<PuzzleTab*>(m_ui->tabWidget->widget(i))) {
            m_ui->tabWidget->setCurrentIndex(i);
            return tab;
        }
    }
    return nullptr;
}

AnalysisTab* MainWindow::get_active_analysis_tab() const {
    if (auto *tab = qobject_cast<AnalysisTab*>(m_ui->tabWidget->currentWidget())) {
        return tab;
    }
    
    for (int i = 0; i < m_ui->tabWidget->count(); ++i) {
        if (auto *tab = qobject_cast<AnalysisTab*>(m_ui->tabWidget->widget(i))) {
            m_ui->tabWidget->setCurrentIndex(i);
            return tab;
        }
    }
    return nullptr;
}

std::vector<std::uint8_t> MainWindow::parse_board_grid(std::ifstream& file, std::uint8_t dim) const {
    std::vector<std::uint8_t> tiles;
    std::string line;
    
    for (int i = 0; i < dim; ++i) {
        if (!std::getline(file, line)) {
            throw std::runtime_error("Faltan líneas en el archivo.");
        }
        std::stringstream ss(line);
        std::string val_str;
        while (std::getline(ss, val_str, ',')) {
            std::stringstream val_ss(val_str);
            int tile = 0;
            if (val_ss >> tile) {
                tiles.push_back(static_cast<std::uint8_t>(tile));
            }
        }
    }
    return tiles;
}

void MainWindow::action_load_board_triggered() {
    QString file_path = QFileDialog::getOpenFileName(
        this, "Cargar Tableros desde Archivo", "", "Archivos de texto (*.txt);;Todos los archivos (*.*)"
    );

    if (file_path.isEmpty()) return;

    std::ifstream file(file_path.toStdString());
    if (!file.is_open()) {
        QMessageBox::critical(this, "Error", "No se pudo abrir el archivo seleccionado.");
        return;
    }

    try {
        std::string line;
        if (!std::getline(file, line)) {
            throw std::runtime_error("El archivo está vacío.");
        }

        std::istringstream dim_stream(line);
        int dim_int = 0;
        if (!(dim_stream >> dim_int) || dim_int < 3 || dim_int > 15) {
            throw std::runtime_error("Dimensión inválida (debe estar entre 3 y 15).");
        }
        std::uint8_t dim = static_cast<std::uint8_t>(dim_int);

        auto initial_tiles = parse_board_grid(file, dim);
        auto goal_tiles = parse_board_grid(file, dim);

        naidu::Board initial_board(initial_tiles);
        naidu::Board goal_board(goal_tiles);

        PuzzleTab *puzzle_tab = get_active_puzzle_tab();
        if (!puzzle_tab) {
            QMessageBox::warning(this, "Aviso", "No se encontró una pestaña de puzzle activa.");
            return;
        }

        puzzle_tab->set_boards(initial_board, goal_board);
        QMessageBox::information(this, "Éxito", "Tableros cargados correctamente.");

    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error de Formato", e.what());
    } catch (...) {
        QMessageBox::critical(this, "Error", "Ocurrió un error inesperado al procesar el archivo.");
    }
}

void MainWindow::action_save_analysis_triggered() {
    AnalysisTab *analysis_tab = get_active_analysis_tab();
    if (!analysis_tab) {
        QMessageBox::warning(this, "Aviso", "No se encontró una pestaña de análisis activa para guardar.");
        return;
    }

    QString dir_path = QFileDialog::getExistingDirectory(
        this, "Seleccionar Carpeta para Guardar Gráficas de Análisis", ""
    );

    if (dir_path.isEmpty()) return;

    analysis_tab->save_all_charts(dir_path);
    QMessageBox::information(this, "Guardado", "Las gráficas de análisis se guardaron exitosamente en la carpeta seleccionada.");
}

void MainWindow::action_quit_triggered() {
    close();
}