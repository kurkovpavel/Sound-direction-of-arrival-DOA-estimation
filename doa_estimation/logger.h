// Logger.h
#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <filesystem>
#include <sstream>
#include <vector>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <map>
#include <variant>
#include <type_traits>
#include "display/display.h" 

// Forward declaration for helper functions
namespace LoggerInternals {
    template<typename T>
    std::string argumentToString(const T& value);
    
    template<typename T>
    std::string getTypeName();
}

// Forward declaration for OLEDDisplay (no need to include header)
class OLEDDisplay;

class FileLogger {
    static inline std::unordered_map<std::string, int> fileVerboseLevels;
    static inline std::ofstream hexFile;
    static inline std::string hexFilename = "data_log.txt";
    static inline bool hexFileInitialized = false;
    
    // New members for readable log
    static inline std::ofstream readableLogFile;
    static inline std::string readableLogFilename = "readable_log.log";
    static inline bool readableLogFileInitialized = false;
    
    // New members for SHOTLOG functionality
    static inline std::ofstream shotLogFile;
    static inline std::string shotLogFilename = "shot_log.log";
    static inline bool shotLogFileInitialized = false;
    static inline bool shotLogActive = false;
    static inline std::vector<std::string> shotLogBuffer;
    static inline std::chrono::system_clock::time_point shotLogStartTime;
    
    // Display members
    static inline OLEDDisplay* display = nullptr;
    static inline bool displayInitialized = false;
    
public:
    static void init(const std::string& configFile = "logging.cfg") {
        //std::cout << "[Logger] Initializing from: " << configFile << std::endl;
        
        std::ifstream file(configFile);
        if (!file.is_open()) {
            std::cerr << "[Logger] Error: Could not open config file: " << configFile << std::endl;
            return;
        }

        std::string line;
        while (std::getline(file, line)) {
            size_t spacePos = line.find_last_of(' ');
            if (spacePos != std::string::npos) {
                std::string filename = line.substr(0, spacePos);
                int level = std::stoi(line.substr(spacePos + 1));
                fileVerboseLevels[filename] = level;
                //std::cout << "[Logger] Registered: " << filename << " at level " << level << std::endl;
            }
        }
        
        // Initialize display
        initDisplay();
    }
    
    static void initDisplay(const std::string& i2c_device = "/dev/i2c-1", uint8_t address = 0x3C) {
        if (displayInitialized) {
            return;
        }
        
        try {
            display = new OLEDDisplay(i2c_device, address);
            if (display && display->init()) {
                displayInitialized = true;
                display->addLine("Logger initialized");
                std::cout << "[Logger] Display initialized successfully" << std::endl;
            } else {
                std::cerr << "[Logger] Failed to initialize display" << std::endl;
                delete display;
                display = nullptr;
            }
        } catch (const std::exception& e) {
            std::cerr << "[Logger] Exception initializing display: " << e.what() << std::endl;
            display = nullptr;
        }
    }
    
    static void displayAddLine(const std::string& text) {
        if (displayInitialized && display) {
            display->addLine(text);
        }
    }
    
    static void displayEditLine(const std::string& text) {
        if (displayInitialized && display) {
            display->editLine(text);
        }
    }
    
    static void shutdownDisplay() {
        if (display) {
            delete display;
            display = nullptr;
        }
        displayInitialized = false;
    }
    
    static bool isEnabled(const std::string& filename, int requiredLevel) {
        std::filesystem::path p(filename);
        std::string basename = p.filename().string();
        
        // Check full path first
        auto it = fileVerboseLevels.find(filename);
        if (it == fileVerboseLevels.end()) {
            // Check basename if full path not found
            it = fileVerboseLevels.find(basename);
        }
        
        if (it != fileVerboseLevels.end()) {
            return it->second >= requiredLevel;
        }
        
        return false;
    }
    
    static void saveHexData(const std::string& direction, const std::vector<uint8_t>& data) {
        if (!hexFileInitialized) {
            hexFile.open(hexFilename, std::ios::app);
            if (!hexFile.is_open()) {
                std::cerr << "[Logger] Error: Could not open hex data file: " << hexFilename << std::endl;
                return;
            }
            hexFileInitialized = true;
        }
        
        // Get current time with milliseconds
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        // Format timestamp
        std::tm tm = *std::localtime(&time_t);
        hexFile << "[" << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << "." 
                << std::setw(3) << std::setfill('0') << ms.count() << "] ";
        
        hexFile << direction << ": ";
        for (size_t i = 0; i < data.size(); ++i) {
            hexFile << "0x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') 
                   << static_cast<int>(data[i]);
            if (i < data.size() - 1) {
                hexFile << " ";
            }
        }
        hexFile << std::endl;
        hexFile.flush();
    }

    static void setHexFilename(const std::string& filename) {
        if (hexFile.is_open()) {
            hexFile.close();
        }
        hexFilename = filename;
        hexFileInitialized = false;
    }

    static void closeHexFile() {
        if (hexFile.is_open()) {
            hexFile.close();
        }
        hexFileInitialized = false;
    }
    
    static void writeReadableLog(const std::string& message) {
        if (!readableLogFileInitialized) {
            readableLogFile.open(readableLogFilename, std::ios::app);
            if (!readableLogFile.is_open()) {
                std::cerr << "[Logger] Error: Could not open readable log file: " << readableLogFilename << std::endl;
                return;
            }
            readableLogFileInitialized = true;
        }
        
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        std::tm tm = *std::localtime(&time_t);
        readableLogFile << "[" << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << "." 
                       << std::setw(3) << std::setfill('0') << ms.count() << "] ";
        
        readableLogFile << message << std::endl;
        readableLogFile.flush();
    }
    
    static void setReadableLogFilename(const std::string& filename) {
        if (readableLogFile.is_open()) {
            readableLogFile.close();
        }
        readableLogFilename = filename;
        readableLogFileInitialized = false;
    }
    
    static void closeReadableLogFile() {
        if (readableLogFile.is_open()) {
            readableLogFile.close();
        }
        readableLogFileInitialized = false;
    }
    
    // SHOTLOG FUNCTIONS
    static void startShotLog() {
        shotLogStartTime = std::chrono::system_clock::now();
        shotLogActive = true;
        shotLogBuffer.clear();
    }

    template<typename... Args>
    static void continueShotLog(Args&&... args) {
        std::ostringstream stream;
        ((stream << LoggerInternals::argumentToString(args) << " "), ...);
        
        std::string entry = stream.str();
        if (!entry.empty() && entry.back() == ' ') {
            entry.pop_back();
        }
        
        shotLogBuffer.push_back(entry);
    }

    template<typename... Args>
    static void stopShotLog(Args&&... args) {
        if constexpr (sizeof...(args) > 0) {
            std::ostringstream stream;
            ((stream << LoggerInternals::argumentToString(args) << " "), ...);
            
            std::string entry = stream.str();
            if (!entry.empty() && entry.back() == ' ') {
                entry.pop_back();
            }
            
            shotLogBuffer.push_back(entry);
        }
        
        flushShotLog();
        shotLogActive = false;
        shotLogBuffer.clear();
    }
    
    static void flushShotLog() {
        if (!shotLogFileInitialized) {
            shotLogFile.open(shotLogFilename, std::ios::app);
            shotLogFileInitialized = true;
        }
        
        auto time_t = std::chrono::system_clock::to_time_t(shotLogStartTime);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            shotLogStartTime.time_since_epoch()) % 1000;
        std::tm tm = *std::localtime(&time_t);
        
        shotLogFile << "[" << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << "." 
                   << std::setw(3) << std::setfill('0') << ms.count() << "]" << std::endl;
        
        for (const auto& entry : shotLogBuffer) {
            shotLogFile << entry << std::endl;
        }
        
        auto now = std::chrono::system_clock::now();
        time_t = std::chrono::system_clock::to_time_t(now);
        ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        tm = *std::localtime(&time_t);
        
        shotLogFile << "[" << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << "." 
                   << std::setw(3) << std::setfill('0') << ms.count() << "]" << std::endl;
        
        shotLogFile << "---" << std::endl << std::endl;
        shotLogFile.flush();
    }
    
    static void setShotLogFilename(const std::string& filename) {
        if (shotLogFile.is_open()) {
            shotLogFile.close();
        }
        shotLogFilename = filename;
        shotLogFileInitialized = false;
    }
    
    static void closeShotLogFile() {
        if (shotLogFile.is_open()) {
            shotLogFile.close();
        }
        shotLogFileInitialized = false;
    }
    
    static bool isShotLogActive() {
        return shotLogActive;
    }
    
    static void cancelShotLog() {
        shotLogActive = false;
        shotLogBuffer.clear();
    }
    
    static int getLogLevel(const std::string& filename) {
        std::filesystem::path p(filename);
        std::string basename = p.filename().string();
        
        auto it = fileVerboseLevels.find(filename);
        if (it == fileVerboseLevels.end()) {
            it = fileVerboseLevels.find(basename);
        }
        
        if (it != fileVerboseLevels.end()) {
            return it->second;
        }
        
        return -1;
    }
};

// Helper namespace for type conversions
namespace LoggerInternals {
    template<typename T>
    std::string argumentToString(const T& value) {
        if constexpr (std::is_same_v<T, std::string>) {
            return value;
        } else if constexpr (std::is_same_v<T, const char*>) {
            return std::string(value);
        } else if constexpr (std::is_same_v<T, char>) {
            return std::string(1, value);
        } else if constexpr (std::is_integral_v<T>) {
            return std::to_string(value);
        } else if constexpr (std::is_floating_point_v<T>) {
            std::ostringstream stream;
            stream << std::fixed << std::setprecision(6) << value;
            return stream.str();
        } else if constexpr (std::is_same_v<T, bool>) {
            return value ? "true" : "false";
        } else {
            std::ostringstream stream;
            stream << value;
            return stream.str();
        }
    }
}

template<typename... Args>
std::string buildLogMessage(Args&&... args) {
    std::ostringstream stream;
    (stream << ... << std::forward<Args>(args));
    return stream.str();
}

// LOG macro with display support for level 1
#define LOG(level, ...)                                                   \
    do {                                                                  \
        std::string fullPath(__FILE__);                                   \
        std::filesystem::path p(fullPath);                                \
        std::string basename = p.filename().string();                     \
        if (FileLogger::isEnabled(fullPath, level)) {                     \
            std::string message = buildLogMessage(__VA_ARGS__);           \
            std::cout << "[" << basename << " LEVEL" << level << "]: "    \
                      << message << std::endl;                            \
            if (level == 1) {                                             \
                FileLogger::displayAddLine(message);                      \
            }                                                             \
        }                                                                 \
    } while (0)

// LOG_EDIT macro for editing display line
#define LOG_EDIT(level, ...)                                              \
    do {                                                                  \
        std::string fullPath(__FILE__);                                   \
        std::filesystem::path p(fullPath);                                \
        std::string basename = p.filename().string();                     \
        if (FileLogger::isEnabled(fullPath, level)) {                     \
            std::string message = buildLogMessage(__VA_ARGS__);           \
            std::cout << "[" << basename << " LEVEL" << level << "]: "    \
                      << message << std::endl;                            \
            if (level == 1) {                                             \
                FileLogger::displayEditLine(message);                     \
            }                                                             \
        }                                                                 \
    } while (0)

#define SAVELOG(direction, data) FileLogger::saveHexData(direction, data)
#define READABLE_LOG(...) FileLogger::writeReadableLog(buildLogMessage(__VA_ARGS__))

#define SHOTLOG(action, ...)                                      \
    do {                                                          \
        if (std::string(action) == "START") {                     \
            FileLogger::startShotLog();                           \
            FileLogger::continueShotLog(__VA_ARGS__);             \
        } else if (std::string(action) == "CONTINUE") {           \
            FileLogger::continueShotLog(__VA_ARGS__);             \
        } else if (std::string(action) == "STOP") {               \
            FileLogger::stopShotLog(__VA_ARGS__);                 \
        } else {                                                  \
            std::cerr << "[Logger] Error: Unknown SHOTLOG action: " \
                      << action << std::endl;                     \
        }                                                         \
    } while (0)

#define GET_LOG_LEVEL() FileLogger::getLogLevel(__FILE__)