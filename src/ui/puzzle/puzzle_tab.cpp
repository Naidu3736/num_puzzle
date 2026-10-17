#include "puzzle_tab.h"
#include "ui_puzzle_tab.h"
#include "board/board.hpp"
#include "board_view.hpp"
#include "solver/solver.hpp"
#include "randomizer/randomizer.hpp"

#include <QMessageBox>
#include <QStringList>
#include <QtConcurrent>
#include <random>

static QString board_to_string(const naidu::Board& board) {
    QStringList parts;
    for (auto tile : board.tiles()) {
        parts.append(QString::number(tile));
    }
    return parts.join(",");
}

static std::optional<naidu::Board> parse_board(const QString& text) {
    QStringList parts = text.split(',', Qt::SkipEmptyParts);
    int total = parts.size();
    int dim_int = static_cast<int>(std::round(std::sqrt(total)));
    
    if (dim_int * dim_int != total || dim_int < 3 || dim_int > 15) return std::nullopt;
    
    std::vector<std::uint8_t> tiles;
    tiles.reserve(parts.size());
    
    for (const QString& part : parts) {
        bool ok = false;
        int val = part.trimmed().toInt(&ok);
        if (!ok || val < 0 || val >= dim_int * dim_int) return std::nullopt;
        tiles.push_back(static_cast<std::uint8_t>(val));
    }
    
    try { return naidu::Board(tiles); }
    catch (...) { return std::nullopt; }
}

PuzzleTab::PuzzleTab(QWidget *parent)
    : QWidget(parent)
    , m_ui(new Ui::PuzzleTab)
{
    m_ui->setupUi(this);
    m_ui->eval_combo->setCurrentIndex(-1);

    connect(m_ui->dimension_slider, &QSlider::valueChanged, this, &PuzzleTab::on_dimension_changed);
    connect(m_ui->speed_slider, &QSlider::valueChanged, this, &PuzzleTab::on_speed_slider_changed);
    connect(m_ui->solve_btn, &QPushButton::clicked, this, &PuzzleTab::on_solve_clicked);
    connect(m_ui->stop_btn, &QPushButton::clicked, this, &PuzzleTab::on_stop_clicked);
    connect(m_ui->shuffle_btn, &QPushButton::clicked, this, &PuzzleTab::on_shuffle_clicked);
    connect(m_ui->replay_btn, &QPushButton::clicked, this, &PuzzleTab::on_replay_animation);

    connect(&m_solver_watcher, &QFutureWatcher<naidu::solver::Solution>::finished, this, &PuzzleTab::on_solver_finished);
    connect(&m_solver_watcher, &QFutureWatcher<naidu::solver::Solution>::canceled, this, &PuzzleTab::on_solver_cancelled);
    connect(&m_anim_timer, &QTimer::timeout, this, &PuzzleTab::on_anim_step);

    m_ui->dimension_value->setText(QString::number(m_ui->dimension_slider->value()));
    
    std::uint8_t default_dim = static_cast<std::uint8_t>(m_ui->dimension_slider->value());
    m_initial_board = m_goal_board = naidu::Board(default_dim);
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

    m_ui->dimension_slider->blockSignals(true);
    m_ui->dimension_slider->setValue(initial.dim());
    m_ui->dimension_slider->blockSignals(false);

    m_ui->dimension_value->setText(QString::number(initial.dim()));
    m_ui->board_view->set_board(initial);
    
    m_ui->initial_input->setText(board_to_string(initial));
    m_ui->goal_input->setText(board_to_string(goal));
}

void PuzzleTab::on_dimension_changed(int value) {
    m_ui->dimension_value->setText(QString::number(value));
    naidu::Board default_board(static_cast<std::uint8_t>(value));
    
    m_ui->initial_input->clear();
    m_ui->goal_input->clear();
    
    m_initial_board = m_goal_board = default_board;
    m_ui->board_view->set_board(default_board);
}

void PuzzleTab::on_speed_slider_changed(int value) {
    m_ui->speed_label->setText(QString("Velocidad: %1 ms").arg(value));
    if (m_anim_timer.isActive()) {
        m_anim_timer.setInterval(value);
    }
}

naidu::evaluation::EvaluationFn PuzzleTab::get_selected_evaluation() const {
    static const naidu::evaluation::EvaluationFn evals[] = {
        naidu::evaluation::evaluation_1,
        naidu::evaluation::evaluation_2,
        naidu::evaluation::evaluation_3
    };
    
    int idx = m_ui->eval_combo->currentIndex();
    if (idx < 0 || idx >= 3) return naidu::evaluation::evaluation_2;
    return evals[idx];
}

void PuzzleTab::on_solve_clicked() {
    if (m_solver_watcher.isRunning()) {
        QMessageBox::warning(this, "Aviso", "Ya hay una búsqueda en curso.");
        return;
    }

    if (m_anim_timer.isActive()) m_anim_timer.stop();

    QString initial_text = m_ui->initial_input->text().trimmed();
    QString goal_text = m_ui->goal_input->text().trimmed();

    if (initial_text.isEmpty()) {
        QMessageBox::warning(this, "Error", "El campo de estado inicial no puede estar vacío.");
        return;
    }

    auto initial_board = parse_board(initial_text);
    if (!initial_board) {
        QMessageBox::warning(this, "Error", "El estado inicial ingresado no es válido.");
        return;
    }

    naidu::Board goal_board;
    if (goal_text.isEmpty()) {
        goal_board = naidu::Board(initial_board->dim());
    } else {
        auto parsed_goal = parse_board(goal_text);
        if (!parsed_goal) {
            QMessageBox::warning(this, "Error", "El estado meta ingresado no es válido.");
            return;
        }
        goal_board = *parsed_goal;
    }

    if (initial_board->dim() != goal_board.dim()) {
        QMessageBox::warning(this, "Error", "El tablero inicial y el tablero meta deben tener la misma dimensión.");
        return;
    }

    m_initial_board = *initial_board;
    m_goal_board = goal_board;

    naidu::solver::Config config;
    config.eval_fn = get_selected_evaluation();
    config.max_nodes = static_cast<std::uint64_t>(m_ui->max_nodes_spin->value());
    config.avoid_revisits = m_ui->avoid_revisits_solver_check->isChecked();
    config.weighted_cost = m_ui->weighted_cost_check->isChecked();
    config.cancel_requested = [this]() { return m_solver_watcher.isCanceled(); };

    m_ui->status_label->setText("Resolviendo en segundo plano...");
    m_ui->solve_btn->setEnabled(false);
    m_ui->stop_btn->setEnabled(true);

    m_start_time = std::chrono::high_resolution_clock::now();

    QFuture<naidu::solver::Solution> future = QtConcurrent::run(
        [initial = *initial_board, goal = goal_board, config]() {
            return naidu::solver::ida(initial, goal, config);
        }
    );

    m_solver_watcher.setFuture(future);
}

void PuzzleTab::on_solver_finished() {
    m_ui->solve_btn->setEnabled(true);
    m_ui->stop_btn->setEnabled(false);

    if (m_solver_watcher.isCanceled()) return;

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
                case naidu::Direction::UP:    path_list.append("U"); break;
                case naidu::Direction::DOWN:  path_list.append("D"); break;
                case naidu::Direction::LEFT:  path_list.append("L"); break;
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
        m_anim_moves = m_solver_watcher.result().moves;
    }

    if (m_anim_moves.empty()) return;

    if (m_anim_timer.isActive()) m_anim_timer.stop();

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
    if (m_anim_timer.isActive()) m_anim_timer.stop();

    naidu::randomizer::Config rand_config;
    rand_config.moves = static_cast<std::uint32_t>(m_ui->moves_spin->value());
    rand_config.max_attempts = static_cast<std::uint32_t>(m_ui->attempts_spin->value());
    rand_config.avoid_revisits = m_ui->avoid_revisits_rand_check->isChecked();

    std::random_device rd;
    std::mt19937 rng(rd());

    m_initial_board = naidu::randomizer::generate(m_goal_board, rand_config, rng);
    m_ui->board_view->set_board(m_initial_board);
    m_ui->initial_input->setText(board_to_string(m_initial_board));
    m_ui->status_label->setText("Tablero mezclado aleatoriamente.");
}