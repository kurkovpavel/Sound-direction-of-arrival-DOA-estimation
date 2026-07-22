CXX = g++
CXXFLAGS = -std=c++17 -O3 -Wall -pthread -I.
LDFLAGS = -lm -lasound -lpthread -lgpiod 
TARGET = doa_estimation
SOURCES = main.cpp audio/doa.cpp audio/audio_capture.cpp display/display.cpp led/led.cpp
HEADERS = audio/doa.hpp audio/audio_capture.hpp display/display.h led/led.hpp
OBJECTS = $(SOURCES:.cpp=.o)

all: $(TARGET)

# Link object files
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Compile each source file to object
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJECTS)
	rm -f audio/*.wav

run: $(TARGET)
	./$(TARGET)

# For maximum parallel compilation
parallel: 
	$(MAKE) -j$(nproc)

.PHONY: all clean run parallel