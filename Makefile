SRC_DIR = src
TEST_DIR = tests
BUILD_DIR = build
DEPS_DIR = deps
INC_DIR = include

CC = clang
CFLAGS = -std=c99 -Wall -Wextra -pedantic -g -I$(INC_DIR) -Ideps -Ideps/unity -Ideps/argparse
LDFLAGS = /usr/lib/x86_64-linux-gnu/liblexbor.so

# Sources
SRCS = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/common/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# Dep Sources (argparse needs compilation)
DEP_SRCS = $(DEPS_DIR)/argparse/argparse.c
DEP_OBJS = $(DEP_SRCS:$(DEPS_DIR)/%.c=$(BUILD_DIR)/%.o)

# Test Sources
TEST_SRCS = $(wildcard $(TEST_DIR)/*.c)
TEST_OBJS = $(TEST_SRCS:$(TEST_DIR)/%.c=$(BUILD_DIR)/%.o)
TEST_RUNNER = $(BUILD_DIR)/test_runner

TARGET = $(BUILD_DIR)/cssoptim

# Phony Targets
.PHONY: all clean test fmt lint san

all: $(TARGET)

$(TARGET): $(OBJS) $(DEP_OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(DEPS_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Tests
# Note: Helper to compile unity.c only once or link it
UNITY_SRC = $(DEPS_DIR)/unity/unity.c
UNITY_OBJ = $(BUILD_DIR)/unity.o

$(UNITY_OBJ): $(UNITY_SRC)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(TEST_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TEST_RUNNER)
	./$(TEST_RUNNER)

$(TEST_RUNNER): $(TEST_OBJS) $(UNITY_OBJ) $(filter-out $(BUILD_DIR)/main.o, $(OBJS)) $(DEP_OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Sanitizers
san: CFLAGS += $(SANITIZERS)
san: LDFLAGS += $(SANITIZERS)
san: clean all test

# Tools
clean:
	rm -rf $(BUILD_DIR)

fmt:
	clang-format -i $(SRCS) $(TEST_SRCS) $(INC_DIR)/cssoptim/*.h

lint:
	clang-tidy $(SRCS) $(TEST_SRCS) -- $(CFLAGS)
