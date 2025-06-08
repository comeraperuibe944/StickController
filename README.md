

# StickController

This project turns your M5StickCPlus2 into a xinput controller, supporting both physical and virtual controls via Wi-Fi.

## How to Use

There are two main ways to play:

1. **Physical Mode:**
   Use the physical buttons on the M5StickCPlus2 — they're mapped to standard XInput buttons.

2. **Virtual Mode (Wi-Fi):**
   The device creates a **Wi-Fi Access Point**. Connect your phone or PC to it, and use the on-screen virtual controller or keyboard.



## 🔧 Installation

It is **highly recommended** to burn the firmware using **[M5Burner](https://shop.m5stack.com/pages/download)** for noob-friendly setup process.

After uploading the code to the board, follow the on-screen instructions displayed on the device.

## 🛠️ How to Compile (Optional)

If you prefer compiling the code yourself using Arduino IDE:

1. Clone repo and open it in **Arduino IDE**
2. Connect **M5StickCPlus2** board ([official guide](https://docs.m5stack.com/en/arduino/m5stickc_plus2/program))
3. Install all required libraries:

   * Most can be installed via the **Library Manager** in Arduino IDE
   * For ([this custom library](https://github.com/Mystfit/ESP32-BLE-CompositeHID)), download as `.zip` and install via
     `Sketch > Include Library > Add .ZIP Library...`

