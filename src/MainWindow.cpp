/*
    InputActions Touchpad Input Visualizer
    Copyright (C) 2025 Marcin Woźniak

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "MainWindow.h"
#include "TouchpadHandler.h"
#include <QPainter>

static const std::vector<QColor> COLORS{
    QColor::fromString("#da4453"),
    QColor::fromString("#27ae60"),
    QColor::fromString("#3daee9"),
    QColor::fromString("#9b59b6"),
    QColor::fromString("#f67400"),
};

MainWindow::MainWindow()
{
    connect(g_touchpadHandler.get(), &TouchpadHandler::touchpadStateChanged, this, [this]() {
        repaint();
    });
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QFont font;
    font.setBold(true);
    font.setPixelSize(24);
    painter.setFont(font);

    painter.setPen(Qt::NoPen);

    if (const auto &touchpad = g_touchpadHandler->activeTouchpad()) {
        if (touchpad->clicked()) {
            painter.setBrush(QColor(255, 255, 255, 10));
            painter.drawRect(0, 0, width(), height());
        }

        const auto &touchPoints = touchpad->touchPoints();
        for (size_t i = 0; i < touchPoints.size(); ++i) {
            const auto &point = touchPoints[i];
            if (!point.active()) {
                continue;
            }

            const auto &color = COLORS.at(i % COLORS.size());
            painter.setBrush(color);

            const auto radius = 20;
            const QPointF center(point.position().x() / static_cast<qreal>(touchpad->size().width()) * width(),
                                 point.position().y() / static_cast<qreal>(touchpad->size().height()) * height());
            painter.drawEllipse(center, radius, radius);

            QTextOption option;
            option.setAlignment(Qt::AlignCenter);
            painter.setPen(palette().color(QPalette::Window));
            painter.drawText(QRectF(center.x() - radius, center.y() - radius, radius * 2, radius * 2), QString::number(i), option);
        }
    }
}
