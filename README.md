# ESP32 Web Server with WiFi Configuration Portal

A lightweight, production-ready ESP32 web server with an intuitive WiFi configuration portal. This educational project demonstrates best practices for ESP32 development, including persistent storage, async web serving, and automatic WiFi reconnection.

## 🌟 Features

- **WiFi Configuration Portal**: Easy-to-use web interface for WiFi setup
- **Access Point Mode**: Automatically creates a hotspot when no WiFi credentials are stored
- **Persistent Storage**: WiFi credentials saved using ESP32 Preferences (NVS)
- **Async Web Server**: Non-blocking web server using ESPAsyncWebServer
- **Auto-Reconnection**: Automatic WiFi reconnection with fallback to AP mode
- **Input Validation**: Client-side and server-side validation for WiFi credentials
- **LittleFS Support**: Serves HTML files from the filesystem
- **Responsive Design**: Clean, mobile-friendly web interface
- **Memory Optimized**: String pre-allocation to prevent heap fragmentation
- **Error Handling**: Comprehensive error checking for all operations

## 📋 Table of Contents

- [Hardware Requirements](#hardware-requirements)
- [Software Requirements](#software-requirements)
- [Installation](#installation)
- [Usage](#usage)
- [Project Structure](#project-structure)
- [Configuration](#configuration)
- [How It Works](#how-it-works)
- [Troubleshooting](#troubleshooting)
- [Contributing](#contributing)
- [License](#license)
- [Acknowledgments](#acknowledgments)

## 🔧 Hardware Requirements

This project works with any ESP32 development board. It has been tested with:

- **WeMos WiFi&Bluetooth Battery** ([ESP32-based board](https://www.espboards.dev/esp32/wemosbat/))
- Any other ESP32 board (ESP32-DevKitC, ESP32-WROOM, NodeMCU-32S, etc.)

No additional hardware components are required.

## 💻 Software Requirements

- [PlatformIO](https://platformio.org/) (IDE or CLI)
- USB cable for programming the ESP32
- Any modern web browser for accessing the WiFi portal

## 📦 Installation

### 1. Clone the Repository

```bash
git clone https://github.com/shinobi81/esp32-wifi-webserver.git
cd esp32-wifi-webserver
```

### 2. Open in PlatformIO

Open the project folder in Visual Studio Code with PlatformIO extension, or use PlatformIO CLI.

### 3. Configure Board (Optional)

If you're not using the WeMos WiFi&Bluetooth Battery, modify `platformio.ini`:

```ini
[env:your_board]
platform = espressif32
board = your_board_name
framework = arduino
```

### 4. Build the Filesystem

```bash
pio run --target buildfs
```

### 5. Upload Filesystem

```bash
pio run --target uploadfs
```

### 6. Build and Upload Code

```bash
pio run --target upload
```

### 7. Monitor Serial Output

```bash
pio device monitor
```

## 🚀 Usage

### First Time Setup

1. **Power on the ESP32** - On first boot, the device will start in Access Point mode
2. **Connect to the WiFi network** - Look for a network named `ESP32-SETUP-XXXXXXXX`
3. **Open your browser** - Navigate to any website (you'll be redirected) or go to `192.168.4.1`
4. **Enter WiFi credentials**:
   - SSID: Your WiFi network name (1-32 characters)
   - Password: Your WiFi password (8-63 characters or leave empty for open networks)
5. **Save and restart** - The ESP32 will restart and connect to your WiFi

### Normal Operation

Once configured, the ESP32 will:
- Automatically connect to the saved WiFi network on boot
- Display the main web page at its assigned IP address
- Monitor the WiFi connection and automatically reconnect if disconnected
- Fall back to AP mode after 3 failed reconnection attempts

### Reconfiguration

To change WiFi settings:
1. Connect to the ESP32's IP address in your browser
2. Navigate to `/wifi_setup.html`
3. Enter new credentials and save

## 📁 Project Structure

```
esp32-wifi-webserver/
├── src/
│   └── main.cpp              # Main application code
├── data/
│   ├── index.html            # Main webpage
│   └── wifi_setup.html       # WiFi configuration page
├── include/
│   └── README                # Header files directory
├── lib/
│   └── README                # Private libraries
├── platformio.ini            # PlatformIO configuration
└── README.md                 # This file
```

## ⚙️ Configuration

### WiFi Settings

Edit these constants in `main.cpp` to customize behavior:

```cpp
const int WIFI_CONNECT_TIMEOUT_ATTEMPTS = 20;  // Connection timeout (20 * 500ms = 10s)
const int WIFI_CONNECT_DELAY_MS = 500;         // Delay between connection checks
const unsigned long WIFI_CHECK_INTERVAL_MS = 10000;  // WiFi monitoring interval
const int WIFI_RECONNECT_ATTEMPTS = 3;         // Max reconnection attempts
```

### Validation Rules

```cpp
const int MIN_SSID_LENGTH = 1;          // Minimum SSID length
const int MAX_SSID_LENGTH = 32;         // Maximum SSID length (WiFi standard)
const int MIN_PASSWORD_LENGTH = 8;      // Minimum password length
const int MAX_PASSWORD_LENGTH = 63;     // Maximum password length (WiFi standard)
```

## 🔍 How It Works

### Boot Sequence

1. **Initialize**: Mount LittleFS, reserve memory for strings
2. **Load Credentials**: Check Preferences for saved WiFi credentials
3. **Connect or AP Mode**:
   - If credentials exist → Attempt to connect to WiFi
   - If no credentials or connection fails → Start Access Point mode
4. **Start Web Server**: Initialize routes and begin serving pages

### WiFi Monitoring

In the main loop, the system:
- Checks WiFi connection every 10 seconds (in station mode only)
- Attempts reconnection if disconnected (up to 3 times)
- Switches to AP mode if all reconnection attempts fail

### Data Persistence

WiFi credentials are stored in ESP32's NVS (Non-Volatile Storage) using the Preferences library:
- Namespace: `wifi`
- Keys: `ssid`, `password`
- Survives power cycles and firmware updates (unless NVS is erased)

## 🐛 Troubleshooting

### Cannot Connect to AP Mode

- Ensure the ESP32 is powered on
- Look for network starting with `ESP32-SETUP-`
- Some devices may take 10-30 seconds to show available networks

### WiFi Credentials Not Saving

- Check serial monitor for error messages
- Verify NVS partition is not corrupted (erase flash and re-upload)
- Ensure credentials meet validation requirements

### Web Page Not Loading

- Verify filesystem was uploaded (`pio run --target uploadfs`)
- Check serial monitor for LittleFS mount errors
- Try erasing flash: `pio run --target erase` then re-upload everything

### ESP32 Keeps Restarting

- Check power supply (USB cable/power source)
- Monitor serial output for crash logs
- May need to erase flash and start fresh

## 🤝 Contributing

This is an educational project, and contributions are welcome! Feel free to:

- Report bugs or issues
- Suggest new features or improvements
- Submit pull requests
- Share your customizations

## 📄 License

This project is licensed under the MIT License - see below for details:

```
MIT License

Copyright (c) 2026

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

## 🙏 Acknowledgments

- Built with [PlatformIO](https://platformio.org/)
- [ESPAsyncWebServer](https://github.com/ESP32Async/ESPAsyncWebServer) library (actively maintained fork)
- [LittleFS](https://github.com/littlefs-project/littlefs) filesystem
- ESP32 Arduino Core and Preferences library
- Inspired by the ESP32 community and open-source projects

---

**Note**: This is an educational project designed as a starting point for ESP32 web server applications. Feel free to customize and extend it for your specific needs!
