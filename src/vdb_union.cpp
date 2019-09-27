/*

Copyright 2019 by Freakin' Sweet Apps, LLC (vdb_cmd@freakinsweetapps.com)

    This file is part of vdb_cmd.

    vdb_cmd is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openvdb/openvdb.h>
#include <openvdb/tools/LevelSetSphere.h>
#include <openvdb/tools/VolumeToMesh.h>
#include <openvdb/tools/MeshToVolume.h>
#include <openvdb/tools/VolumeToMesh.h>
#include <openvdb/math/Transform.h>
#include <openvdb/tools/Composite.h>
#include <iostream>

void print_usage() {
    fprintf(stderr, "usage: vdb_union <vdb file> [ <vdb file> ... ] <output file>\n");
    fprintf(stderr, "    Performs a union operation on all listed vdb files (make sure they're the same voxel size).\n");
}

int main(int argc, char **argv)
{
    if(argc < 3) {
        print_usage();
        exit(2);
    }

    // Initialize the OpenVDB library.  This must be called at least
    // once per program and may safely be called multiple times.
    openvdb::initialize();

    openvdb::io::File file(argv[1]);
    file.open();
    openvdb::FloatGrid::Ptr grid;

    for (openvdb::io::File::NameIterator nameIter = file.beginName();
        nameIter != file.endName(); ++nameIter) {
      openvdb::GridBase::Ptr baseGrid = file.readGrid(nameIter.gridName());

      openvdb::FloatGrid::Ptr curGrid = openvdb::gridPtrCast<openvdb::FloatGrid>(baseGrid);
      if(curGrid != NULL) {
        grid = curGrid;
      }
    }

    for(int i = 2; i < argc-1; i++) {
      openvdb::io::File curFile(argv[i]);
      curFile.open();

      for (openvdb::io::File::NameIterator nameIter = curFile.beginName();
          nameIter != curFile.endName(); ++nameIter) {
        openvdb::GridBase::Ptr baseGrid = curFile.readGrid(nameIter.gridName());

        openvdb::FloatGrid::Ptr curGrid = openvdb::gridPtrCast<openvdb::FloatGrid>(baseGrid);
        if(curGrid != NULL) {
          openvdb::tools::csgUnion(*grid, *curGrid, true);
        }
      }
    }

    char *out_filename = argv[argc-1];
    openvdb::GridPtrVec grids;
    grids.push_back(grid);

    openvdb::io::File outFile(out_filename);
    outFile.write(grids);
    outFile.close();

}
