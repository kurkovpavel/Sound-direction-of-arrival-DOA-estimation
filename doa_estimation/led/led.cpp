#include "led.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <cmath>

LEDController::LEDController(int num_leds) 
    : spi_fd(-1), num_leds(num_leds), initialized(false) {
    buffer.resize(4 * num_leds, 0);
}

LEDController::~LEDController() {
    shutdown();
}

bool LEDController::initialize() {
    // Open SPI device
    spi_fd = open("/dev/spidev0.1", O_RDWR);
    if (spi_fd < 0) {
        std::cerr << "Failed to open SPI device" << std::endl;
        return false;
    }
    
    // Configure SPI
    uint8_t mode = SPI_MODE_0;
    uint8_t bits = 8;
    uint32_t speed = 8000000;
    
    if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) < 0) {
        std::cerr << "Failed to set SPI mode" << std::endl;
        close(spi_fd);
        spi_fd = -1;
        return false;
    }
    
    if (ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) {
        std::cerr << "Failed to set SPI bits per word" << std::endl;
        close(spi_fd);
        spi_fd = -1;
        return false;
    }
    
    if (ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
        std::cerr << "Failed to set SPI speed" << std::endl;
        close(spi_fd);
        spi_fd = -1;
        return false;
    }
    
    initialized = true;
    off();
    
    return true;
}

void LEDController::shutdown() {
    if (initialized) {
        off();
        if (spi_fd >= 0) {
            close(spi_fd);
            spi_fd = -1;
        }
        initialized = false;
    }
}

void LEDController::send_frame() {
    if (spi_fd < 0 || !initialized) return;
    
    // Start frame (4 zero bytes)
    uint8_t start[4] = {0, 0, 0, 0};
    write(spi_fd, start, 4);
    
    // LED data
    write(spi_fd, buffer.data(), buffer.size());
    
    // End frame
    int end_bytes = (num_leds + 15) / 16;
    std::vector<uint8_t> end(end_bytes, 0);
    write(spi_fd, end.data(), end_bytes);
}

void LEDController::show_direction(double angle) {
    if (!initialized) return;
    
    // Normalize angle
    while (angle < 0) angle += 360.0;
    while (angle >= 360.0) angle -= 360.0;
    
    // Calculate LED index (0-11)
    int led_index = static_cast<int>(std::round(angle / 30.0)) % num_leds;
    
    // Build LED data
    for (int i = 0; i < num_leds; i++) {
        int pos = i * 4;
        if (i == led_index) {
            // LED on - bright blue
            buffer[pos] = 0b11111111;     // Full brightness
            buffer[pos + 1] = 0;          // Red
            buffer[pos + 2] = 100;        // Green
            buffer[pos + 3] = 255;        // Blue
        } else {
            // LED off
            buffer[pos] = 0b11100000;     // Brightness 0
            buffer[pos + 1] = 0;
            buffer[pos + 2] = 0;
            buffer[pos + 3] = 0;
        }
    }
    
    send_frame();
}

void LEDController::off() {
    if (!initialized) return;
    
    // Turn off all LEDs
    for (int i = 0; i < num_leds; i++) {
        int pos = i * 4;
        buffer[pos] = 0b11100000;     // Brightness 0
        buffer[pos + 1] = 0;
        buffer[pos + 2] = 0;
        buffer[pos + 3] = 0;
    }
    
    send_frame();
}