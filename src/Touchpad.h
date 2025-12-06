#pragma once

#include <QPoint>
#include <QSize>

class TouchPoint
{
public:
    bool active() const { return m_active; }
    void setActive(bool value) { m_active = value; }

    const QPoint &position() const { return m_position; }
    void setPosition(QPoint value) { m_position = std::move(value); }

private:
    bool m_active{};
    QPoint m_position;
};

class Touchpad
{
public:
    bool clicked() const { return m_clicked; }
    void setClicked(bool value) { m_clicked = value; }

    const QSize &size() const { return m_size; }
    void setSize(QSize value) { m_size = value; }

    const std::vector<TouchPoint> &touchPoints() const { return m_touchPoints; }
    std::vector<TouchPoint> &touchPoints() { return m_touchPoints; }
    void setTouchPoints(std::vector<TouchPoint> value) { m_touchPoints = std::move(value); }

private:
    bool m_clicked{};
    QSize m_size;
    std::vector<TouchPoint> m_touchPoints;
};