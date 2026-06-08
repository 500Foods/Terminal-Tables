# Makefile for tables C implementation - Optimized for smallest executable
CC = gcc
# Aggressive optimization flags for smallest binary
CFLAGS = -Wall -Wextra -Os -ffunction-sections -fdata-sections -fno-asynchronous-unwind-tables -fno-unwind-tables -s $(shell pkg-config --cflags jansson)
LDFLAGS = -Wl,--gc-sections -Wl,--strip-all $(shell pkg-config --libs jansson)
TARGET = tables
SOURCES = *.c

# Default target to build optimized and compressed executable
all: clean $(TARGET)

# Build the optimized executable and compress in-place
$(TARGET):
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET) $(LDFLAGS)
	strip --strip-all $(TARGET)
	@if command -v upx >/dev/null 2>&1; then \
		echo "Compressing $(TARGET) with UPX..."; \
		upx --best --lzma $(TARGET); \
		echo "Final compressed executable: $(TARGET)"; \
		ls -lh $(TARGET); \
	else \
		echo "UPX not found, executable optimized but not compressed"; \
		echo "Final executable: $(TARGET)"; \
		ls -lh $(TARGET); \
	fi

# Build uncompressed optimized version only
uncompressed: clean
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET) $(LDFLAGS)
	strip --strip-all $(TARGET)

# Clean build artifacts
clean:
	rm -f $(TARGET)

# Install UPX if not present (Ubuntu/Debian)
install-upx:
	@if ! command -v upx >/dev/null 2>&1; then \
		echo "Installing UPX..."; \
		sudo apt-get update && sudo apt-get install -y upx-ucl; \
	else \
		echo "UPX is already installed"; \
	fi

# Phony targets
.PHONY: all clean uncompressed install-upx
