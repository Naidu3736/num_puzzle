#pragma once

#include "ui/analysis/analysis_tab.h"
#include "ui/puzzle/puzzle_tab.h"

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = nullptr);
        ~MainWindow() override;

        PuzzleTab* get_active_puzzle_tab() const;
        AnalysisTab* get_active_analysis_tab() const;
        std::vector<std::uint8_t> parse_board_grid(std::ifstream& file, std::uint8_t dim) const;

    private slots:
        void action_load_board_triggered();
        void action_save_analysis_triggered();
        void action_quit_triggered();

    private:
        Ui::MainWindow *m_ui;
};