BIN_DIR := bin
CMDS := $(addprefix $(BIN_DIR)/,stl2vdb vdb2stl vdb_boolean)

CC := g++
CFLAGS=-O3

all: $(CMDS)

$(BIN_DIR)/%: src/%.cpp
	g++ -std=c++11 -I/usr/include -I/tmp/openvdb -L/tmp/openvdb/openvdb/ -I/tmp/tbb-2019_U5/include -L/tmp/tbb-2019_U5/build/linux_intel64_gcc_cc4.8.5_libc2.17_kernel4.9.125_release -I/usr/local/include -L/usr/local/lib -ltbb -lopenvdb `pkg-config --libs OpenEXR` $(OUTPUT_OPTION) $<

$(CMDS): | $(BIN_DIR)

$(BIN_DIR):
	mkdir $(BIN_DIR)

clean:
	rm -rf $(BIN_DIR)
