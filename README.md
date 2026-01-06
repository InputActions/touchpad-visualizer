# Touchpad Visualizer
> [!IMPORTANT]  
> This tool is internal and intended to be used only for the development of [InputActions](https://github.com/taj-ny/InputActions). Feature requests are likely to be rejected.

# Building

Dependencies:
- Extra CMake Modules
- Qt6 Widgets
- libevdev

```
git clone https://github.com/InputActions/touchpad-visualizer
cd touchpad-visualizer
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr
make -j$(nproc)
sudo make install
```

# Usage
Run ``inputactions-touchpad-visualizer``.

Read access to the touchpad device is required. The tool can be run as root, or a udev rule may be created at ``/etc/udev/rules.d/71-touchpad.rules`` with the following contents:
```
ENV{ID_INPUT_TOUCHPAD}=="1", TAG+="uaccess"
```
This will give all programs read and write access to all touchpads.

# Showcase
https://github.com/user-attachments/assets/108d20bc-7a42-4d55-b3c1-a43a6ff8d5c7
