#ifndef DISPLAY_H
#define DISPLAY_H

#include <string>
#include <cstdint>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>

enum class DisplayOpType {
    ADD_LINE,
    EDIT_LINE,
    CLEAR,
    SHUTDOWN
};

struct DisplayOperation {
    DisplayOpType type;
    std::string text;
    
    DisplayOperation(DisplayOpType t, const std::string& txt = "") 
        : type(t), text(txt) {}
};

class OLEDDisplay {
public:
    OLEDDisplay(const std::string& i2c_device = "/dev/i2c-1", uint8_t address = 0x3C);
    ~OLEDDisplay();
    
    bool init();
    void clear();  // Non-blocking
    void addLine(const std::string& text);  // Non-blocking
    void editLine(const std::string& text);  // Non-blocking
    
    void shutdown();  // Stop the display thread
    
private:
    std::string i2c_device_;
    uint8_t i2c_address_;
    int i2c_fd_;
    
    static const int WIDTH = 128;
    static const int HEIGHT = 64;
    static const int PAGES = HEIGHT / 8;
    static const int FONT_WIDTH = 6;
    static const int FONT_HEIGHT = 8;
    static const int MAX_CHARS = 21; // 128 / 6
    
    static const uint8_t font[][6];
    uint8_t* buffer_;
    
    std::string line1_;
    std::string line2_;
    std::string line3_;
    std::string line4_;
    std::string line5_;
    std::string line6_;
    std::string line7_;
    std::string line8_;
    
    // Threading members
    std::thread display_thread_;
    std::queue<DisplayOperation> operation_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    std::atomic<bool> running_;
    
    bool i2cWrite(uint8_t* data, size_t length);
    void sendCommand(uint8_t cmd);
    void update();
    void drawChar(int x, int y, char c);
    void drawString(int x, int y, const std::string& text);
    void render();
    std::string cropText(const std::string& text);
    
    // Thread function
    void displayThreadFunc();
    
    // Internal methods that actually perform the operations
    void internalAddLine(const std::string& text);
    void internalEditLine(const std::string& text);
    void internalClear();
};

#endif