# Copyright (C) 2018 Gabriele Proietti Mattia <gabry.gabry@hotmail.it> & Alexandru Daniel Tufa <alex.tufa94@gmail.com>
# 
# This file is part of Fibers (Library).
# 
# Fibers (Library) is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# Fibers (Library) is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with Fibers (Library).  If not, see <http://www.gnu.org/licenses/>.
# 

CC := gcc
SRCDIR := src
INCDIR := include
BUILDDIR := build
DOCDIR := doxy
CFLAGS := -std=gnu11 -Wall -g
LIBS := -pthread
INC := -Iinclude

TARGET_BIN_DIR := bin
TARGET_LIB_DIR := lib
TARGET_LIB = libfiber
TARGET_BIN = fiber_example

$(TARGET_BIN_DIR)/$(TARGET_BIN): $(BUILDDIR)/$(TARGET_BIN).o $(TARGET_LIB_DIR)/$(TARGET_LIB).a
	@echo "==> Compiling binary..."
	@mkdir -p $(TARGET_BIN_DIR) 
	@$(CC) $(CFLAGS) $(LIBS) $(INC) $^ -o $@

$(BUILDDIR)/$(TARGET_BIN).o: $(SRCDIR)/fiber_example.c
	@echo "==> Compiling binary object..."
	@mkdir -p $(BUILDDIR) 
	@$(CC) $(CFLAGS) $(LIBS) $(INC) -c $< -o $@

$(TARGET_LIB_DIR)/$(TARGET_LIB).a: $(BUILDDIR)/$(TARGET_LIB).o
	@echo "==> Packing library..."
	@mkdir -p $(TARGET_LIB_DIR) 
	@ar rcs $@ $^

$(BUILDDIR)/$(TARGET_LIB).o: $(SRCDIR)/core.c $(INCDIR)/*.h
	@echo "==> Compiling library object..."
	@mkdir -p $(BUILDDIR) 
	@$(CC) $(CFLAGS) $(LIBS) $(INC) -c -o $@ $<

lib: $(TARGET_LIB_DIR)/$(TARGET_LIB).a
bin: $(TARGET_BIN_DIR)/$(TARGET_BIN)
doc: 
	doxygen Doxyfile

clean:
	rm -rvf $(TARGET_BIN_DIR) $(TARGET_LIB_DIR) $(BUILDDIR) $(DOCDIR)

default: bin