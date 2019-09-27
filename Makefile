BIN_DIR := bin
CMDS := $(addprefix $(BIN_DIR)/,stl2vdb vdb2stl vdb_boolean vdb_union)

CC := g++
CFLAGS=-O3

all: $(CMDS)

$(BIN_DIR)/%: src/%.cpp
	g++ -std=c++11 -I/usr/local/include -L/usr/local/lib -ltbb -lopenvdb `pkg-config --libs OpenEXR` $(OUTPUT_OPTION) $<

$(CMDS): | $(BIN_DIR)

$(BIN_DIR):
	mkdir $(BIN_DIR)

clean:
	rm -rf $(BIN_DIR)
