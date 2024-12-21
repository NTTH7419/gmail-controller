CXX = g++
CXXFLAGS = -std=c++17 -DUNICODE -D_UNICODE -I./lib/FTXUI/include -I./lib/curl_8.11/include -I./lib/json/include
LDFLAGS = -L./lib/curl_8.11/lib -lpsapi -lws2_32 -lcurl

# List all source files explicitly
SRCS = $(wildcard *.cpp) $(wildcard gmailapi/*.cpp) \
       $(wildcard lib/FTXUI/src/ftxui/component/*.cpp) \
       $(wildcard lib/FTXUI/src/ftxui/dom/*.cpp) \
       $(wildcard lib/FTXUI/src/ftxui/screen/*.cpp)

# Specify the directories for the output executable and object files
BIN_DIR = ../../release/client
BUILD_DIR = build

# Define the target executable name
TARGET = client

# Generate object files from source files
OBJS = $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SRCS))

# Default target
all: $(BIN_DIR)/$(TARGET)

# Link object files to create the final executable
$(BIN_DIR)/$(TARGET): $(OBJS)
ifeq ($(OS),Windows_NT)
	@if not exist "$(BIN_DIR)" mkdir "$(BIN_DIR)"
else
	mkdir -p $(BIN_DIR)
endif
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Compile source files into object files
$(BUILD_DIR)/%.o: %.cpp
ifeq ($(OS),Windows_NT)
	@if not exist "$(dir $@)" mkdir "$(dir $@)"
else
	mkdir -p $(dir $@)
endif
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Cleanup
clean:
ifeq ($(OS),Windows_NT)
	del /Q $(BIN_DIR)\$(TARGET).exe $(BUILD_DIR)\*.o
else
	rm -f $(BIN_DIR)/$(TARGET) $(OBJS)
	rm -rf $(BUILD_DIR)
endif
