CC := gcc
SRCDIR := src
INCDIR := include
BUILDDIR := build
SRCEXT := c
CFLAGS := -std=c11 -Wall
LIBS := -lpthread
INC := -Iinclude

TARGET_BIN_DIR := bin
TARGET_LIB_DIR := lib
TARGET_LIB = libfiber
TARGET_BIN = fiber_example

$(TARGET_BIN_DIR)/$(TARGET_BIN): $(BUILDDIR)/$(TARGET_BIN).o $(TARGET_LIB_DIR)/$(TARGET_LIB).a
	@echo "==> Compiling binary..."
	@mkdir -p $(TARGET_BIN_DIR) 
	@gcc $(CFLAGS) $(LIBS) $(INC) $^ -o $@

$(BUILDDIR)/$(TARGET_BIN).o: $(SRCDIR)/fiber_example.c
	@echo "==> Compiling binary object..."
	@mkdir -p $(BUILDDIR) 
	@gcc $(CFLAGS) $(LIBS) $(INC) -c $< -o $@

$(TARGET_LIB_DIR)/$(TARGET_LIB).a: $(BUILDDIR)/$(TARGET_LIB).o
	@echo "==> Packing library..."
	@mkdir -p $(TARGET_LIB_DIR) 
	@ar rcs $@ $^

$(BUILDDIR)/$(TARGET_LIB).o: $(SRCDIR)/fiber.c $(INCDIR)/fiber.h
	@echo "==> Compiling library object..."
	@mkdir -p $(BUILDDIR) 
	@gcc $(CFLAGS) $(LIBS) $(INC) -c -o $@ $<

lib: $(TARGET_LIB_DIR)/$(TARGET_LIB).a
bin: $(TARGET_BIN_DIR)/$(TARGET_BIN)

clean:
	rm -rvf $(TARGET_BIN_DIR) $(TARGET_LIB_DIR) $(BUILDDIR)

default: bin