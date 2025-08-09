# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic -O2
DEBUG_FLAGS = -g -DDEBUG
TARGET = json_to_sexpr

# Directories
SRC_DIR := src
INC_DIR := include
BUILD_DIR := build
TEST_DIR := tests
DATA_DIR := $(TEST_DIR)/data

# Sources / Objects / Headers
SOURCES := $(wildcard $(SRC_DIR)/*.c)
OBJECTS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))
HEADERS := $(wildcard $(INC_DIR)/*.h)

# Ensure build directory exists
$(shell mkdir -p $(BUILD_DIR))

# Default target
all: $(TARGET)

# Link
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -I$(INC_DIR) -o $@ $^

# Compile objects
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS)
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

# Debug build
debug: CFLAGS += $(DEBUG_FLAGS)
debug: clean all

# Run basic test using sample test file
test: $(TARGET)
	@echo "Running basic tests..."
	@echo '{"name": "test", "value": 42}' | ./$(TARGET)
	@echo ""
	@echo "Running data set tests (if present)..."
	@if [ -f $(DATA_DIR)/test.json ]; then ./$(TARGET) $(DATA_DIR)/test.json > /dev/null && echo "test.json OK"; fi
	@if [ -f $(DATA_DIR)/sample.json ]; then ./$(TARGET) $(DATA_DIR)/sample.json > /dev/null && echo "sample.json OK"; fi
	@echo "Test completed."

# Generate sample JSON inside data dir
sample:
	@mkdir -p $(DATA_DIR)
	@echo 'Creating $(DATA_DIR)/sample.json...'
	@echo '{"receipt": "Oz-Ware Purchase Invoice", "date": "2012-08-06", "customer": {"first_name": "Dorothy", "family_name": "Gale"}, "items": [{"part_no": "A4786", "descrip": "Water Bucket (Filled)", "price": 1.47, "quantity": 4}, {"part_no": "E1628", "descrip": "High Heeled \"Ruby\" Slippers", "size": 8, "price": 133.7, "quantity": 1}]}' > $(DATA_DIR)/sample.json
	@echo 'Sample JSON file created.'

# Install (copy to /usr/local/bin)
install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/

# Uninstall
uninstall:
	sudo rm -f /usr/local/bin/$(TARGET)

# Clean
clean:
	rm -f $(TARGET)
	rm -rf $(BUILD_DIR)

# Lint placeholder (extend with clang-tidy / cppcheck if desired)
lint:
	@echo "(Add clang-tidy / cppcheck invocation here)"

# Formatting placeholder (e.g., clang-format)
format:
	@echo "(Add clang-format invocation here)"

# Help
help:
	@echo "Available targets:"
	@echo "  all       - Build the program (default)"
	@echo "  debug     - Debug build (adds -g)"
	@echo "  clean     - Remove build artifacts"
	@echo "  install   - Install binary to /usr/local/bin" \
		"(may require sudo)"
	@echo "  uninstall - Remove installed binary"
	@echo "  test      - Run basic tests"
	@echo "  sample    - Generate sample JSON test file"
	@echo "  lint      - Run static analysis (placeholder)"
	@echo "  format    - Run code formatter (placeholder)"
	@echo "  help      - Show this help"

.PHONY: all debug clean install uninstall test sample lint format help
