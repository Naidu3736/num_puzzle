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
#include <QThread>
#include <QThreadPool>
#include <random>
#include <chrono>
#include <cstdint>
#include <QDir>
#include <QPixmap>
#include <algorithm>
#include <vector>

// ---------- Helpers estáticos ----------

static void clear_chart(QChart* chart) {
    chart->removeAllSeries();
    for (QAbstractAxis* axis : chart->axes()) {
        chart->removeAxis(axis);
        delete axis;
    }
}

AnalysisTab::AnalysisTab(QWidget *parent)
    : QWidget(parent)
    , m_ui(new Ui::AnalysisTab)
{
    m_ui->setupUi(this);
    m_ui->eval_combo->setCurrentIndex(-1);

    setup_charts();

    connect(m_ui->dimension_slider, &QSlider::valueChanged,
            this, &AnalysisTab::on_dimension_changed);
    connect(m_ui->run_analysis_btn, &QPushButton::clicked,
            this, &AnalysisTab::on_run_analysis_clicked);
    connect(m_ui->stop_analysis_btn, &QPushButton::clicked,
            this, &AnalysisTab::on_cancel_analysis_clicked);
    connect(&m_analysis_watcher, &QFutureWatcher<void>::finished,
            this, &AnalysisTab::on_analysis_finished);

    m_ui->dimension_value_label->setText(QString::number(m_ui->dimension_slider->value()));
    m_goal_board = naidu::Board(static_cast<std::uint8_t>(m_ui->dimension_slider->value()));
}

AnalysisTab::~AnalysisTab() {
    if (m_analysis_watcher.isRunning()) {
        m_analysis_watcher.cancel();
        m_analysis_watcher.waitForFinished();
    }
    delete m_pending_results;
    delete m_pending_mutex;
    delete m_ui;
}

// ---------- Setup ----------

QChartView* AnalysisTab::create_chart_view(QChart* chart, QWidget* container) {
    chart->setAnimationOptions(QChart::SeriesAnimations);
    QChartView* view = new QChartView(chart, container);
    view->setRenderHint(QPainter::Antialiasing);

    if (container->layout()) {
        delete container->layout();
    }
    QVBoxLayout* layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(view);
    return view;
}

void AnalysisTab::setup_charts() {
    m_time_chart    = new QChart();
    m_nodes_chart   = new QChart();
    m_success_chart = new QChart();
    m_optimal_chart = new QChart();

    m_time_chart_view    = create_chart_view(m_time_chart,    m_ui->time_chart_view);
    m_nodes_chart_view   = create_chart_view(m_nodes_chart,   m_ui->nodes_chart_view);
    m_success_chart_view = create_chart_view(m_success_chart, m_ui->success_chart_view);
    m_optimal_chart_view = create_chart_view(m_optimal_chart, m_ui->optimal_chart_view);
}

QString board_to_string(const naidu::Board& board) {
    QStringList parts;
    for (auto tile : board.tiles()) {
        parts.append(QString::number(tile));
    }
    return parts.join(",");
}

// ---------- Chart updates ----------

void AnalysisTab::update_time_chart(const AnalysisResults& results) {
    clear_chart(m_time_chart);

    auto make_series = [](const QString& name, const QList<QPointF>& pts) -> QLineSeries* {
        QList<QPointF> sorted = pts;
        std::sort(sorted.begin(), sorted.end(),
                  [](const QPointF& a, const QPointF& b) { return a.y() < b.y(); });

        QLineSeries* s = new QLineSeries();
        s->setName(name);
        for (int i = 0; i < sorted.size(); ++i) {
            qreal y_val = sorted[i].y();
            if (y_val < 0.01) y_val = 0.01; 
            s->append(i, y_val);
        }
        return s;
    };

    QLineSeries* line_easy = make_series("Fácil",   results.easy.execution_times);
    QLineSeries* line_med  = make_series("Medio",   results.medium.execution_times);
    QLineSeries* line_hard = make_series("Difícil", results.hard.execution_times);

    m_time_chart->addSeries(line_easy);
    m_time_chart->addSeries(line_med);
    m_time_chart->addSeries(line_hard);

    int n_instances = std::max({
        results.easy.execution_times.size(),
        results.medium.execution_times.size(),
        results.hard.execution_times.size()
    });
    if (n_instances <= 0) n_instances = 1;

    QValueAxis* axisX = new QValueAxis();
    axisX->setTitleText("Instancias (Ordenadas por Tiempo)");
    axisX->setRange(0, n_instances - 1);
    axisX->setTickCount(std::min(n_instances, 11));
    m_time_chart->addAxis(axisX, Qt::AlignBottom);
    line_easy->attachAxis(axisX);
    line_med->attachAxis(axisX);
    line_hard->attachAxis(axisX);

    qreal max_time = 10.0;
    for (const auto* s : {line_easy, line_med, line_hard}) {
        for (const auto& p : s->points()) {
            max_time = std::max(max_time, p.y());
        }
    }

    QLogValueAxis* axisY = new QLogValueAxis();
    axisY->setTitleText("Tiempo (ms) [Escala Log]");
    axisY->setBase(10.0);
    axisY->setLabelFormat("%.3f");
    axisY->setMinorTickCount(0);
    axisY->setRange(0.01, max_time * 2.0);

    m_time_chart->addAxis(axisY, Qt::AlignLeft);
    line_easy->attachAxis(axisY);
    line_med->attachAxis(axisY);
    line_hard->attachAxis(axisY);
}

void AnalysisTab::update_nodes_chart(const AnalysisResults& results) {
    clear_chart(m_nodes_chart);

    // 1. Extraemos y transformamos a Log10
    auto extract_log = [](const DifficultyMetrics& m) {
        std::vector<qreal> values;
        values.reserve(m.nodes_expanded_points.size());
        for (const auto& pt : m.nodes_expanded_points) {
            qreal val = pt.y();
            if (val < 1.0) val = 1.0;
            values.push_back(std::log10(val)); 
        }
        std::sort(values.begin(), values.end());
        return values;
    };

    const auto easy_vals = extract_log(results.easy);
    const auto med_vals  = extract_log(results.medium);
    const auto hard_vals = extract_log(results.hard);

    if (easy_vals.empty() && med_vals.empty() && hard_vals.empty()) {
        m_nodes_chart->setTitle("Nodos Expandidos: sin datos");
        return;
    }

    auto make_box = [](const std::vector<qreal>& v, const QString& label) -> QBoxSet* {
        auto quantile = [&](double q) {
            if (v.empty()) return 0.0;
            if (v.size() == 1) return v[0];
            double idx = q * (v.size() - 1);
            std::size_t lo = static_cast<std::size_t>(std::floor(idx));
            std::size_t hi = static_cast<std::size_t>(std::ceil(idx));
            double frac = idx - lo;
            return v[lo] * (1 - frac) + v[hi] * frac;
        };

        QBoxSet* box = new QBoxSet(label);
        if (v.empty()) {
            for (int k = 0; k < 5; ++k) box->setValue(k, 0);
        } else {
            box->setValue(QBoxSet::LowerExtreme,  v.front());
            box->setValue(QBoxSet::LowerQuartile, quantile(0.25));
            box->setValue(QBoxSet::Median,        quantile(0.50));
            box->setValue(QBoxSet::UpperQuartile, quantile(0.75));
            box->setValue(QBoxSet::UpperExtreme,  v.back());
        }
        return box;
    };

    QBoxPlotSeries* series = new QBoxPlotSeries();
    series->setName("Nodos (Log10)");
    series->append(make_box(easy_vals, "Fácil"));
    series->append(make_box(med_vals,  "Medio"));
    series->append(make_box(hard_vals, "Difícil"));
    m_nodes_chart->addSeries(series);

    QBarCategoryAxis* axisX = new QBarCategoryAxis();
    axisX->append({"Fácil", "Medio", "Difícil"});
    m_nodes_chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis* axisY = new QValueAxis();
    
    qreal min_val = 10.0;
    qreal max_val = -10.0;
    for (const auto* v : {&easy_vals, &med_vals, &hard_vals}) {
        if (!v->empty()) {
            min_val = std::min(min_val, v->front());
            max_val = std::max(max_val, v->back());
        }
    }
    
    axisY->setRange(min_val - 0.5, max_val + 0.5);
    axisY->setTitleText("Nodos expandidos (Log10)");
    axisY->setLabelFormat("%.1f");

    m_nodes_chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
}

void AnalysisTab::update_success_chart(const AnalysisResults& results) {
    clear_chart(m_success_chart);

    QBarSet* set_found     = new QBarSet("Encontradas");
    QBarSet* set_not_found = new QBarSet("No Encontradas");

    *set_found     << results.easy.successes << results.medium.successes << results.hard.successes;
    *set_not_found << results.easy.failures  << results.medium.failures  << results.hard.failures;

    QPercentBarSeries* bar_series = new QPercentBarSeries();
    bar_series->append(set_found);
    bar_series->append(set_not_found);
    m_success_chart->addSeries(bar_series);

    QBarCategoryAxis* axisX_bar = new QBarCategoryAxis();
    axisX_bar->append({"Fácil", "Medio", "Difícil"});
    m_success_chart->addAxis(axisX_bar, Qt::AlignBottom);
    bar_series->attachAxis(axisX_bar);

    QValueAxis* axisY_bar = new QValueAxis();
    axisY_bar->setRange(0, 100);
    axisY_bar->setTitleText("Porcentaje (%)");
    m_success_chart->addAxis(axisY_bar, Qt::AlignLeft);
    bar_series->attachAxis(axisY_bar);
}

void AnalysisTab::update_optimal_chart(const AnalysisResults& results) {
    clear_chart(m_optimal_chart);

    QBarSet* bar_cost = new QBarSet("Promedio de Costo Real");
    QBarSet* bar_mix  = new QBarSet("Movimientos de Mezcla");

    auto avg = [](double total, int count) {
        return count > 0 ? total / count : 0.0;
    };

    *bar_cost << avg(results.easy.total_cost,   results.easy.successes)
              << avg(results.medium.total_cost, results.medium.successes)
              << avg(results.hard.total_cost,   results.hard.successes);
    *bar_mix  << avg(results.easy.total_mixing,   results.easy.successes)
              << avg(results.medium.total_mixing, results.medium.successes)
              << avg(results.hard.total_mixing,   results.hard.successes);

    QBarSeries* compare_series = new QBarSeries();
    compare_series->append(bar_cost);
    compare_series->append(bar_mix);
    m_optimal_chart->addSeries(compare_series);

    QBarCategoryAxis* axisX_opt = new QBarCategoryAxis();
    axisX_opt->append({"Fácil", "Medio", "Difícil"});
    m_optimal_chart->addAxis(axisX_opt, Qt::AlignBottom);
    compare_series->attachAxis(axisX_opt);

    QValueAxis* axisY_opt = new QValueAxis();
    axisY_opt->setTitleText("Número de Movimientos");
    m_optimal_chart->addAxis(axisY_opt, Qt::AlignLeft);
    compare_series->attachAxis(axisY_opt);
}

void AnalysisTab::update_all_charts(const AnalysisResults& results) {
    update_time_chart(results);
    update_nodes_chart(results);
    update_success_chart(results);
    update_optimal_chart(results);
}

void AnalysisTab::save_all_charts(const QString& directory_path) {
    QDir dir(directory_path);
    if (m_time_chart_view)    m_time_chart_view->grab().save(dir.filePath("tiempo_ejecucion.png"));
    if (m_nodes_chart_view)   m_nodes_chart_view->grab().save(dir.filePath("nodos_expandidos.png"));
    if (m_success_chart_view) m_success_chart_view->grab().save(dir.filePath("porcentaje_exito.png"));
    if (m_optimal_chart_view) m_optimal_chart_view->grab().save(dir.filePath("comparativa_optima.png"));
}

// ---------- Board input ----------

void AnalysisTab::set_board(const naidu::Board& goal) {
    m_goal_board = goal;
    std::uint8_t dim = goal.dim();

    m_ui->dimension_slider->blockSignals(true);
    m_ui->dimension_slider->setValue(dim);
    m_ui->dimension_slider->blockSignals(false);

    m_ui->dimension_value_label->setText(QString::number(dim));
    m_ui->goal_input->setText(board_to_string(goal));
}

std::optional<naidu::Board> AnalysisTab::parse_goal_input() const {
    const QString text = m_ui->goal_input->text().trimmed();
    const int dim = m_ui->dimension_slider->value();

    if (text.isEmpty()) {
        return naidu::Board(static_cast<std::uint8_t>(dim));
    }

    const QStringList parts = text.split(',', Qt::SkipEmptyParts);
    const int parsed_dim = static_cast<int>(std::round(std::sqrt(parts.size())));

    if (parsed_dim * parsed_dim != parts.size() || parsed_dim < 3 || parsed_dim > 15) {
        return std::nullopt;
    }

    std::vector<std::uint8_t> tiles;
    tiles.reserve(parts.size());
    for (const QString& part : parts) {
        bool ok = false;
        const int val = part.trimmed().toInt(&ok);
        if (!ok || val < 0 || val >= parsed_dim * parsed_dim) return std::nullopt;
        tiles.push_back(static_cast<std::uint8_t>(val));
    }

    try { return naidu::Board(tiles); }
    catch (...) { return std::nullopt; }
}

// ---------- Analysis ----------

void AnalysisTab::on_run_analysis_clicked() {
    if (m_is_analyzing) {
        QMessageBox::warning(this, "Aviso", "Ya hay un análisis en ejecución.");
        return;
    }

    const int instances    = m_ui->instances_spin->value();
    const int easy_moves   = m_ui->easy_spin->value();
    const int med_moves    = m_ui->medium_spin->value();
    const int hard_moves   = m_ui->hard_spin->value();
    const int max_attempts = m_ui->attempts_spin->value();
    const int target_dim   = m_ui->dimension_slider->value();
    const bool rand_avoid  = m_ui->avoid_revisits_rand_radio->isChecked();

    auto goal_parsed = parse_goal_input();
    if (!goal_parsed) {
        QMessageBox::warning(this, "Error", "El estado meta ingresado no es válido.");
        return;
    }

    const naidu::Board goal_board = *goal_parsed;
    if (static_cast<int>(goal_board.dim()) != target_dim) {
        QMessageBox::warning(this, "Error",
            QString("La dimensión de la meta (%1) no coincide con el slider (%2).")
                .arg(goal_board.dim()).arg(target_dim));
        return;
    }

    m_goal_board = goal_board;

    naidu::solver::Config config;
    switch (m_ui->eval_combo->currentIndex()) {
        case 0: config.eval_fn = naidu::evaluation::evaluation_1; break;
        case 1: config.eval_fn = naidu::evaluation::evaluation_2; break;
        case 2: config.eval_fn = naidu::evaluation::evaluation_3; break;
        default: config.eval_fn = naidu::evaluation::evaluation_2; break;
    }
    config.max_nodes      = static_cast<std::uint64_t>(m_ui->max_nodes_spin->value());
    config.weighted_cost  = m_ui->weighted_cost_radio->isChecked();
    config.avoid_revisits = m_ui->avoid_revisits_solver_radio->isChecked();

    delete m_pending_results;
    delete m_pending_mutex;
    m_pending_results = new AnalysisResults();
    m_pending_results->board_dimension = target_dim;
    m_pending_mutex = new QMutex();

    AnalysisResults* results_ptr = m_pending_results;

    m_is_analyzing = true;
    m_ui->run_analysis_btn->setEnabled(false);
    m_ui->stop_analysis_btn->setEnabled(true);

    QFuture<void> future = QtConcurrent::run(
        [this, results_ptr, goal_board, config,
         instances, easy_moves, med_moves, hard_moves,
         max_attempts, rand_avoid]() {

        auto batch = [&](int moves_count, DifficultyMetrics& metrics) {
            naidu::randomizer::Config rand_config;
            rand_config.moves          = static_cast<std::uint32_t>(moves_count);
            rand_config.max_attempts   = static_cast<std::uint32_t>(
                max_attempts == 0 ? 50000 : max_attempts);
            rand_config.avoid_revisits = rand_avoid;

            QList<int> tasks;
            for (int i = 0; i < instances; ++i) tasks.append(i);

            QMutex local_mutex;
            int instance_counter = 0;

            QtConcurrent::blockingMap(tasks, [&](int) {
                if (m_analysis_watcher.isCanceled()) return;

                std::random_device rd;
                std::mt19937 rng(rd());

                naidu::Board initial = naidu::randomizer::generate(
                    goal_board, rand_config, rng);

                auto start = std::chrono::high_resolution_clock::now();
                auto solution = naidu::solver::ida(initial, goal_board, config);
                auto end = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
                    end - start).count() / 1000.0;

                QMutexLocker locker(&local_mutex);
                int current_id = instance_counter++;
                if (solution.found) {
                    metrics.successes++;
                    metrics.execution_times.append(
                        QPointF(current_id, static_cast<qreal>(duration)));
                    metrics.nodes_expanded_points.append(
                        QPointF(current_id, static_cast<qreal>(solution.nodes_expanded)));
                    metrics.total_cost   += solution.cost;
                    metrics.total_mixing += moves_count;
                } else {
                    metrics.failures++;
                }
            });
        };

        batch(easy_moves, results_ptr->easy);
        if (!m_analysis_watcher.isCanceled())
            batch(med_moves, results_ptr->medium);
        if (!m_analysis_watcher.isCanceled())
            batch(hard_moves, results_ptr->hard);
    });

    m_analysis_watcher.setFuture(future);
}

void AnalysisTab::on_cancel_analysis_clicked() {
    if (m_is_analyzing) {
        m_analysis_watcher.cancel();
        m_ui->stop_analysis_btn->setEnabled(false);
    }
}

void AnalysisTab::on_analysis_finished() {
    m_is_analyzing = false;
    m_ui->run_analysis_btn->setEnabled(true);
    m_ui->stop_analysis_btn->setEnabled(false);

    if (m_analysis_watcher.isCanceled()) {
        QMessageBox::information(this, "Cancelado",
            "El análisis empírico fue cancelado.");
    } else if (m_pending_results) {
        update_all_charts(*m_pending_results);
        QMessageBox::information(this, "Completado",
            QString("Análisis para el tablero %1x%1 finalizado.")
                .arg(m_pending_results->board_dimension));
    }

    delete m_pending_results;
    delete m_pending_mutex;
    m_pending_results = nullptr;
    m_pending_mutex   = nullptr;
}

void AnalysisTab::on_dimension_changed(int value) {
    m_ui->dimension_value_label->setText(QString::number(value));
    m_goal_board = naidu::Board(static_cast<std::uint8_t>(value));
    m_ui->goal_input->clear();
}