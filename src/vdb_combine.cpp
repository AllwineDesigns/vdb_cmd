/*

Copyright 2017 by Freakin' Sweet Apps, LLC (vdb_cmd@freakinsweetapps.com)

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
#include <vector>
#include <openvdb/openvdb.h>
#include <openvdb/tools/LevelSetSphere.h>
#include <openvdb/tools/VolumeToMesh.h>
#include <openvdb/tools/MeshToVolume.h>
#include <openvdb/tools/VolumeToMesh.h>
#include <openvdb/math/Transform.h>
#include <openvdb/tools/Composite.h>
#include <iostream>

void print_usage() {
    fprintf(stderr, "usage: vdb_combine -l <input vdb file> -o <output vdb file> [ -i <vdb file> ... ] [ -u <vdb file> ... ] [ -d <vdb file> ... ]\n");
    fprintf(stderr, "    Performs CSG boolean operations on vdb files (make sure they're the same voxel size).\n"
                    "     -i - performs the intersection\n"
                    "     -u - performs the union\n"
                    "     -d - performs the difference\n");
}

enum Op {
  UNION,
  INTERSECTION,
  DIFFERENCE
};
struct Operation {
  Op operation;
  char* filename;

  Operation(Op o, char* f) : operation(o), filename(f) {}
};

int main(int argc, char **argv) {
    std::vector<Operation> ops;

    int errflg = 0;
    char *input_file;
    char *output_file;
    int input_set = 0;
    int output_set = 0;

    int c;

    while((c = getopt(argc, argv, "l:o:i:u:d:")) != -1) {
        switch(c) {
            case 'l':
                input_set = 1;
                input_file = optarg;
                break;
            case 'o':
                output_set = 1;
                output_file = optarg;
                break;
            case 'i':
                ops.push_back(Operation(INTERSECTION, optarg));
                break;
            case 'u':
                ops.push_back(Operation(UNION, optarg));
                break;
            case 'd':
                ops.push_back(Operation(DIFFERENCE, optarg));
                break;
            case '?':
                fprintf(stderr, "Unrecognized option: '-%c'\n", optopt);
                errflg++;
                break;
        }
    }

    if(errflg || !(input_set && output_set) || optind != argc) {
        print_usage();
        exit(2);
    }

    // Initialize the OpenVDB library.  This must be called at least
    // once per program and may safely be called multiple times.
    openvdb::initialize();
    
    openvdb::io::File inputFile(input_file);

    inputFile.open();

    openvdb::FloatGrid::Ptr mainGrid;

    for (openvdb::io::File::NameIterator nameIter = inputFile.beginName();
        nameIter != inputFile.endName(); ++nameIter) {
      openvdb::GridBase::Ptr baseGrid = inputFile.readGrid(nameIter.gridName());

      openvdb::FloatGrid::Ptr grid = openvdb::gridPtrCast<openvdb::FloatGrid>(baseGrid);
      if(grid != NULL) {
        mainGrid = grid;
      }
    }

    std::vector<Operation>::iterator itr = ops.begin();
    while(itr != ops.end()) {
      Operation o = *itr;

      openvdb::io::File curFile(o.filename);

      curFile.open();

      for (openvdb::io::File::NameIterator nameIter = curFile.beginName();
          nameIter != curFile.endName(); ++nameIter) {
        openvdb::GridBase::Ptr baseGrid = curFile.readGrid(nameIter.gridName());

        openvdb::FloatGrid::Ptr grid = openvdb::gridPtrCast<openvdb::FloatGrid>(baseGrid);
        if(grid != NULL) {
          if(o.operation == UNION) {
            openvdb::FloatGrid::Ptr u = openvdb::tools::csgUnionCopy<openvdb::Grid<openvdb::FloatTree>>(*mainGrid, *grid);
            mainGrid = u;
          } else if(o.operation == INTERSECTION) {
            openvdb::FloatGrid::Ptr intersection = openvdb::tools::csgIntersectionCopy<openvdb::Grid<openvdb::FloatTree>>(*mainGrid, *grid);
            mainGrid = intersection;
            break;
          } else if(o.operation == DIFFERENCE) {
            openvdb::FloatGrid::Ptr difference = openvdb::tools::csgDifferenceCopy<openvdb::Grid<openvdb::FloatTree>>(*mainGrid, *grid);
            mainGrid = difference;
          }
        }
      }
      curFile.close();

      ++itr;
    }

    mainGrid->tree().prune();

    openvdb::GridPtrVec grids;
    grids.push_back(mainGrid);

    openvdb::io::File file(output_file);
    file.write(grids);
    file.close();

}
