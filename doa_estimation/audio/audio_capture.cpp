#include "audio_capture.hpp"
#include <alsa/asoundlib.h>
#include <iostream>
#include <cstring>
#include <chrono>
#include "logger.h"

AudioCapture::AudioCapture(unsigned int sample_rate, 
                           unsigned int channels, 
                           unsigned int bits_per_sample,
                           float buffer_duration_seconds)
    : sample_rate_(sample_rate)
    , channels_(channels)
    , bits_per_sample_(bits_per_sample)
    , alsa_handle_(nullptr)
    , write_pos_(0)
    , read_pos_(0)
    , running_(false)
    , capture_active_(false) {
    
    bytes_per_sample_ = bits_per_sample_ / 8;
    buffer_size_ = calculateBufferSize(buffer_duration_seconds);
    ring_buffer_.resize(buffer_size_);
    LOG(2, "AudioCapture initialized", "  Sample rate: ", sample_rate_, " Hz", "  Channels: ", channels_, "  Bits per sample: ", bits_per_sample_, "  Buffer size: ", buffer_size_);
}

AudioCapture::~AudioCapture() {
    stop();
}

size_t AudioCapture::calculateBufferSize(float duration_seconds) {
    size_t bytes_per_second = sample_rate_ * channels_ * bytes_per_sample_;
    return static_cast<size_t>(bytes_per_second * duration_seconds);
}

bool AudioCapture::initialize(const std::string& alsa_device) {
    if (capture_active_) {
        last_error_ = "Already initialized";
        return false;
    }
    
    snd_pcm_t* handle = nullptr;
    int err;
    LOG(2, "Opening ALSA device: ", alsa_device);
    
    // Open PCM device for recording
    err = snd_pcm_open(&handle, alsa_device.c_str(), SND_PCM_STREAM_CAPTURE, 0);
    if (err < 0) {
        last_error_ = std::string("Cannot open audio device: ") + snd_strerror(err);
        return false;
    }
    
    // Configure PCM parameters
    snd_pcm_hw_params_t* params;
    snd_pcm_hw_params_alloca(&params);
    
    // Fill params with default values
    snd_pcm_hw_params_any(handle, params);
    
    // Set interleaved mode
    err = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (err < 0) {
        last_error_ = std::string("Cannot set access mode: ") + snd_strerror(err);
        snd_pcm_close(handle);
        return false;
    }
    
    // Set format
    err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
    if (err < 0) {
        last_error_ = std::string("Cannot set format S16_LE: ") + snd_strerror(err);
        snd_pcm_close(handle);
        return false;
    }
    
    // Set channels
    err = snd_pcm_hw_params_set_channels(handle, params, channels_);
    if (err < 0) {
        last_error_ = std::string("Cannot set ") + std::to_string(channels_) 
                    + " channels: " + snd_strerror(err);
        snd_pcm_close(handle);
        return false;
    }
    
    // Set sample rate
    unsigned int actual_rate = sample_rate_;
    err = snd_pcm_hw_params_set_rate_near(handle, params, &actual_rate, nullptr);
    if (err < 0) {
        last_error_ = std::string("Cannot set sample rate: ") + snd_strerror(err);
        snd_pcm_close(handle);
        return false;
    }
    
    if (actual_rate != sample_rate_) {
        LOG(1, "Note: Actual sample rate is ", actual_rate);
        sample_rate_ = actual_rate;
        buffer_size_ = calculateBufferSize(3.0f);
        ring_buffer_.resize(buffer_size_);
    }
    
    // Set period size (smaller for faster response)
    snd_pcm_uframes_t frames = 256;
    err = snd_pcm_hw_params_set_period_size_near(handle, params, &frames, nullptr);
    if (err < 0) {
        last_error_ = std::string("Cannot set period size: ") + snd_strerror(err);
        snd_pcm_close(handle);
        return false;
    }
    
    // Apply parameters
    err = snd_pcm_hw_params(handle, params);
    if (err < 0) {
        last_error_ = std::string("Cannot apply parameters: ") + snd_strerror(err);
        snd_pcm_close(handle);
        return false;
    }
    
    alsa_handle_ = handle;
    capture_active_ = true;
    LOG(2, "✓ Audio device initialized successfully:", "  Device: ", alsa_device, "  Channels: ", channels_, "  Sample rate: ", sample_rate_);
    return true;
}

bool AudioCapture::start() {
    if (!capture_active_) {
        last_error_ = "Not initialized";
        return false;
    }
    
    if (running_) {
        last_error_ = "Already running";
        return false;
    }
    
    // Prepare device
    if (snd_pcm_prepare(static_cast<snd_pcm_t*>(alsa_handle_)) < 0) {
        last_error_ = "Cannot prepare device";
        return false;
    }
    
    running_ = true;
    capture_thread_ = std::thread(&AudioCapture::captureThread, this);
    bind_thread_to_core(capture_thread_, 7);
    LOG(2, "Audio capture started");
    return true;
}

void AudioCapture::stop() {
    if (running_) {
        LOG(2, "Stopping audio capture...");
        running_ = false;
        if (capture_thread_.joinable()) {
            capture_thread_.join();
        }
    }
    
    if (alsa_handle_) {
        snd_pcm_close(static_cast<snd_pcm_t*>(alsa_handle_));
        alsa_handle_ = nullptr;
    }
    
    capture_active_ = false;
    LOG(2, "Audio capture stopped");
}

bool AudioCapture::isRunning() const {
    return running_;
}

void AudioCapture::captureThread() {
    snd_pcm_t* handle = static_cast<snd_pcm_t*>(alsa_handle_);
    
    // Calculate frame size: channels * bytes_per_sample
    size_t frame_size = channels_ * bytes_per_sample_;
    const size_t frames_per_read = 256;  // Small for faster filling
    const size_t bytes_per_read = frames_per_read * frame_size;
    
    std::vector<uint8_t> read_buffer(bytes_per_read);
    LOG(3, "Capture thread started. Frame size: ", frame_size, " bytes, Buffer per read: ", bytes_per_read);
    
    size_t total_captured = 0;
    
    while (running_) {
        snd_pcm_sframes_t frames = snd_pcm_readi(handle, read_buffer.data(), frames_per_read);
        
        if (frames > 0) {
            size_t bytes = frames * frame_size;
            // total_captured += bytes;
            
            // Debug: Show first capture
            // if (total_captured == bytes) {
            //     std::cout << "First audio captured! " << bytes << " bytes\n";
            //     // Show first few samples
            //     int16_t* samples = reinterpret_cast<int16_t*>(read_buffer.data());
            //     std::cout << "First 4 samples (L,R): ";
            //     for (int i = 0; i < 8 && i < frames * channels_; i += 2) {
            //         std::cout << "(" << samples[i] << "," << samples[i+1] << ") ";
            //     }
            //     std::cout << "\n";
            // }
            
            {
                std::lock_guard<std::mutex> lock(buffer_mutex_);
                size_t available_space = buffer_size_ - getAvailableBytes();
                if (available_space < bytes) {
                    // Buffer overrun - skip some old data
                    LOG(2, "Audio buffer overrun! Skipping ", bytes - available_space, " bytes");
                    // Advance read position to make space
                    read_pos_.store((read_pos_.load() + (bytes - available_space)) % buffer_size_);
                }
                writeToBuffer(read_buffer.data(), bytes);
            }
            
            data_available_.notify_one();
            
        } else if (frames == -EPIPE) {
            LOG(2, "Buffer overrun, recovering...");
            snd_pcm_prepare(handle);
        } else if (frames == -EAGAIN) {
            // No data available, sleep a bit
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        } else if (frames < 0) {
            LOG(1, "ALSA read error: " , snd_strerror(frames));
            break;
        }
    }
    LOG(3, "Capture thread ended. Total bytes captured: ", total_captured);
}

void AudioCapture::writeToBuffer(const uint8_t* data, size_t bytes) {
    size_t current_write_pos = write_pos_.load();
    
    for (size_t i = 0; i < bytes; i++) {
        ring_buffer_[current_write_pos] = data[i];
        current_write_pos = (current_write_pos + 1) % buffer_size_;
    }
    
    write_pos_.store(current_write_pos);
}

size_t AudioCapture::getAudioChunk(uint8_t* buffer, size_t max_bytes) {
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    
    size_t wp = write_pos_.load();
    size_t rp = read_pos_.load();
    size_t available;
    
    // Calculate available bytes
    if (wp >= rp) {
        available = wp - rp;
    } else {
        available = buffer_size_ - rp + wp;
    }
    
    // DEBUG: Log buffer usage
    static size_t last_log = 0;
    if (available > buffer_size_ * 0.8) {  // 80% full
        LOG(1, "WARNING: Audio buffer is ", (available * 100) / buffer_size_, "% full!");
    }

    if (available == 0) {
        return 0;
    }
    
    size_t to_read = (available < max_bytes) ? available : max_bytes;
    
    // Read from ring buffer
    if (rp + to_read <= buffer_size_) {
        // No wrap-around
        memcpy(buffer, &ring_buffer_[rp], to_read);
        read_pos_.store(rp + to_read);
    } else {
        // Wrap-around
        size_t first_part = buffer_size_ - rp;
        memcpy(buffer, &ring_buffer_[rp], first_part);
        memcpy(buffer + first_part, &ring_buffer_[0], to_read - first_part);
        read_pos_.store(to_read - first_part);
    }
    
    return to_read;
}

size_t AudioCapture::getAudioChunk(uint8_t* buffer, size_t max_bytes, int timeout_ms) {
    // Simple implementation - poll until data is available
    auto start = std::chrono::steady_clock::now();
    
    while (true) {
        size_t bytes = getAudioChunk(buffer, max_bytes);
        if (bytes > 0) {
            return bytes;
        }
        
        // Check timeout
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
        if (elapsed >= timeout_ms) {
            return 0;
        }
        
        // Sleep a bit before trying again
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

bool AudioCapture::hasEnoughData(size_t required_bytes) const {
    return getAvailableBytes() >= required_bytes;
}

size_t AudioCapture::getCurrentPosition() const {
    return write_pos_.load();
}

size_t AudioCapture::getAvailableBytes() const {
    size_t write_pos = write_pos_.load();
    size_t read_pos = read_pos_.load();
    
    if (write_pos >= read_pos) {
        return write_pos - read_pos;
    } else {
        return buffer_size_ - read_pos + write_pos;
    }
}

void AudioCapture::bind_thread_to_core(std::thread& thread, int core_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);
    
    pthread_t native_handle = thread.native_handle();
    if (pthread_setaffinity_np(native_handle, sizeof(cpu_set_t), &cpuset) != 0) {
        perror("Failed to set thread affinity");
    }
}