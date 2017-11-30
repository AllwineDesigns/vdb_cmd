vdb_cmd
=======

Commands that manipulate OpenVDB sparse volume files. 

Getting started
---------------

Ensure you have OpenVDB installed. Below are steps to take to install it on a Mac.

    git clone https://github.com/AllwineDesigns/vdb_cmd.git
    cd vdb_cmd
    make

# Commands

### stl2vdb

   stl2vdb [-v <voxel size>] <input STL file> <output VDB file>

Rasterizes an STL mesh into an OpenVDB sparse volume levelset grid with the provided voxel size. If the voxel size is omitted, it defaults to .5.

### vdb2stl

   vdb2stl [-d <detail>] <input VDB file> <output STL file>

Convert an OpenVDB sparse volume levelset grid to an STL mesh with the provided detail (0-1). If detail is omitted, it defaults to .9.

### vdb_boolean

    vdb_boolean -a <VDB file A> -b <VDB file B> [ -i ] [ -u ] [ -d ] <out file>

Performs a CSG boolean operation on VDB levelset grids A and B. -i will perform the intersection of A and B. -u will 
perform the union of A and B. -d will perform the difference of A and B.

# Install Steps for Mac

    # Install most dependencies with homebrew
    brew install boost
    brew install tbb
    brew install openexr
    brew install ilmbase
    brew install cppunit
    brew install log4cplus
    brew install glfw
    brew install doxygen

    # install blosc from source
    curl -L https://github.com/Blosc/c-blosc/archive/v1.12.1.tar.gz -o c-blosc-1.12.1.tar.gz
    cd c-blosc-1.12.1
    mkdir build
    cd build
    cmake -DCMAKE_INSTALL_PREFIX=/usr/local ..
    cmake --build .
    ctest
    cmake --build . --target install

## Edit Makefile

Change DESTDIR to `/usr/local`

Change Homebrew paths
* boost - `/usr/local/Cellar/boost/<version>/`
* tbb - `/usr/local/Cellar/tbb/<version>/`
* openexr - `/usr/local/Cellar/openexr/<version>/`
* etc.
* python - `/usr/local/Cellar/python/<version>/Frameworks/Python.framework/Headers`

Change Source paths
* blosc - `/usr/local/`

Comment out `CONCURRENT_MALLOC_LIB := -ljemalloc`

Uncomment `CONCURRENT_MALLOC_LIB := -ltbbmalloc_proxy -ltbbmalloc`

Set `CONCURRNET_MALLOC_LIB_DIR` to `$(TBB_LIB_DIR)`

Comment out python

Change `libboost_thread` to `libboost_thread-mt`

Copyright 2017 Freakin' Sweet Apps, LLC (stl_cmd@freakinsweetapps.com)
