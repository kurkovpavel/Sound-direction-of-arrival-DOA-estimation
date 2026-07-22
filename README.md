# Seeed Voicecard Driver & DOA Estimation (Kernel 6.1.21+ Fork)

This repository is a fork of [respeaker/seeed-voicecard](https://github.com/respeaker/seeed-voicecard). It includes updated source code to support modern Raspberry Pi Linux kernels, along with a practical example for Direction of Arrival (DOA) estimation using the built-in LED ring.

---

## 📂 Repository Structure

*   **`seeed-voicecard_driver_for_kernel6.1.21`**: Contains the updated driver source code compiled and optimized specifically for the Linux Raspberry kernel version `6.1.21-v8+`.
*   **`doa_estimation`**: An example application demonstrating Direction of Arrival estimation and LED control.

---

## 🛠️ Installation & Driver Setup

To build and install the sound card drivers on your Raspberry Pi:

1. Navigate to the driver directory:
   ```bash
   cd seeed-voicecard_driver_for_kernel6.1.21
   ```
2. Follow the specific instructions provided in the local `README` file inside that folder to compile and insert the kernel modules.

---

## 🎙️ DOA Estimation Example

The `doa_estimation` folder contains an application that calculates the direction of incoming sound and visualizes it using the sound card's onboard LEDs.

### Building the Project
Navigate to the directory and compile the source code:
```bash
cd doa_estimation
make clean
make
```

### Running the Application
Start the DOA system by running:
```bash
./doa_estimation
```

### Application Controls
Once running, you can interact with the system using your keyboard:

| Key | Action |
| :--- | :--- |
| `Up` / `Down` | Shift LED direction by **5°** |
| `Left` / `Right` | Shift LED direction by **1°** (fine adjustment) |
| `S` | Toggle shift **ON / OFF** |
| `R` | Reset shift to **0°** |
| `Q` | **Quit** the application |

---

## 🔍 Troubleshooting

### Thread Affinity Error
If you see the following message during startup:
```text
Failed to set thread affinity: No such device or address
```
*Note: This is a known warning regarding CPU core binding on certain kernel configurations. The application will still run successfully right after displaying it.*

### LEDs Are Not Lighting Up
If the system states `DOA running` but the physical hardware LEDs do not turn on:
1. Ensure your hardware connections are secure.
2. Run the provided GPIO diagnosis script to check the interface pins:
   ```bash
   python3 checkgpio.py
   ```

