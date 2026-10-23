CXX      := g++
WARN     := -pedantic -Wall -fdiagnostics-color=always
DEPFLAGS := -MMD -MP

CXXFLAGS_REL := -O3 -DNDEBUG -fopenmp $(WARN) $(DEPFLAGS)
CXXFLAGS_DBG := -g -O0 -fopenmp -Werror $(WARN) $(DEPFLAGS) \
                -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC
LDFLAGS  := -fopenmp

SRCS   := Main.cpp PuppetShape.cpp Puppet.cpp AlgMani.cpp Table.cpp Solver.cpp BackwardGenerator.cpp
TARGET := Main

BUILD_REL := build/release
BUILD_DBG := build/debug
OBJS_REL  := $(SRCS:%.cpp=$(BUILD_REL)/%.o)
OBJS_DBG  := $(SRCS:%.cpp=$(BUILD_DBG)/%.o)
DEPS      := $(OBJS_REL:.o=.d) $(OBJS_DBG:.o=.d)

.PHONY: all debug run saveOutput clean

# default: optimized build
all: $(TARGET)

$(TARGET): $(OBJS_REL)
	$(CXX) $(OBJS_REL) -o $@ $(LDFLAGS)

$(BUILD_REL)/%.o: %.cpp
	@mkdir -p $(BUILD_REL)
	$(CXX) $(CXXFLAGS_REL) -c $< -o $@

# debug build for finding bugs: make debug
debug: $(TARGET)_debug

$(TARGET)_debug: $(OBJS_DBG)
	$(CXX) $(OBJS_DBG) -o $@ $(LDFLAGS)

$(BUILD_DBG)/%.o: %.cpp
	@mkdir -p $(BUILD_DBG)
	$(CXX) $(CXXFLAGS_DBG) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

saveOutput: $(TARGET)
	./$(TARGET) > output.txt

clean:
	rm -rf build $(TARGET) $(TARGET)_debug

-include $(DEPS)