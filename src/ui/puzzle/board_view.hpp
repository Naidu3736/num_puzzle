#pragma once 

#include "board/board.hpp"

#include <QWidget>
#include <qtconfigmacros.h>
#include <qtmetamacros.h>
#include <qwidget.h>

class BoardView : public QWidget {
    Q_OBJECT

    private:
        naidu::Board m_board;
        bool m_has_board;
        static constexpr int MARGIN = 4;
        static constexpr int MIN_CELL = 30;
    
    public:
        explicit BoardView(QWidget *parent);

        /**
         * @brief Recibe y almacena una copia del tablero lógico del núcleo (naidu::Board) para actualizar su representación visual.
         * @param board Referencia al tablero del núcleo que se va a renderizar.
         */
        void set_board(const naidu::Board& board);
        void clear();

        QSize sizeHint() const override;

    protected:
        void paintEvent(QPaintEvent *event) override;
};

