#pragma once

#include <QWidget>
#include <QFutureWatcher>
#include <QChartView>
#include <QChart>
#include <QLineSeries>
#include <QBarSeries>
#include <QBarSet>
#include <QPercentBarSeries>
#include <QBarCategoryAxis>
#include <QValueAxis>
#include "solver/solver.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
    class AnalysisTab;
}
QT_END_NAMESPACE

struct DifficultyMetrics {
    QVector<QPointF> execution_times;
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
        QFutureWatcher<void> m_analysis_watcher;
        bool m_is_analyzing = false;

        QChart *m_time_chart = nullptr;
        QChart *m_success_chart = nullptr;
        QChart *m_optimal_chart = nullptr;

        QChartView *m_time_chart_view = nullptr;
        QChartView *m_success_chart_view = nullptr;
        QChartView *m_optimal_chart_view = nullptr;

        void setup_charts();
        void update_time_chart(const AnalysisResults& results);
        void update_success_chart(const AnalysisResults& results);
        void update_optimal_chart(const AnalysisResults& results);
        void update_all_charts(const AnalysisResults& results);
        
        /**
         * @brief Ejecuta el análisis empírico masivo coordinando lotes de tableros generados por naidu::randomizer y resueltos por naidu::solver::ida.
         */
        AnalysisResults execute_empirical_analysis(int instances, int easy_m, int med_m, int hard_m, int max_attempts, int dim, const naidu::solver::Config& config, bool rand_avoid);
        
        /**
         * @brief Ejecuta un lote de pruebas de dificultad para medir rendimiento y métricas utilizando el núcleo de resolución.
         */
        DifficultyMetrics run_difficulty_batch(int moves_count, int instances, std::uint8_t dim, int max_attempts, bool rand_avoid, const naidu::solver::Config& config);

        void on_run_analysis_clicked();
        void on_cancel_analysis_clicked();

    public:
        explicit AnalysisTab(QWidget *parent = nullptr);
        ~AnalysisTab() override;
        
        /**
         * @brief Guarda las gráficas estadísticas del análisis en el directorio especificado.
         */
        void save_all_charts(const QString& directory_path);
};