#include "puzzle_tab.h"
#include "board/board.hpp"
#include "ui_puzzle_tab.h"
#include "board_view.hpp"
#include "solver/solver.hpp"
#include "randomizer/randomizer.hpp"

#include <QMessageBox>
#include <QStringList>
#include <QtConcurrent/QtConcurrent>
#include <random>
#include <QScrollArea>
#include <QLabel>

PuzzleTab::PuzzleTab(QWidget *parent)
    : QWidget(parent)
    , m_ui(new Ui::PuzzleTab)
{
    m_ui->setupUi(this);
    m_ui->eval_combo->setCurrentIndex(-1);

    connect(m_ui->dimension_slider, &QSlider::valueChanged, this, &PuzzleTab::on_dimension_changed);
    m_ui->dimension_value->setText(QString::number(m_ui->dimension_slider->value()));

    connect(m_ui->speed_slider, &QSlider::valueChanged, this, &PuzzleTab::on_speed_slider_changed);
    connect(m_ui->solve_btn, &QPushButton::clicked, this, &PuzzleTab::on_solve_clicked);
    connect(m_ui->stop_btn, &QPushButton::clicked, this, &PuzzleTab::on_stop_clicked);
    connect(m_ui->shuffle_btn, &QPushButton::clicked, this, &PuzzleTab::on_shuffle_clicked);
    connect(m_ui->replay_btn, &QPushButton::clicked, this, &PuzzleTab::on_replay_animation);

    connect(&m_solver_watcher, &QFutureWatcher<naidu::solver::Solution>::finished, this, &PuzzleTab::on_solver_finished);
    connect(&m_solver_watcher, &QFutureWatcher<naidu::solver::Solution>::canceled, this, &PuzzleTab::on_solver_cancelled);
    connect(&m_anim_timer, &QTimer::timeout, this, &PuzzleTab::on_anim_step);

    std::uint8_t default_dim = static_cast<std::uint8_t>(m_ui->dimension_slider->value());
    naidu::Board default_board(default_dim);
    m_initial_board = default_board;
    m_goal_board = default_board;
    m_ui->board_view->set_board(m_initial_board);
}

PuzzleTab::~PuzzleTab() {
    if (m_solver_watcher.isRunning()) {
        m_solver_watcher.cancel();
        m_solver_watcher.waitForFinished();
    }
    delete m_ui;
}

void PuzzleTab::set_boards(const naidu::Board& initial, const naidu::Board& goal) {
    m_initial_board = initial;
    m_goal_board = goal;

    std::uint8_t dim = initial.dim();
    
    m_ui->dimension_slider->blockSignals(true);
    m_ui->dimension_slider->setValue(dim);
    m_ui->dimension_slider->blockSignals(false);
    m_ui->dimension_value->setText(QString::number(dim));

    m_ui->board_view->set_board(initial);

    QString initial_str, goal_str;
    auto initial_tiles = initial.tiles();
    for (std::size_t i = 0; i < initial_tiles.size(); ++i) {
        initial_str += QString::number(initial_tiles[i]);
        if (i + 1 < initial_tiles.size()) initial_str += ",";
    }
    m_ui->initial_input->setText(initial_str);

    auto goal_tiles = goal.tiles();
    for (std::size_t i = 0; i < goal_tiles.size(); ++i) {
        goal_str += QString::number(goal_tiles[i]);
        if (i + 1 < goal_tiles.size()) goal_str += ",";
    }
    m_ui->goal_input->setText(goal_str);
}

void PuzzleTab::on_dimension_changed(int value) {
    m_ui->dimension_value->setText(QString::number(value));
    naidu::Board default_board(static_cast<std::uint8_t>(value));
    m_initial_board = default_board;
    m_goal_board = default_board;
    m_ui->board_view->set_board(default_board);
}

void PuzzleTab::on_speed_slider_changed(int value) {
    m_ui->speed_label->setText(QString("Velocidad: %1 ms").arg(value));
    if (m_anim_timer.isActive()) {
        m_anim_timer.setInterval(value);
    }
}

naidu::evaluation::EvaluationFn PuzzleTab::get_selected_evaluation() const {
    int index = m_ui->eval_combo->currentIndex();
    if (index == -1) return naidu::evaluation::evaluation_1;

    switch (index) {
        case 0: return naidu::evaluation::evaluation_1;
        case 1: return naidu::evaluation::evaluation_2;
        case 2: return naidu::evaluation::evaluation_3;
        default: return naidu::evaluation::evaluation_1;
    }
}

std::optional<naidu::Board> PuzzleTab::parse_board_input(const QString& text, std::uint8_t dim) const {
    QStringList parts = text.split(',', Qt::SkipEmptyParts);
    std::vector<std::uint8_t> tiles;
    tiles.reserve(parts.size());
    for (const QString& part : parts) {
        bool ok = false;
        int val = part.trimmed().toInt(&ok);
        if (!ok || val < 0 || val >= dim * dim) return std::nullopt;
        tiles.push_back(static_cast<std::uint8_t>(val));
    }
    try {
        return naidu::Board(tiles);
    } catch (...) {
        return std::nullopt;
    }
}

void PuzzleTab::on_solve_clicked() {
    if (m_solver_watcher.isRunning()) {
        QMessageBox::warning(this, "Aviso", "Ya hay una búsqueda en curso.");
        return;
    }

    if (m_anim_timer.isActive()) {
        m_anim_timer.stop();
    }

    QString initial_text = m_ui->initial_input->text().trimmed();
    QString goal_text = m_ui->goal_input->text().trimmed();

    if (initial_text.isEmpty()) {
        QMessageBox::warning(this, "Error", "El campo de estado inicial no puede estar vacío.");
        return;
    }

    QStringList initial_parts = initial_text.split(',', Qt::SkipEmptyParts);
    int initial_total = initial_parts.size();
    int initial_dim_int = static_cast<int>(std::round(std::sqrt(initial_total)));
    if (initial_dim_int * initial_dim_int != initial_total || initial_dim_int < 3 || initial_dim_int > 15) {
        QMessageBox::warning(this, "Error", "El tamaño del estado inicial no forma un tablero cuadrado válido.");
        return;
    }
    std::uint8_t initial_dim = static_cast<std::uint8_t>(initial_dim_int);

    auto initial_parsed = parse_board_input(initial_text, initial_dim);
    if (!initial_parsed) {
        QMessageBox::warning(this, "Error", "El estado inicial ingresado no es válido.");
        return;
    }

    naidu::Board initial_board = *initial_parsed;
    naidu::Board goal_board;

    // Si el campo meta está vacío, se asume automáticamente la meta estándar por defecto
    if (goal_text.isEmpty()) {
        goal_board = naidu::Board(initial_dim);
    } else {
        QStringList goal_parts = goal_text.split(',', Qt::SkipEmptyParts);
        int goal_total = goal_parts.size();
        int goal_dim_int = static_cast<int>(std::round(std::sqrt(goal_total)));
        if (goal_dim_int * goal_dim_int != goal_total || goal_dim_int < 3 || goal_dim_int > 15) {
            QMessageBox::warning(this, "Error", "El tamaño del estado meta no forma un tablero cuadrado válido.");
            return;
        }
        std::uint8_t goal_dim = static_cast<std::uint8_t>(goal_dim_int);

        auto goal_parsed = parse_board_input(goal_text, goal_dim);
        if (!goal_parsed) {
            QMessageBox::warning(this, "Error", "El estado meta ingresado no es válido.");
            return;
        }
        goal_board = *goal_parsed;
    }

    if (initial_board.dim() != goal_board.dim()) {
        QMessageBox::warning(this, "Error", "El tablero inicial y el tablero meta deben tener exactamente la misma dimensión.");
        return;
    }

    m_initial_board = initial_board;
    m_goal_board = goal_board;

    naidu::solver::Config config;
    config.eval_fn = get_selected_evaluation();
    config.max_nodes = static_cast<std::uint64_t>(m_ui->max_nodes_spin->value());
    config.avoid_revisits = m_ui->avoid_revisits_solver_check->isChecked();
    config.weighted_cost = m_ui->weighted_cost_check->isChecked();
    config.cancel_requested = [this]() {
        return m_solver_watcher.isCanceled();
    };

    m_ui->status_label->setText("Resolviendo en segundo plano...");
    m_ui->solve_btn->setEnabled(false);
    m_ui->stop_btn->setEnabled(true);

    m_start_time = std::chrono::high_resolution_clock::now();

    QFuture<naidu::solver::Solution> future = QtConcurrent::run(
        [initial_board, goal_board, config]() {
            return naidu::solver::ida(initial_board, goal_board, config);
        }
    );

    m_solver_watcher.setFuture(future);
}

void PuzzleTab::on_solver_finished() {
    m_ui->solve_btn->setEnabled(true);
    m_ui->stop_btn->setEnabled(false);

    if (m_solver_watcher.isCanceled()) {
        return;
    }

    naidu::solver::Solution solution = m_solver_watcher.result();

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - m_start_time).count();
    m_ui->runtime_value_label->setText(QString::number(duration_ms) + " ms");

    if (solution.found) {
        m_ui->status_label->setText("¡Solución encontrada! Animando...");
        m_ui->nodes_value_label->setText(QString::number(solution.nodes_expanded));
        m_ui->cost_value_label->setText(QString::number(solution.cost));

        QStringList path_list;
        for (auto d : solution.moves) {
            switch(d) {
                case naidu::Direction::UP: path_list.append("U"); break;
                case naidu::Direction::DOWN: path_list.append("D"); break;
                case naidu::Direction::LEFT: path_list.append("L"); break;
                case naidu::Direction::RIGHT: path_list.append("R"); break;
            }
        }
        
        m_ui->path_content_label->setText(path_list.join(","));
        m_ui->path_content_label->adjustSize();
        m_ui->path_scroll_area->updateGeometry();

        m_anim_moves = solution.moves;
        on_replay_animation();
    } else {
        m_ui->status_label->setText("No se encontró solución (límite de nodos alcanzado).");
        m_ui->nodes_value_label->setText(QString::number(solution.nodes_expanded));
        m_ui->path_content_label->clear();
    }
}

void PuzzleTab::on_stop_clicked() {
    if (m_solver_watcher.isRunning()) {
        m_solver_watcher.cancel();
        m_ui->solve_btn->setEnabled(true);
        m_ui->stop_btn->setEnabled(false);
        m_ui->status_label->setText("Búsqueda cancelada por el usuario.");
    }
}

void PuzzleTab::on_solver_cancelled() {
    m_ui->solve_btn->setEnabled(true);
    m_ui->stop_btn->setEnabled(false);
    m_ui->status_label->setText("Búsqueda cancelada.");
}

void PuzzleTab::on_replay_animation() {
    if (m_anim_moves.empty() && m_solver_watcher.isFinished()) {
        auto solution = m_solver_watcher.result();
        m_anim_moves = solution.moves;
    }

    if (m_anim_moves.empty()) return;

    if (m_anim_timer.isActive()) {
        m_anim_timer.stop();
    }

    m_current_anim_board = m_initial_board;
    m_anim_index = 0;
    m_ui->board_view->set_board(m_current_anim_board);

    m_anim_timer.start(m_ui->speed_slider->value());
    m_ui->status_label->setText("Animando solución...");
}

void PuzzleTab::on_anim_step() {
    if (m_anim_index < m_anim_moves.size()) {
        m_current_anim_board.move(m_anim_moves[m_anim_index]);
        m_ui->board_view->set_board(m_current_anim_board);
        ++m_anim_index;
    } else {
        m_anim_timer.stop();
        m_ui->status_label->setText("¡Animación completada!");
    }
}

void PuzzleTab::on_shuffle_clicked() {
    if (m_anim_timer.isActive()) {
        m_anim_timer.stop();
    }

    std::uint8_t dim = static_cast<std::uint8_t>(m_ui->dimension_slider->value());

    naidu::randomizer::Config rand_config;
    rand_config.moves = static_cast<std::uint32_t>(m_ui->moves_spin->value());
    rand_config.max_attempts = static_cast<std::uint32_t>(m_ui->attempts_spin->value());
    rand_config.avoid_revisits = m_ui->avoid_revisits_rand_check->isChecked();

    std::random_device rd;
    std::mt19937 rng(rd());

    naidu::Board shuffled = naidu::randomizer::generate(dim, rand_config, rng);
    m_initial_board = shuffled;
    m_ui->board_view->set_board(shuffled);

    QString tiles_str;
    auto tiles = shuffled.tiles();
    for (std::size_t i = 0; i < tiles.size(); ++i) {
        tiles_str += QString::number(tiles[i]);
        if (i + 1 < tiles.size()) tiles_str += ",";
    }
    m_ui->initial_input->setText(tiles_str);
    m_ui->status_label->setText("Tablero mezclado aleatoriamente.");
}