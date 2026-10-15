#include "board_view.hpp"

#include <qnamespace.h>
#include <qsize.h>
#include <qsizepolicy.h>
#include <qwidget.h>
#include <QPainter>
#include <QPainterPath>

BoardView::BoardView(QWidget *parent) 
    : QWidget(parent)
    , m_has_board(false)
{   
    setMinimumSize(200, 200);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void BoardView::set_board(const naidu::Board& board) {
    m_board = board;
    m_has_board = true;
    update();
}

void BoardView::clear() {
    m_has_board = false;
    update();
}

QSize BoardView::sizeHint() const {
    const int side = (m_has_board ? m_board.dim() : 5) * MIN_CELL + 2 * MARGIN;
    return QSize(side, side);
}

void BoardView::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    p.fillRect(rect(), QColor(30, 32, 38));

    if (!m_has_board) {
        p.setPen(QColor(140, 145, 155));
        QFont placeholder_font = p.font();
        placeholder_font.setPointSize(12);
        p.setFont(placeholder_font);
        p.drawText(rect(), Qt::AlignCenter, "Sin tablero");
        return;
    }

    const int n = m_board.dim();
    const int avail = std::min(width(), height()) - 2 * MARGIN;
    const int cell = avail / n;
    const int board_size = cell * n;

    const int x0 = (width() - board_size) / 2;
    const int y0 = (height() - board_size) / 2;

    p.setPen(QPen(QColor(50, 54, 64), 2));
    p.setBrush(QColor(24, 26, 31));
    p.drawRoundedRect(x0 - 4, y0 - 4, board_size + 8, board_size + 8, 8, 8);

    QFont font = p.font();
    font.setPixelSize(std::max(10, cell / 3));
    font.setBold(true);
    p.setFont(font);

    const int radius = std::max(4, cell / 10);

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            const int x = x0 + j * cell + 2;
            const int y = y0 + i * cell + 2;
            const int inner_size = cell - 4;
            const QRect r(x, y, inner_size, inner_size);

            const std::uint8_t tile = m_board.at(i, j);

            if (tile == 0) {
                p.setPen(Qt::NoPen);
                p.setBrush(QColor(38, 41, 48));
                p.drawRoundedRect(r, radius, radius);
            } else {
                p.setPen(QPen(QColor(70, 130, 200), 1));
                p.setBrush(QColor(41, 128, 185));
                p.drawRoundedRect(r, radius, radius);

                p.setPen(QColor(255, 255, 255));
                p.drawText(r, Qt::AlignCenter, QString::number(tile));
            }
        }
    }
}