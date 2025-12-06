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

#include "TouchpadHandler.h"
#include <QDir>
#include <fcntl.h>
#include <sys/inotify.h>

TouchpadHandler::TouchpadHandler()
{
    for (const auto &entry : QDir("/dev/input").entryInfoList(QDir::Files | QDir::NoSymLinks | QDir::System)) {
        evdevDeviceAdded(entry.filePath());
    }

    m_inotifyFd = inotify_init();
    fcntl(m_inotifyFd, F_SETFD, FD_CLOEXEC);
    fcntl(m_inotifyFd, F_SETFL, fcntl(m_inotifyFd, F_GETFL, 0) | O_NONBLOCK);

    m_inotifyNotifier = std::make_unique<QSocketNotifier>(m_inotifyFd, QSocketNotifier::Read);
    m_inotifyNotifier->setEnabled(false);
    connect(m_inotifyNotifier.get(), &QSocketNotifier::activated, this, &TouchpadHandler::inotifyTimerTick);
    inotify_add_watch(m_inotifyFd, "/dev/input", IN_CREATE | IN_DELETE);
}

const Touchpad *TouchpadHandler::activeTouchpad()
{
    return m_activeTouchpad ? &m_activeTouchpad->touchpad : nullptr;
}

void TouchpadHandler::evdevDeviceAdded(const QString &path)
{
    if (!path.startsWith("/dev/input/event")) {
        return;
    }

    const auto fd = open(path.toStdString().c_str(), O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        qWarning() << "Failed to open device";
        return;
    }

    auto device = std::make_unique<EvdevDevice>();
    device->path = path;
    if (libevdev_new_from_fd(fd, &device->libevdev) != 0) {
        return;
    }

    device->notifier = std::make_unique<QSocketNotifier>(fd, QSocketNotifier::Read);
    connect(device->notifier.get(), &QSocketNotifier::activated, this, [this, devicePtr = device.get()]() {
        poll(devicePtr);
    });

    if (!libevdev_has_event_type(device->libevdev, EV_ABS)) {
        return;
    }

    const auto *x = libevdev_get_abs_info(device->libevdev, ABS_X);
    const auto *y = libevdev_get_abs_info(device->libevdev, ABS_Y);
    if (!x || !y) {
        return;
    }

    device->absMin = {std::abs(x->minimum), std::abs(y->minimum)};
    const QSize size(device->absMin.x() + x->maximum, device->absMin.y() + y->maximum);
    if (size.width() == 0 || size.height() == 0) {
        return;
    }
    device->touchpad.setSize(size);

    device->buttonPad = libevdev_has_property(device->libevdev, INPUT_PROP_BUTTONPAD) == 1;
    uint8_t slotCount = 1;
    if (libevdev_has_event_code(device->libevdev, EV_ABS, ABS_MT_SLOT)) {
        slotCount = libevdev_get_abs_maximum(device->libevdev, ABS_MT_SLOT) + 1;
    }
    device->touchpad.setTouchPoints(std::vector<TouchPoint>(slotCount));

    m_devices.push_back(std::move(device));
}

void TouchpadHandler::evdevDeviceRemoved(const QString &path)
{
    for (auto it = m_devices.begin(); it != m_devices.end();) {
        if (it->get()->path == path) {
            m_devices.erase(it);
            break;
        }
    }
}

void TouchpadHandler::poll(EvdevDevice *device)
{
    m_activeTouchpad = device;

    input_event event;
    int status{};
    while (true) {
        auto flags = status == LIBEVDEV_READ_STATUS_SYNC ? LIBEVDEV_READ_FLAG_SYNC : LIBEVDEV_READ_FLAG_NORMAL;
        status = libevdev_next_event(device->libevdev, flags, &event);
        if (status != LIBEVDEV_READ_STATUS_SUCCESS && status != LIBEVDEV_READ_STATUS_SYNC) {
            break;
        }

        handleEvdevEvent(device, event);
    }

    Q_EMIT touchpadStateChanged();
}

void TouchpadHandler::handleEvdevEvent(EvdevDevice *sender, const input_event &event)
{
    const auto code = event.code;
    const auto value = event.value;
    switch (event.type) {
        case EV_KEY:
            switch (code) {
                case BTN_LEFT:
                case BTN_MIDDLE:
                case BTN_RIGHT:
                    if (sender->buttonPad) {
                        sender->touchpad.setClicked(value);
                    }
                    break;
            }
            break;
        case EV_ABS:
            auto &currentTouchPoint = sender->touchpad.touchPoints()[sender->currentSlot];
            switch (code) {
                case ABS_MT_SLOT:
                    sender->currentSlot = value;
                    break;
                case ABS_MT_TRACKING_ID:
                    currentTouchPoint.setActive(value != -1);
                    break;
                case ABS_MT_POSITION_X:
                    currentTouchPoint.setPosition({value + sender->absMin.x(), currentTouchPoint.position().y()});
                    break;
                case ABS_MT_POSITION_Y:
                    currentTouchPoint.setPosition({currentTouchPoint.position().x(), value + sender->absMin.y()});
                    break;
            }
            break;
    }
}

void TouchpadHandler::inotifyTimerTick()
{
    std::array<int8_t, 16 * (sizeof(inotify_event) + NAME_MAX + 1)> buffer{};
    while (true) {
        auto length = read(m_inotifyFd, &buffer, sizeof(buffer));
        if (length <= 0) {
            break;
        }

        for (int i = 0; i < length;) {
            auto *event = (inotify_event *)&buffer[i];
            const auto path = QString("/dev/input/%1").arg(QString::fromLocal8Bit(event->name, event->len).replace('\0', ""));

            if (event->mask & IN_CREATE) {
                evdevDeviceAdded(path);
            } else if (event->mask & IN_DELETE) {
                evdevDeviceRemoved(path);
            }
            i += sizeof(inotify_event) + event->len;
        }
    }
}