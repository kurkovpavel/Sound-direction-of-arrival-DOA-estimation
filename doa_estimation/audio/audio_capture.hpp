#ifndef AUDIO_CAPTURE_HPP
#define AUDIO_CAPTURE_HPP

#include <vector>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <cstdint>
#include <string>

class AudioCapture {
public:
    // Constructor - ES8388 needs stereo (2 channels), not mono (1)
    AudioCapture(unsigned int sample_rate = 44100, 
                 unsigned int channels = 4,  // CHANGED: Default to 2 (stereo)
                 unsigned int bits_per_sample = 16,
                 float buffer_duration_seconds = 0.5f);
    
    // Destructor
    ~AudioCapture();
    
    // Initialize audio capture with specified ALSA device
    bool initialize(const std::string& alsa_device = "hw:1,0");  // CHANGED: Default to ES8388
    
    // Start audio capture thread
    bool start();
    
    // Stop audio capture thread
    void stop();
    
    // Check if capture is running
    bool isRunning() const;
    
    // Get audio data from ring buffer (non-blocking)
    size_t getAudioChunk(uint8_t* buffer, size_t max_bytes);
    
    // Get audio data from ring buffer (with timeout)
    size_t getAudioChunk(uint8_t* buffer, size_t max_bytes, int timeout_ms);
    
    // Check if there's enough new audio data available
    bool hasEnoughData(size_t required_bytes) const;
    
    // Get current write position (for external sync)
    size_t getCurrentPosition() const;
    
    // Get available data size (in bytes)
    size_t getAvailableBytes() const;
    
    // Get audio parameters
    unsigned int getSampleRate() const { return sample_rate_; }
    unsigned int getChannels() const { return channels_; }
    unsigned int getBitsPerSample() const { return bits_per_sample_; }
    size_t getBufferSizeBytes() const { return ring_buffer_.size(); }
    
    // Get ALSA error message if initialization failed
    std::string getLastError() const { return last_error_; }
    
    // CHANGED: Add method to try different channel counts
    bool tryInitialize(const std::string& alsa_device = "hw:5,0");
    
private:
    // Audio capture thread function
    void captureThread();
    void bind_thread_to_core(std::thread& thread, int core_id);
    // Update ring buffer with new audio data
    void writeToBuffer(const uint8_t* data, size_t bytes);
    
    // Calculate buffer size based on audio parameters
    size_t calculateBufferSize(float duration_seconds);
    
    // Audio parameters
    unsigned int sample_rate_;
    unsigned int channels_;
    unsigned int bits_per_sample_;
    size_t bytes_per_sample_;
    
    // ALSA device handle
    void* alsa_handle_;
    
    // Ring buffer
    std::vector<uint8_t> ring_buffer_;
    std::atomic<size_t> write_pos_;
    std::atomic<size_t> read_pos_;
    size_t buffer_size_;
    
    // Thread control
    std::atomic<bool> running_;
    std::atomic<bool> capture_active_;
    std::thread capture_thread_;
    
    // Synchronization
    mutable std::mutex buffer_mutex_;
    std::condition_variable data_available_;
    
    // Error handling
    std::string last_error_;
    
    // Prevent copying
    AudioCapture(const AudioCapture&) = delete;
    AudioCapture& operator=(const AudioCapture&) = delete;
};

#endif // AUDIO_CAPTURE_HPP