DISABLED_WARNINGS = -Wno-format-security

CXX = g++
CXXFLAGS := -Wall -Wextra -std=c++17 $(DISABLED_WARNINGS)

BUILD = build
INCLUDE = include
SRC = src
OBJS := $(BUILD)/objs

SOURCES := $(wildcard $(SRC)/*.cc)
OBJECTS := $(patsubst $(SRC)/%.cc, $(OBJS)/%.o, $(SOURCES))

BIN := $(BUILD)/scriptlang

all: create-build-folder $(BIN)

debug: CXXFLAGS += -ggdb -DDEBUG
debug: all

$(BIN): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(OBJS)/%.o: $(SRC)/%.cc $(INCLUDE)/%.h
	$(CXX) -c $(CXXFLAGS) $< -o $@

$(OBJS)/%.o: $(SRC)/%.cc
	$(CXX) -c $(CXXFLAGS) $< -o $@

.PHONY: clean create-build-folder add debug

add:
	@touch $(SRC)/$(file).cc
	@touch $(INCLUDE)/$(file).h

clean:
	@rm -rf $(BUILD)

create-build-folder:
	@mkdir -p $(BUILD)
	@mkdir -p $(OBJS)
