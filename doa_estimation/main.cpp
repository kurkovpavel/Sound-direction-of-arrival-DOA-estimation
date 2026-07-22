#include <iostream>
#include <vector>
#include <cmath>
#include <thread>
#include <chrono>
#include <atomic>
#include <signal.h>
#include <iomanip>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

#include "audio/audio_capture.hpp"
#include "audio/doa.hpp"
#include "led/led.hpp"

using namespace std;

// Key codes
#define KEY_UP 1000
#define KEY_DOWN 1001
#define KEY_LEFT 1002
#define KEY_RIGHT 1003

atomic<bool> running{true};
AudioCapture* audio = nullptr;
LEDController* leds = nullptr;
atomic<double> current_angle{0.0};
atomic<double> led_shift{0.0};  // Shift applied to LED
atomic<bool> shift_active{false};

// Keyboard input
struct termios old_termios;

void reset_terminal() {
    tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);
}

void init_terminal() {
    struct termios new_termios;
    tcgetattr(STDIN_FILENO, &old_termios);
    new_termios = old_termios;
    new_termios.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
    
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
}

int get_key() {
    char ch;
    int n = read(STDIN_FILENO, &ch, 1);
    if (n == 1) {
        if (ch == 27) {
            char seq[2];
            if (read(STDIN_FILENO, &seq[0], 1) == 1 &&
                read(STDIN_FILENO, &seq[1], 1) == 1) {
                if (seq[0] == '[') {
                    if (seq[1] == 'A') return KEY_UP;
                    if (seq[1] == 'B') return KEY_DOWN;
                    if (seq[1] == 'C') return KEY_RIGHT;
                    if (seq[1] == 'D') return KEY_LEFT;
                }
            }
            return -1;
        }
        return ch;
    }
    return -1;
}

void signalHandler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        cout << "\nStopping..." << endl;
        running = false;
        reset_terminal();
        _exit(0);
    }
}

double normalizeAngle(double angle) {
    while (angle < 0) angle += 360.0;
    while (angle >= 360.0) angle -= 360.0;
    return angle;
}

class AngleAverager {
    vector<double> buffer;
    size_t max_size;
    size_t idx{0};
public:
    AngleAverager(size_t size = 15) : max_size(size) { buffer.reserve(size); }
    void add(double a) {
        if (buffer.size() < max_size) buffer.push_back(a);
        else buffer[idx % max_size] = a;
        idx++;
    }
    double get() {
        if (buffer.empty()) return 0.0;
        double sc = 0.0, ss = 0.0;
        for (double a : buffer) {
            double r = a * M_PI / 180.0;
            sc += cos(r);
            ss += sin(r);
        }
        double a = atan2(ss, sc) * 180.0 / M_PI;
        if (a < 0) a += 360.0;
        return a;
    }
};

void keyboardThread() {
    init_terminal();
    
    cout << "\nControls:" << endl;
    cout << "  Up/Down: Shift LED direction by 5°" << endl;
    cout << "  Left/Right: Shift LED direction by 1° (fine)" << endl;
    cout << "  S: Toggle shift ON/OFF" << endl;
    cout << "  R: Reset shift to 0" << endl;
    cout << "  Q: Quit" << endl;
    cout << "========================================" << endl;
    
    while (running) {
        int ch = get_key();
        
        if (ch == -1) {
            this_thread::sleep_for(chrono::milliseconds(50));
            continue;
        }
        
        if (ch == 'q' || ch == 'Q') {
            running = false;
            break;
        }
        
        if (ch == 's' || ch == 'S') {
            shift_active = !shift_active.load();
            if (shift_active.load()) {
                cout << "\nShift ENABLED - Offset: " << fixed << setprecision(1) << led_shift.load() << "°" << endl;
            } else {
                cout << "\nShift DISABLED" << endl;
            }
            continue;
        }
        
        if (ch == 'r' || ch == 'R') {
            led_shift.store(0.0);
            cout << "\nShift reset to 0°" << endl;
            continue;
        }
        
        double current_shift = led_shift.load();
        bool updated = false;
        
        if (ch == KEY_UP) {
            current_shift += 5.0;
            if (current_shift >= 360.0) current_shift -= 360.0;
            updated = true;
        }
        else if (ch == KEY_DOWN) {
            current_shift -= 5.0;
            if (current_shift < 0.0) current_shift += 360.0;
            updated = true;
        }
        else if (ch == KEY_LEFT) {
            current_shift -= 1.0;
            if (current_shift < 0.0) current_shift += 360.0;
            updated = true;
        }
        else if (ch == KEY_RIGHT) {
            current_shift += 1.0;
            if (current_shift >= 360.0) current_shift -= 360.0;
            updated = true;
        }
        
        if (updated) {
            led_shift.store(current_shift);
            shift_active.store(true);
            
            double display_angle = normalizeAngle(current_angle.load() + current_shift);
            cout << "\rShift: " << fixed << setprecision(1) << current_shift << "° | LED: " << display_angle << "°     " << flush;
            
            // Update LED immediately
            if (leds && leds->is_initialized()) {
                leds->show_direction(display_angle);
            }
        }
    }
    
    reset_terminal();
}

void ledThread() {
    double last_angle = -1.0;
    
    while (running) {
        double doa = current_angle.load();
        double shift = led_shift.load();
        bool active = shift_active.load();
        
        double display_angle;
        if (active) {
            display_angle = normalizeAngle(doa + shift);
        } else {
            display_angle = doa;
        }
        
        if (abs(display_angle - last_angle) > 0.5 && leds && leds->is_initialized()) {
            try { 
                leds->show_direction(display_angle); 
                last_angle = display_angle; 
            } catch (...) {}
        }
        
        this_thread::sleep_for(chrono::milliseconds(50));
    }
}

void audioThread() {
    unsigned int rate = 44100;
    unsigned int ch = 4;
    unsigned int bits = 16;
    float duration = 1.0f;
    
    AudioCapture capture(rate, ch, bits, duration);
    audio = &capture;
    
    if (!capture.initialize("plughw:1,0")) {
        cerr << "Audio init failed" << endl;
        return;
    }
    
    if (!capture.start()) {
        cerr << "Audio start failed" << endl;
        return;
    }
    
    int fs = rate;
    int nfft = 1024;
    double frame = 0.05;
    
    vector<vector<double>> pos(2, vector<double>(ch));
    vector<double> center = {0.0, 0.0};
    double radius = 0.04;
    
    for (unsigned int i = 0; i < ch; i++) {
        double a = i * 90.0 * M_PI / 180.0;
        pos[0][i] = radius * cos(a);
        pos[1][i] = radius * sin(a);
    }
    
    DOAEstimator estimator(fs, nfft);
    estimator.setMicPositions(pos);
    estimator.setArrayCenter(center);
    
    size_t bytes_per_sample = bits / 8;
    size_t samples_per_frame = rate * frame;
    size_t bytes_per_frame = samples_per_frame * ch * bytes_per_sample;
    
    vector<uint8_t> buffer(bytes_per_frame);
    vector<vector<double>> data(ch);
    for (auto& d : data) d.resize(samples_per_frame);
    
    cout << "DOA running" << endl;
    cout << "---------------------------------------------" << endl;
    
    int count = 0;
    vector<double> src = {1.0, 1.0};
    double angle = 0.0;
    bool first = true;
    AngleAverager avg(15);
    
    while (running) {
        if (capture.hasEnoughData(bytes_per_frame)) {
            size_t read = capture.getAudioChunk(buffer.data(), bytes_per_frame);
            if (read > 0) {
                int16_t* ptr = reinterpret_cast<int16_t*>(buffer.data());
                size_t samples = read / bytes_per_sample;
                size_t frames = samples / ch;
                if (frames > samples_per_frame) frames = samples_per_frame;
                if (frames == 0) continue;
                
                for (unsigned int c = 0; c < ch; c++) {
                    fill(data[c].begin(), data[c].end(), 0.0);
                }
                
                for (size_t i = 0; i < frames; i++) {
                    for (unsigned int c = 0; c < ch; c++) {
                        size_t idx = i * ch + c;
                        if (idx < samples) {
                            data[c][i] = ptr[idx] / 32768.0;
                        }
                    }
                }
                
                try {
                    auto result = estimator.estimateDOA(data, src);
                    double a = result.first;
                    if (a < 0) a += 360.0;
                    if (a >= 360.0) a -= 360.0;
                    
                    count++;
                    cout << "\r[" << count << "] DOA: " << fixed << setprecision(1) << a << "°     " << flush;
                    
                    avg.add(a);
                    double filtered = avg.get();
                    
                    if (first) { angle = filtered; first = false; }
                    else {
                        double diff = filtered - angle;
                        if (diff > 180.0) diff -= 360.0;
                        if (diff < -180.0) diff += 360.0;
                        angle += diff * 0.3;
                        if (angle < 0) angle += 360.0;
                        if (angle >= 360.0) angle -= 360.0;
                    }
                    
                    current_angle.store(angle);
                    
                } catch (...) {
                    count++;
                    cout << "\r[" << count << "] DOA: 0.0°     " << flush;
                }
            }
        } else {
            this_thread::sleep_for(chrono::milliseconds(10));
        }
    }
    
    capture.stop();
    cout << "\nAudio stopped" << endl;
}

int main() {
    cout << "========================================" << endl;
    cout << "  DOA System with LED Shift" << endl;
    cout << "========================================" << endl;
    
    cout << "\nLED init..." << endl;
    LEDController led(12);
    leds = &led;
    
    bool has_led = false;
    try {
        has_led = led.initialize();
        if (has_led) {
            cout << "LED ready" << endl;
            led.show_direction(0);
        } else {
            cerr << "LED init failed" << endl;
        }
    } catch (...) {
        cerr << "LED init exception" << endl;
    }
    
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    cout << "\nStarting..." << endl;
    
    thread kb_thread(keyboardThread);
    thread lt;
    if (has_led) lt = thread(ledThread);
    thread at(audioThread);
    
    while (running) {
        this_thread::sleep_for(chrono::milliseconds(100));
    }
    
    if (at.joinable()) at.join();
    if (has_led && lt.joinable()) lt.join();
    if (kb_thread.joinable()) kb_thread.join();
    
    if (leds && leds->is_initialized()) {
        try { leds->off(); leds->shutdown(); } catch (...) {}
    }
    
    reset_terminal();
    cout << "\nDone" << endl;
    return 0;
}