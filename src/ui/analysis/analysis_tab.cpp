#include "analysis_tab.h"
#include "ui_analysis_tab.h"
#include "board/board.hpp"
#include "randomizer/randomizer.hpp"
#include "solver/solver.hpp"
#include "evaluation/evaluation.hpp"

#include <QMessageBox>
#include <QtConcurrent>
#include <QVBoxLayout>
#include <QMutex>
#include <QMutexLocker>
#include <random>
#include <chrono>
#include <cstdint>
#include <QDir>
#include <QPixmap>

AnalysisTab::AnalysisTab(QWidget *parent) 
    : QWidget(parent)
    , m_ui(new Ui::AnalysisTab)
{
    m_ui->setupUi(this);
    m_ui->eval_combo->setCurrentIndex(-1);

    setup_charts();

    connect(
        m_ui->dimension_slider, &QSlider::valueChanged,
        this, [this](int val) {
            m_ui->dimension_value_label->setText(QString::number(val));
        }
    );

    connect(
        m_ui->run_analysis_btn, &QPushButton::clicked,
        this, &AnalysisTab::on_run_analysis_clicked
    );

    connect(
        m_ui->stop_analysis_btn, &QPushButton::clicked,
        this, &AnalysisTab::on_cancel_analysis_clicked
    );
}

AnalysisTab::~AnalysisTab() {
    if (m_analysis_watcher.isRunning()) {
        m_analysis_watcher.cancel();
        m_analysis_watcher.waitForFinished();
    }
    delete m_ui;
}

void AnalysisTab::setup_charts() {
    m_time_chart = new QChart();
    m_time_chart->setTitle("Tiempo de Ejecución por Instancia");
    m_time_chart->setAnimationOptions(QChart::SeriesAnimations);
    m_time_chart_view = new QChartView(m_time_chart, m_ui->time_chart_view);
    m_time_chart_view->setRenderHint(QPainter::Antialiasing);
    
    QVBoxLayout *time_layout = new QVBoxLayout(m_ui->time_chart_view);
    time_layout->addWidget(m_time_chart_view);

    m_success_chart = new QChart();
    m_success_chart->setTitle("Porcentaje de Soluciones Encontradas vs No Encontradas");
    m_success_chart->setAnimationOptions(QChart::SeriesAnimations);
    m_success_chart_view = new QChartView(m_success_chart, m_ui->success_chart_view);
    m_success_chart_view->setRenderHint(QPainter::Antialiasing);
    
    QVBoxLayout *success_layout = new QVBoxLayout(m_ui->success_chart_view);
    success_layout->addWidget(m_success_chart_view);

    m_optimal_chart = new QChart();
    m_optimal_chart->setTitle("Comparativa: Costo Real Promedio vs Movimientos de Mezcla");
    m_optimal_chart->setAnimationOptions(QChart::SeriesAnimations);
    m_optimal_chart_view = new QChartView(m_optimal_chart, m_ui->optimal_chart_view);
    m_optimal_chart_view->setRenderHint(QPainter::Antialiasing);
    
    QVBoxLayout *optimal_layout = new QVBoxLayout(m_ui->optimal_chart_view);
    optimal_layout->addWidget(m_optimal_chart_view);
}

void AnalysisTab::update_time_chart(const AnalysisResults& results) {
    m_time_chart->removeAllSeries();
    for (QAbstractAxis *axis : m_time_chart->axes()) { m_time_chart->removeAxis(axis); delete axis; }

    QLineSeries *line_easy = new QLineSeries();
    line_easy->setName("Fácil");
    for (const auto& pt : results.easy.execution_times) line_easy->append(pt);

    QLineSeries *line_med = new QLineSeries();
    line_med->setName("Medio");
    for (const auto& pt : results.medium.execution_times) line_med->append(pt);

    QLineSeries *line_hard = new QLineSeries();
    line_hard->setName("Difícil");
    for (const auto& pt : results.hard.execution_times) line_hard->append(pt);

    m_time_chart->addSeries(line_easy);
    m_time_chart->addSeries(line_med);
    m_time_chart->addSeries(line_hard);
    m_time_chart->createDefaultAxes();

    if (auto axisX = m_time_chart->axes(Qt::Horizontal).value(0, nullptr)) axisX->setTitleText("Índice de Instancia");
    if (auto axisY = m_time_chart->axes(Qt::Vertical).value(0, nullptr)) axisY->setTitleText(QString("Tiempo (ms) [Tablero %1x%1]").arg(results.board_dimension));
}

void AnalysisTab::update_success_chart(const AnalysisResults& results) {
    m_success_chart->removeAllSeries();
    for (QAbstractAxis *axis : m_success_chart->axes()) { m_success_chart->removeAxis(axis); delete axis; }

    QBarSet *set_found = new QBarSet("Encontradas");
    QBarSet *set_not_found = new QBarSet("No Encontradas");

    *set_found << results.easy.successes << results.medium.successes << results.hard.successes;
    *set_not_found << results.easy.failures << results.medium.failures << results.hard.failures;

    QPercentBarSeries *bar_series = new QPercentBarSeries();
    bar_series->append(set_found);
    bar_series->append(set_not_found);
    m_success_chart->addSeries(bar_series);

    QStringList categories_bar = {"Fácil", "Medio", "Difícil"};
    QBarCategoryAxis *axisX_bar = new QBarCategoryAxis();
    axisX_bar->append(categories_bar);
    m_success_chart->addAxis(axisX_bar, Qt::AlignBottom);
    bar_series->attachAxis(axisX_bar);

    QValueAxis *axisY_bar = new QValueAxis();
    axisY_bar->setRange(0, 100);
    axisY_bar->setTitleText("Porcentaje (%)");
    m_success_chart->addAxis(axisY_bar, Qt::AlignLeft);
    bar_series->attachAxis(axisY_bar);
}

void AnalysisTab::update_optimal_chart(const AnalysisResults& results) {
    m_optimal_chart->removeAllSeries();
    for (QAbstractAxis *axis : m_optimal_chart->axes()) { m_optimal_chart->removeAxis(axis); delete axis; }

    QBarSet *bar_cost = new QBarSet("Promedio de Costo Real");
    QBarSet *bar_mix = new QBarSet("Promedio de Mezcla");

    double avg_c_e = results.easy.successes > 0 ? results.easy.total_cost / results.easy.successes : 0;
    double avg_m_e = results.easy.successes > 0 ? results.easy.total_mixing / results.easy.successes : 0;
    double avg_c_m = results.medium.successes > 0 ? results.medium.total_cost / results.medium.successes : 0;
    double avg_m_m = results.medium.successes > 0 ? results.medium.total_mixing / results.medium.successes : 0;
    double avg_c_h = results.hard.successes > 0 ? results.hard.total_cost / results.hard.successes : 0;
    double avg_m_h = results.hard.successes > 0 ? results.hard.total_mixing / results.hard.successes : 0;

    *bar_cost << avg_c_e << avg_c_m << avg_c_h;
    *bar_mix << avg_m_e << avg_m_m << avg_m_h;

    QBarSeries *compare_series = new QBarSeries();
    compare_series->append(bar_cost);
    compare_series->append(bar_mix);
    m_optimal_chart->addSeries(compare_series);

    QStringList categories_bar = {"Fácil", "Medio", "Difícil"};
    QBarCategoryAxis *axisX_opt = new QBarCategoryAxis();
    axisX_opt->append(categories_bar);
    m_optimal_chart->addAxis(axisX_opt, Qt::AlignBottom);
    compare_series->attachAxis(axisX_opt);

    QValueAxis *axisY_opt = new QValueAxis();
    axisY_opt->setTitleText("Número de Movimientos");
    m_optimal_chart->addAxis(axisY_opt, Qt::AlignLeft);
    compare_series->attachAxis(axisY_opt);
}

void AnalysisTab::update_all_charts(const AnalysisResults& results) {
    update_time_chart(results);
    update_success_chart(results);
    update_optimal_chart(results);
}

void AnalysisTab::save_all_charts(const QString& directory_path) {
    if (m_time_chart_view) {
        QPixmap pixmap = m_time_chart_view->grab();
        pixmap.save(QDir(directory_path).filePath("tiempo_ejecucion.png"));
    }
    if (m_success_chart_view) {
        QPixmap pixmap = m_success_chart_view->grab();
        pixmap.save(QDir(directory_path).filePath("porcentaje_exito.png"));
    }
    if (m_optimal_chart_view) {
        QPixmap pixmap = m_optimal_chart_view->grab();
        pixmap.save(QDir(directory_path).filePath("comparativa_optima.png"));
    }
}

DifficultyMetrics AnalysisTab::run_difficulty_batch(int moves_count, int instances, std::uint8_t dim, int max_attempts, bool rand_avoid, const naidu::solver::Config& config) {
    DifficultyMetrics metrics;
    naidu::randomizer::Config rand_config;
    rand_config.moves = static_cast<std::uint32_t>(moves_count);
    rand_config.max_attempts = static_cast<std::uint32_t>(max_attempts == 0 ? 50000 : max_attempts);
    rand_config.avoid_revisits = rand_avoid;

    QList<int> tasks;
    for (int i = 0; i < instances; ++i) tasks.append(i);

    QMutex mutex;
    int instance_counter = 0;

    QtConcurrent::blockingMap(tasks, [&](int) {
        std::random_device rd;
        std::mt19937 rng(rd());

        naidu::Board initial = naidu::randomizer::generate(dim, rand_config, rng);
        naidu::Board goal(dim);

        if (initial.is_solvable()) {
            auto start = std::chrono::high_resolution_clock::now();
            auto solution = naidu::solver::ida(initial, goal, config);
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

            QMutexLocker locker(&mutex);
            int current_id = instance_counter++;
            if (solution.found) {
                metrics.successes++;
                metrics.execution_times.append(QPointF(current_id, static_cast<qreal>(duration)));
                metrics.total_cost += solution.cost;
                metrics.total_mixing += moves_count;
            } else {
                metrics.failures++;
            }
        } else {
            QMutexLocker locker(&mutex);
            metrics.failures++;
        }
    });

    return metrics;
}

AnalysisResults AnalysisTab::execute_empirical_analysis(int instances, int easy_m, int med_m, int hard_m, int max_attempts, int dim, const naidu::solver::Config& config, bool rand_avoid) {
    AnalysisResults results;
    results.board_dimension = dim;

    results.easy = run_difficulty_batch(easy_m, instances, static_cast<std::uint8_t>(dim), max_attempts, rand_avoid, config);
    results.medium = run_difficulty_batch(med_m, instances, static_cast<std::uint8_t>(dim), max_attempts, rand_avoid, config);
    results.hard = run_difficulty_batch(hard_m, instances, static_cast<std::uint8_t>(dim), max_attempts, rand_avoid, config);

    return results;
}

void AnalysisTab::on_run_analysis_clicked() {
    if (m_is_analyzing) {
        QMessageBox::warning(this, "Aviso", "Ya hay un análisis en ejecución.");
        return;
    }

    int instances = m_ui->instances_spin->value();
    int easy_moves = m_ui->easy_spin->value();
    int med_moves = m_ui->medium_spin->value();
    int hard_moves = m_ui->hard_spin->value();
    int max_attempts = m_ui->attempts_spin->value();
    int target_dim = m_ui->dimension_slider->value();
    bool rand_avoid = m_ui->avoid_revisits_rand_radio->isChecked();

    naidu::solver::Config config;
    int eval_index = m_ui->eval_combo->currentIndex();
    switch (eval_index) {
        case 0: config.eval_fn = naidu::evaluation::evaluation_1; break;
        case 1: config.eval_fn = naidu::evaluation::evaluation_2; break;
        case 2: config.eval_fn = naidu::evaluation::evaluation_3; break;
        default: config.eval_fn = naidu::evaluation::evaluation_1; break;
    }

    config.max_nodes = static_cast<std::uint64_t>(m_ui->max_nodes_spin->value());
    config.weighted_cost = m_ui->weighted_cost_radio->isChecked();
    config.avoid_revisits = m_ui->avoid_revisits_solver_radio->isChecked();

    // Marcar como en ejecución y actualizar botones
    m_is_analyzing = true;
    m_ui->run_analysis_btn->setEnabled(false);
    m_ui->stop_analysis_btn->setEnabled(true);

    QFuture<void> future = QtConcurrent::run([=, this]() {
        AnalysisResults results;
        results.board_dimension = target_dim;

        auto batch_with_check = [&](int moves_count, DifficultyMetrics& metrics) {
            naidu::randomizer::Config rand_config;
            rand_config.moves = static_cast<std::uint32_t>(moves_count);
            rand_config.max_attempts = static_cast<std::uint32_t>(max_attempts == 0 ? 50000 : max_attempts);
            rand_config.avoid_revisits = rand_avoid;

            QList<int> tasks;
            for (int i = 0; i < instances; ++i) tasks.append(i);

            QMutex mutex;
            int instance_counter = 0;

            QtConcurrent::blockingMap(tasks, [&](int) {
                if (QThread::currentThread()->isInterruptionRequested() || m_analysis_watcher.isCanceled()) {
                    return;
                }

                std::random_device rd;
                std::mt19937 rng(rd());

                naidu::Board initial = naidu::randomizer::generate(static_cast<std::uint8_t>(target_dim), rand_config, rng);
                naidu::Board goal(static_cast<std::uint8_t>(target_dim));

                if (initial.is_solvable()) {
                    auto start = std::chrono::high_resolution_clock::now();
                    auto solution = naidu::solver::ida(initial, goal, config);
                    auto end = std::chrono::high_resolution_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

                    QMutexLocker locker(&mutex);
                    int current_id = instance_counter++;
                    if (solution.found) {
                        metrics.successes++;
                        metrics.execution_times.append(QPointF(current_id, static_cast<qreal>(duration)));
                        metrics.total_cost += solution.cost;
                        metrics.total_mixing += moves_count;
                    } else {
                        metrics.failures++;
                    }
                } else {
                    QMutexLocker locker(&mutex);
                    metrics.failures++;
                }
            });
        };

        batch_with_check(easy_moves, results.easy);
        if (!m_analysis_watcher.isCanceled()) batch_with_check(med_moves, results.medium);
        if (!m_analysis_watcher.isCanceled()) batch_with_check(hard_moves, results.hard);

        QMetaObject::invokeMethod(this, [=, this]() {
            // Liberar estado al terminar
            m_is_analyzing = false;
            m_ui->run_analysis_btn->setEnabled(true);
            m_ui->stop_analysis_btn->setEnabled(false);

            if (m_analysis_watcher.isCanceled()) {
                QMessageBox::information(this, "Cancelado", "El análisis empírico fue cancelado por el usuario.");
            } else {
                update_all_charts(results);
                QMessageBox::information(this, "Completado", QString("Análisis individual para el tablero de %1x%1 finalizado.").arg(target_dim));
            }
        }, Qt::QueuedConnection);
    });

    m_analysis_watcher.setFuture(future);
}

void AnalysisTab::on_cancel_analysis_clicked() {
    if (m_is_analyzing) {
        m_analysis_watcher.cancel();
        m_ui->stop_analysis_btn->setEnabled(false);
    }
}