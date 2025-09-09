# Compiler and flags
CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++17

# Target executable
TARGET := main

# Source files
SRC := main.cpp
OBJ := $(SRC:.cpp=.o)

# Build target
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile source files to object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJ) $(TARGET)
