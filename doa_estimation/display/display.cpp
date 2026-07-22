#include "display.h"
#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

// 6x8 font data (same as before)
const uint8_t OLEDDisplay::font[][6] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // space
    {0x00, 0x00, 0x5F, 0x00, 0x00, 0x00}, // !
    {0x00, 0x07, 0x00, 0x07, 0x00, 0x00}, // "
    {0x14, 0x7F, 0x14, 0x7F, 0x14, 0x00}, // #
    {0x24, 0x2A, 0x7F, 0x2A, 0x12, 0x00}, // $
    {0x23, 0x13, 0x08, 0x64, 0x62, 0x00}, // %
    {0x36, 0x49, 0x55, 0x22, 0x50, 0x00}, // &
    {0x00, 0x05, 0x03, 0x00, 0x00, 0x00}, // '
    {0x00, 0x1C, 0x22, 0x41, 0x00, 0x00}, // (
    {0x00, 0x41, 0x22, 0x1C, 0x00, 0x00}, // )
    {0x08, 0x2A, 0x1C, 0x2A, 0x08, 0x00}, // *
    {0x08, 0x08, 0x3E, 0x08, 0x08, 0x00}, // +
    {0x00, 0x50, 0x30, 0x00, 0x00, 0x00}, // ,
    {0x08, 0x08, 0x08, 0x08, 0x08, 0x00}, // -
    {0x00, 0x60, 0x60, 0x00, 0x00, 0x00}, // .
    {0x20, 0x10, 0x08, 0x04, 0x02, 0x00}, // /
    {0x3E, 0x51, 0x49, 0x45, 0x3E, 0x00}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46, 0x00}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31, 0x00}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10, 0x00}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39, 0x00}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30, 0x00}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03, 0x00}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36, 0x00}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E, 0x00}, // 9
    {0x00, 0x36, 0x36, 0x00, 0x00, 0x00}, // :
    {0x00, 0x56, 0x36, 0x00, 0x00, 0x00}, // ;
    {0x08, 0x14, 0x22, 0x41, 0x00, 0x00}, // <
    {0x14, 0x14, 0x14, 0x14, 0x14, 0x00}, // =
    {0x00, 0x41, 0x22, 0x14, 0x08, 0x00}, // >
    {0x02, 0x01, 0x51, 0x09, 0x06, 0x00}, // ?
    {0x32, 0x49, 0x79, 0x41, 0x3E, 0x00}, // @
    {0x7E, 0x11, 0x11, 0x11, 0x7E, 0x00}, // A
    {0x7F, 0x49, 0x49, 0x49, 0x36, 0x00}, // B
    {0x3E, 0x41, 0x41, 0x41, 0x22, 0x00}, // C
    {0x7F, 0x41, 0x41, 0x22, 0x1C, 0x00}, // D
    {0x7F, 0x49, 0x49, 0x49, 0x41, 0x00}, // E
    {0x7F, 0x09, 0x09, 0x09, 0x01, 0x00}, // F
    {0x3E, 0x41, 0x49, 0x49, 0x7A, 0x00}, // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F, 0x00}, // H
    {0x00, 0x41, 0x7F, 0x41, 0x00, 0x00}, // I
    {0x20, 0x40, 0x41, 0x3F, 0x01, 0x00}, // J
    {0x7F, 0x08, 0x14, 0x22, 0x41, 0x00}, // K
    {0x7F, 0x40, 0x40, 0x40, 0x40, 0x00}, // L
    {0x7F, 0x02, 0x0C, 0x02, 0x7F, 0x00}, // M
    {0x7F, 0x04, 0x08, 0x10, 0x7F, 0x00}, // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E, 0x00}, // O
    {0x7F, 0x09, 0x09, 0x09, 0x06, 0x00}, // P
    {0x3E, 0x41, 0x51, 0x21, 0x5E, 0x00}, // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46, 0x00}, // R
    {0x46, 0x49, 0x49, 0x49, 0x31, 0x00}, // S
    {0x01, 0x01, 0x7F, 0x01, 0x01, 0x00}, // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F, 0x00}, // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F, 0x00}, // V
    {0x3F, 0x40, 0x38, 0x40, 0x3F, 0x00}, // W
    {0x63, 0x14, 0x08, 0x14, 0x63, 0x00}, // X
    {0x07, 0x08, 0x70, 0x08, 0x07, 0x00}, // Y
    {0x61, 0x51, 0x49, 0x45, 0x43, 0x00}, // Z
    {0x00, 0x7F, 0x41, 0x41, 0x00, 0x00}, // [
    {0x02, 0x04, 0x08, 0x10, 0x20, 0x00}, // backslash
    {0x00, 0x41, 0x41, 0x7F, 0x00, 0x00}, // ]
    {0x04, 0x02, 0x01, 0x02, 0x04, 0x00}, // ^
    {0x40, 0x40, 0x40, 0x40, 0x40, 0x00}, // _
    {0x00, 0x01, 0x02, 0x04, 0x00, 0x00}, // `
    {0x20, 0x54, 0x54, 0x54, 0x78, 0x00}, // a
    {0x7F, 0x48, 0x44, 0x44, 0x38, 0x00}, // b
    {0x38, 0x44, 0x44, 0x44, 0x20, 0x00}, // c
    {0x38, 0x44, 0x44, 0x48, 0x7F, 0x00}, // d
    {0x38, 0x54, 0x54, 0x54, 0x18, 0x00}, // e
    {0x08, 0x7E, 0x09, 0x01, 0x02, 0x00}, // f
    {0x0C, 0x52, 0x52, 0x52, 0x3E, 0x00}, // g
    {0x7F, 0x08, 0x04, 0x04, 0x78, 0x00}, // h
    {0x00, 0x44, 0x7D, 0x40, 0x00, 0x00}, // i
    {0x20, 0x40, 0x44, 0x3D, 0x00, 0x00}, // j
    {0x7F, 0x10, 0x28, 0x44, 0x00, 0x00}, // k
    {0x00, 0x41, 0x7F, 0x40, 0x00, 0x00}, // l
    {0x7C, 0x04, 0x18, 0x04, 0x78, 0x00}, // m
    {0x7C, 0x08, 0x04, 0x04, 0x78, 0x00}, // n
    {0x38, 0x44, 0x44, 0x44, 0x38, 0x00}, // o
    {0x7C, 0x14, 0x14, 0x14, 0x08, 0x00}, // p
    {0x08, 0x14, 0x14, 0x18, 0x7C, 0x00}, // q
    {0x7C, 0x08, 0x04, 0x04, 0x08, 0x00}, // r
    {0x48, 0x54, 0x54, 0x54, 0x20, 0x00}, // s
    {0x04, 0x3F, 0x44, 0x40, 0x20, 0x00}, // t
    {0x3C, 0x40, 0x40, 0x20, 0x7C, 0x00}, // u
    {0x1C, 0x20, 0x40, 0x20, 0x1C, 0x00}, // v
    {0x3C, 0x40, 0x30, 0x40, 0x3C, 0x00}, // w
    {0x44, 0x28, 0x10, 0x28, 0x44, 0x00}, // x
    {0x0C, 0x50, 0x50, 0x50, 0x3C, 0x00}, // y
    {0x44, 0x64, 0x54, 0x4C, 0x44, 0x00}, // z
    {0x00, 0x08, 0x36, 0x41, 0x00, 0x00}, // {
    {0x00, 0x00, 0x7F, 0x00, 0x00, 0x00}, // |
    {0x00, 0x41, 0x36, 0x08, 0x00, 0x00}, // }
    {0x08, 0x08, 0x2A, 0x1C, 0x08, 0x00}, // ~
};

OLEDDisplay::OLEDDisplay(const std::string& i2c_device, uint8_t address)
    : i2c_device_(i2c_device), i2c_address_(address), i2c_fd_(-1), running_(false) {
    buffer_ = new uint8_t[WIDTH * PAGES];
    memset(buffer_, 0, WIDTH * PAGES);
}

OLEDDisplay::~OLEDDisplay() {
    shutdown();
    if (i2c_fd_ >= 0) close(i2c_fd_);
    delete[] buffer_;
}

bool OLEDDisplay::i2cWrite(uint8_t* data, size_t length) {
    if (write(i2c_fd_, data, length) != (ssize_t)length) {
        return false;
    }
    return true;
}

void OLEDDisplay::sendCommand(uint8_t cmd) {
    uint8_t buffer[2] = {0x00, cmd};
    i2cWrite(buffer, 2);
    usleep(100);
}

void OLEDDisplay::update() {
    for (int page = 0; page < PAGES; page++) {
        sendCommand(0xB0 + page);
        sendCommand(0x00);
        sendCommand(0x10);
        
        uint8_t data[129];
        data[0] = 0x40;
        memcpy(data + 1, &buffer_[page * WIDTH], WIDTH);
        i2cWrite(data, WIDTH + 1);
        usleep(100);
    }
}

void OLEDDisplay::clear() {
    if (running_) {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        operation_queue_.emplace(DisplayOpType::CLEAR);
        queue_cv_.notify_one();
    } else {
        internalClear();
    }
}

void OLEDDisplay::addLine(const std::string& text) {
    if (running_) {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        operation_queue_.emplace(DisplayOpType::ADD_LINE, text);
        queue_cv_.notify_one();
    } else {
        internalAddLine(text);
    }
}

void OLEDDisplay::editLine(const std::string& text) {
    if (running_) {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        operation_queue_.emplace(DisplayOpType::EDIT_LINE, text);
        queue_cv_.notify_one();
    } else {
        internalEditLine(text);
    }
}

void OLEDDisplay::shutdown() {
    if (running_) {
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            operation_queue_.emplace(DisplayOpType::SHUTDOWN);
            queue_cv_.notify_one();
        }
        if (display_thread_.joinable()) {
            display_thread_.join();
        }
        running_ = false;
    }
}

void OLEDDisplay::drawChar(int x, int y, char c) {
    if (c < 32 || c > 126) c = 32;
    const uint8_t* char_data = font[c - 32];
    
    for (int row = 0; row < FONT_HEIGHT; row++) {
        for (int col = 0; col < FONT_WIDTH; col++) {
            if (char_data[col] & (1 << row)) {
                int px = x + col;
                int py = y + row;
                
                if (px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT) {
                    int page = py / 8;
                    int bit = py % 8;
                    buffer_[page * WIDTH + px] |= (1 << bit);
                }
            }
        }
    }
}

void OLEDDisplay::drawString(int x, int y, const std::string& text) {
    int currentX = x;
    for (char c : text) {
        if (currentX + FONT_WIDTH > WIDTH) break;
        drawChar(currentX, y, c);
        currentX += FONT_WIDTH;
    }
}

std::string OLEDDisplay::cropText(const std::string& text) {
    if (text.length() <= MAX_CHARS) {
        return text;
    }
    return text.substr(0, MAX_CHARS);
}

void OLEDDisplay::render() {
    // Clear the buffer first
    memset(buffer_, 0, WIDTH * PAGES);
    
    // Draw lines from bottom to top (line4 is bottom, line1 is top)
    drawString(0, FONT_HEIGHT * 7, line8_); // Bottom line (row 56)
    drawString(0, FONT_HEIGHT * 6, line7_); // Row 48
    drawString(0, FONT_HEIGHT * 5, line6_); // Row 40
    drawString(0, FONT_HEIGHT * 4, line5_); // Row 32    
    drawString(0, FONT_HEIGHT * 3, line4_); // Bottom line (row 24)
    drawString(0, FONT_HEIGHT * 2, line3_); // Third line (row 16)
    drawString(0, FONT_HEIGHT * 1, line2_); // Second line (row 8)
    drawString(0, FONT_HEIGHT * 0, line1_); // Top line (row 0)
    
    update();
}

void OLEDDisplay::internalAddLine(const std::string& text) {
    // Scroll up: old line2 becomes line1, line3 becomes line2, line4 becomes line3
    line1_ = line2_;
    line2_ = line3_;
    line3_ = line4_;
    line4_ = line5_;
    line5_ = line6_;
    line6_ = line7_;
    line7_ = line8_;
    // New text becomes line8 (bottom)
    line8_ = cropText(text);
    
    render();
}

void OLEDDisplay::internalEditLine(const std::string& text) {
    // Edit the last line (line4_) without scrolling
    line4_ = cropText(text);
    render();
}

void OLEDDisplay::internalClear() {
    line1_.clear();
    line2_.clear();
    line3_.clear();
    line4_.clear();
    line5_.clear();
    line6_.clear();
    line7_.clear();
    line8_.clear();
    memset(buffer_, 0, WIDTH * PAGES);
    update();
}

void OLEDDisplay::displayThreadFunc() {
    while (running_) {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        
        // Wait for operations or timeout (to allow periodic updates if needed)
        queue_cv_.wait_for(lock, std::chrono::milliseconds(100), 
                          [this] { return !operation_queue_.empty(); });
        
        // Process all pending operations
        while (!operation_queue_.empty()) {
            DisplayOperation op = std::move(operation_queue_.front());
            operation_queue_.pop();
            lock.unlock();  // Unlock while processing
            
            switch (op.type) {
                case DisplayOpType::ADD_LINE:
                    internalAddLine(op.text);
                    break;
                case DisplayOpType::EDIT_LINE:
                    internalEditLine(op.text);
                    break;
                case DisplayOpType::CLEAR:
                    internalClear();
                    break;
                case DisplayOpType::SHUTDOWN:
                    return;
            }
            
            lock.lock();  // Re-lock for next iteration
        }
    }
}

bool OLEDDisplay::init() {
    i2c_fd_ = open(i2c_device_.c_str(), O_RDWR);
    if (i2c_fd_ < 0) {
        std::cerr << "Failed to open I2C device: " << i2c_device_ << std::endl;
        return false;
    }
    
    if (ioctl(i2c_fd_, I2C_SLAVE, i2c_address_) < 0) {
        std::cerr << "Failed to set I2C address: 0x" << std::hex << (int)i2c_address_ << std::endl;
        close(i2c_fd_);
        return false;
    }
    
    std::cout << "I2C connected to " << i2c_device_ << " at 0x" << std::hex << (int)i2c_address_ << std::endl;
    
    // SH1106 initialization for 128x32
    sendCommand(0xAE); // Display off
    sendCommand(0xD5); sendCommand(0x80);
    //sendCommand(0xA8); sendCommand(0x1F); // 32 rows
    if (HEIGHT == 64) {
        sendCommand(0xA8); sendCommand(0x3F); // 64 rows
        sendCommand(0xDA); sendCommand(0x12); // COM pins for 128x64
    } else { // Assume 32 rows
        sendCommand(0xA8); sendCommand(0x1F); // 32 rows
        sendCommand(0xDA); sendCommand(0x02); // COM pins for 128x32
    }    

    sendCommand(0xD3); sendCommand(0x00);
    sendCommand(0x40);
    sendCommand(0x8D); sendCommand(0x14); // Charge pump enable
    sendCommand(0x20); sendCommand(0x00);
    
    // FOR ROTATED DISPLAY (180 degrees):
    sendCommand(0xA0); // Instead of 0xA1 (segment remap - mirrored horizontally)
    sendCommand(0xC0); // Instead of 0xC8 (COM scan direction - bottom to top)
    
    //sendCommand(0xDA); sendCommand(0x02); // COM pins for 128x32
    sendCommand(0x81); sendCommand(0x80);
    sendCommand(0xD9); sendCommand(0xF1);
    sendCommand(0xDB); sendCommand(0x40);
    sendCommand(0xA4);
    sendCommand(0xA6);
    sendCommand(0xAF); // Display on
    
    usleep(10000);
    
    // Clear the display first
    internalClear();
    
    // Start the display thread
    running_ = true;
    display_thread_ = std::thread(&OLEDDisplay::displayThreadFunc, this);
    
    return true;
}