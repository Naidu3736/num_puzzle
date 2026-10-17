#pragma once

#include "board/board.hpp"

#include <QWidget>
#include <QFutureWatcher>
#include <QChartView>
#include <QtCharts>
#include <optional>

QT_BEGIN_NAMESPACE
namespace Ui {
    class AnalysisTab;
}
QT_END_NAMESPACE

struct DifficultyMetrics {
    QList<QPointF> execution_times;
    QList<QPointF> nodes_expanded_points;
    int successes = 0;
    int failures = 0;
    double total_cost = 0;
    double total_mixing = 0;
};

struct AnalysisResults {
    DifficultyMetrics easy;
    DifficultyMetrics medium;
    DifficultyMetrics hard;
    int board_dimension = 3;
};

class AnalysisTab : public QWidget {
    Q_OBJECT

    private:
        Ui::AnalysisTab *m_ui;
        naidu::Board m_goal_board;
        QFutureWatcher<void> m_analysis_watcher;
        bool m_is_analyzing = false;

        QChart *m_time_chart = nullptr;
        QChart *m_nodes_chart = nullptr;
        QChart *m_success_chart = nullptr;
        QChart *m_optimal_chart = nullptr;

        QChartView *m_time_chart_view = nullptr;
        QChartView *m_nodes_chart_view = nullptr;
        QChartView *m_success_chart_view = nullptr;
        QChartView *m_optimal_chart_view = nullptr;

        AnalysisResults*   m_pending_results = nullptr;
         QMutex* m_pending_mutex   = nullptr;

        QChartView* create_chart_view(QChart* chart, QWidget* container);
        void setup_charts();
        void update_time_chart(const AnalysisResults& results);
        void update_nodes_chart(const AnalysisResults& results);
        void update_success_chart(const AnalysisResults& results);
        void update_optimal_chart(const AnalysisResults& results);
        void update_all_charts(const AnalysisResults& results);
        
        void on_run_analysis_clicked();
        void on_cancel_analysis_clicked();
        void on_analysis_finished();
        void on_dimension_changed(int value);

        std::optional<naidu::Board> parse_goal_input() const;

    public:
        explicit AnalysisTab(QWidget *parent = nullptr);
        ~AnalysisTab() override;
        
        /**
         * @brief Guarda las gráficas estadísticas del análisis en el directorio especificado.
         */
        void save_all_charts(const QString& directory_path);

        void set_board(const naidu::Board& goal);
};