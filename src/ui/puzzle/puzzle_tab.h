#pragma once 

#include "board/board.hpp"
#include "evaluation/evaluation.hpp"
#include "solver/solver.hpp"

#include <QWidget>
#include <QString>
#include <optional>
#include <chrono>
#include <vector>
#include <QFutureWatcher>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui {
    class PuzzleTab;
}
QT_END_NAMESPACE

class PuzzleTab : public QWidget {
    Q_OBJECT

    private:
        Ui::PuzzleTab *m_ui;

        QFutureWatcher<naidu::solver::Solution> m_solver_watcher;
        QTimer m_anim_timer;
        
        naidu::Board m_initial_board;
        naidu::Board m_goal_board;
        naidu::Board m_current_anim_board;
        std::vector<naidu::Direction> m_anim_moves;
        std::size_t m_anim_index = 0;

        std::chrono::high_resolution_clock::time_point m_start_time;

        void on_dimension_changed(int value);
        void on_solve_clicked();
        void on_stop_clicked();
        void on_shuffle_clicked();
        void on_solver_finished();
        void on_solver_cancelled();
        void on_anim_step();
        void on_speed_slider_changed(int value);
        void on_replay_animation();
        std::optional<naidu::Board> parse_board_input(const QString& text, std::uint8_t dim) const;

    public:
        explicit PuzzleTab(QWidget *parent = nullptr);
        ~PuzzleTab() override;

        void set_boards(const naidu::Board& initial, const naidu::Board& goal);
        naidu::evaluation::EvaluationFn get_selected_evaluation() const;
};