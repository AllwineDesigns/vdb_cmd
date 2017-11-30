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
#include <openvdb/openvdb.h>
#include <openvdb/tools/LevelSetSphere.h>
#include <openvdb/tools/VolumeToMesh.h>
#include <openvdb/tools/MeshToVolume.h>
#include <openvdb/tools/VolumeToMesh.h>
#include <openvdb/math/Transform.h>
#include <openvdb/tools/Composite.h>
#include <iostream>

void print_usage() {
    fprintf(stderr, "usage: vdb_boolean -a <vdb file A> -b <vdb file B> [ -i ] [ -u ] [ -d ] <output file>\n");
    fprintf(stderr, "    Performs a CSG boolean operation on vdb files A and B (make sure they're the same voxel size).\n"
                    "     -i - performs the intersection of A and B\n"
                    "     -u - performs the union of A and B (default)\n"
                    "     -d - performs the difference of A and B\n");
}

int main(int argc, char **argv)
{
    int errflg = 0;
    char *a_file;
    char *b_file;
    int a_set = 0;
    int b_set = 0;

    int unionAB = 0;
    int intersection = 0;
    int difference = 0;

    int c;

    while((c = getopt(argc, argv, "a:b:iud")) != -1) {
        switch(c) {
            case 'a':
                a_set = 1;
                a_file = optarg;
                break;
            case 'b':
                b_set = 1;
                b_file = optarg;
                break;
            case 'i':
                intersection = 1;
                if(unionAB || difference) {
                    fprintf(stderr, "intersection option provided when union or difference already specified.\n");
                    unionAB = 0;
                    difference = 0;
                }
                break;
            case 'u':
                unionAB = 1;
                if(intersection || difference) {
                    fprintf(stderr, "union option provided when intersection or difference already specified.\n");
                    intersection = 0;
                    difference = 0;
                }
                break;
            case 'd':
                difference = 1;
                if(intersection || unionAB) {
                    fprintf(stderr, "difference option provided when intersection or union already specified.\n");
                    intersection = 0;
                    unionAB = 0;
                }
                break;
            case '?':
                fprintf(stderr, "Unrecognized option: '-%c'\n", optopt);
                errflg++;
                break;
        }
    }

    if(!unionAB && !intersection && !difference) {
        unionAB = 1;
    }

    if(errflg || !(a_set && b_set) || optind >= argc) {
        print_usage();
        exit(2);
    }

    // Initialize the OpenVDB library.  This must be called at least
    // once per program and may safely be called multiple times.
    openvdb::initialize();
    
    openvdb::io::File fileA(a_file);
    openvdb::io::File fileB(b_file);

    fileA.open();
    fileB.open();

    openvdb::FloatGrid::Ptr gridA;
    openvdb::FloatGrid::Ptr gridB;

    for (openvdb::io::File::NameIterator nameIter = fileA.beginName();
        nameIter != fileA.endName(); ++nameIter) {
      openvdb::GridBase::Ptr baseGrid = fileA.readGrid(nameIter.gridName());

      openvdb::FloatGrid::Ptr grid = openvdb::gridPtrCast<openvdb::FloatGrid>(baseGrid);
      if(grid != NULL) {
        gridA = grid;
      }
    }

    for (openvdb::io::File::NameIterator nameIter = fileB.beginName();
        nameIter != fileB.endName(); ++nameIter) {
      openvdb::GridBase::Ptr baseGrid = fileB.readGrid(nameIter.gridName());

      openvdb::FloatGrid::Ptr grid = openvdb::gridPtrCast<openvdb::FloatGrid>(baseGrid);
      if(grid != NULL) {
        gridB = grid;
      }
    }

    if(unionAB) {
      openvdb::tools::csgUnion(*gridA, *gridB, true);
    }
    if(intersection) {
      openvdb::tools::csgIntersection(*gridA, *gridB, true);
    }
    if(difference) {
      openvdb::tools::csgDifference(*gridA, *gridB, true);
    }

    char *out_filename = argv[optind];
    openvdb::GridPtrVec grids;
    grids.push_back(gridA);

    openvdb::io::File file(out_filename);
    file.write(grids);
    file.close();

}
