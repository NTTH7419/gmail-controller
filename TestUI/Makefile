CXX = g++
CXXFLAGS = -std=c++17 -DUNICODE -D_UNICODE -I./FTXUI/include
LDFLAGS = 

# List all source files explicitly
SRCS = $(wildcard *.cpp) \
       $(wildcard FTXUI/src/ftxui/component/*.cpp) \
       $(wildcard FTXUI/src/ftxui/dom/*.cpp) \
       $(wildcard FTXUI/src/ftxui/screen/*.cpp)

# Specify the directories for the output executable and object files
BIN_DIR = bin
BUILD_DIR = build

# Generate object files from source files
OBJS = $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SRCS))

# Default target
all: $(BIN_DIR)/main

# Link object files to create the final executable
$(BIN_DIR)/main: $(OBJS)
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
	del /Q $(BIN_DIR)\main.exe $(BUILD_DIR)\*.o
else
	rm -f $(BIN_DIR)/main $(OBJS)
	rm -rf $(BUILD_DIR)
endif
