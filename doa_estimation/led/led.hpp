#ifndef LED_HPP
#define LED_HPP

#include <vector>
#include <cstdint>
#include <string>

class LEDController {
private:
    int spi_fd;
    int num_leds;
    std::vector<uint8_t> buffer;
    bool initialized;
    
    void send_frame();
    
public:
    LEDController(int num_leds = 12);
    ~LEDController();
    
    bool initialize();
    void shutdown();
    bool is_initialized() const { return initialized; }
    
    void show_direction(double angle);
    void off();
    
    // Dummy functions (do nothing)
    void wakeup(int) {}
    void listen() {}
    void think() {}
    void speak() {}
    void show(const std::vector<uint8_t>&) {}
};

#endif