BIN_DIR := bin
CMDS := $(addprefix $(BIN_DIR)/,stl2vdb vdb2stl vdb_boolean)

CC := g++
CFLAGS=-O3

TBB_LIB_DIR := $(shell echo -n /tmp/tbb-2019_U5/build/*_release)

all: $(CMDS)

$(BIN_DIR)/%: src/%.cpp
	g++ -std=c++11 -I/usr/include -I/tmp/openvdb -L/tmp/openvdb/openvdb/ -I/tmp/tbb-2019_U5/include -L$(TBB_LIB_DIR)  -I/usr/local/include -L/usr/local/lib -ltbb -lopenvdb `pkg-config --libs OpenEXR` $(OUTPUT_OPTION) $<

$(CMDS): | $(BIN_DIR)

$(BIN_DIR):
	mkdir $(BIN_DIR)

clean:
	rm -rf $(BIN_DIR)
