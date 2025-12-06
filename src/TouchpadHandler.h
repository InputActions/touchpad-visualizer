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

#pragma once

#include "Touchpad.h"
#include <QSocketNotifier>
#include <libevdev/libevdev.h>

struct EvdevDevice
{
    QString path;
    struct libevdev *libevdev{};
    std::unique_ptr<QSocketNotifier> notifier;

    Touchpad touchpad;

    uint8_t currentSlot{};
    /**
     * Absolute minimum values of ABS_X and ABS_Y.
     */
    QPoint absMin;
    bool buttonPad;
};

class TouchpadHandler : public QObject
{
    Q_OBJECT

public:
    TouchpadHandler();
    ~TouchpadHandler() override = default;

    const Touchpad *activeTouchpad();

signals:
    void touchpadStateChanged();

private slots:
    void inotifyTimerTick();

private:
    void evdevDeviceAdded(const QString &path);
    void evdevDeviceRemoved(const QString &path);

    void poll(EvdevDevice *device);
    void handleEvdevEvent(EvdevDevice *sender, const input_event &event);

    int m_inotifyFd;
    std::unique_ptr<QSocketNotifier> m_inotifyNotifier;

    EvdevDevice *m_activeTouchpad{};
    std::vector<std::unique_ptr<EvdevDevice>> m_devices;
};

inline std::shared_ptr<TouchpadHandler> g_touchpadHandler;