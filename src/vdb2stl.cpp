/*

Copyright 2017 by Allwine Designs, LLC

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
#include <openvdb/tools/VolumeToMesh.h>
#include <openvdb/tools/MeshToVolume.h>
#include <openvdb/tools/VolumeToMesh.h>
#include <openvdb/math/Transform.h>
#include <openvdb/tools/Composite.h>
#include <iostream>

#include "stl_util.h"

using namespace openvdb::v5_0;

// taken from stl_threads, should probably be factored out to a shared header/library or something
void write_tri(FILE *f,
                vec *p1, 
                vec *p2, 
                vec *p3, int rev) {
    if(rev) {
        vec *tmp = p1;
        p1 = p3;
        p3 = tmp;
    }

    vec n1;
    vec v1,v2;

    vec_sub(p2,p1,&v1);
    vec_sub(p3,p1,&v2);

    vec_cross(&v1,&v2,&n1);
    vec_normalize(&n1,&n1);

    uint16_t abc = 0;

    fwrite(&n1, 1, 12, f);
    fwrite(p1, 1, 12, f);
    fwrite(p2, 1, 12, f);
    fwrite(p3, 1, 12, f);
    fwrite(&abc,1,  2, f);
}

// taken from stl_threads, should probably be factored out to a shared header/library or something
void write_quad(FILE *f,
                vec *p1, 
                vec *p2, 
                vec *p3, 
                vec *p4,int rev) {
    //write two triangles 1,2,3 and 1,3,4
    if(rev) {
        vec *tmp = p1;
        p1 = p4;
        p4 = tmp;

        tmp = p2;
        p2 = p3;
        p3 = tmp;
    }

    vec n1,n2;

    vec v1,v2,v3;

    vec_sub(p2,p1,&v1);
    vec_sub(p3,p1,&v2);
    vec_sub(p4,p1,&v3);

    vec_cross(&v1,&v2,&n1);
    vec_normalize(&n1,&n1);

    vec_cross(&v2,&v3,&n2);
    vec_normalize(&n2,&n2);

    uint16_t abc = 0;

    fwrite(&n1, 1, 12, f);
    fwrite(p1, 1, 12, f);
    fwrite(p2, 1, 12, f);
    fwrite(p3, 1, 12, f);
    fwrite(&abc,1,  2, f);

    fwrite(&n2, 1, 12, f);
    fwrite(p1, 1, 12, f);
    fwrite(p3, 1, 12, f);
    fwrite(p4, 1, 12, f);
    fwrite(&abc,1,  2, f);
}

void print_usage() {
    fprintf(stderr, "usage: vdb2stl [ -d <detail>] <input file> <output file>\n");
    fprintf(stderr, "    Converts a VDB levelset to an STL mesh with the specified detail (0-1). If detail is omitted, it defaults to .9.\n");
}

int main(int argc, char **argv)
{
    int errflg = 0;

    float detail = .9;

    int c;

    while((c = getopt(argc, argv, "d:")) != -1) {
        switch(c) {
            case 'd':
                detail = atof(optarg);
                break;
            case '?':
                fprintf(stderr, "Unrecognized option: '-%c'\n", optopt);
                errflg++;
                break;
        }
    }

    if(errflg || optind >= argc+1) {
        print_usage();
        exit(2);
    }

    char *in_filename = argv[optind];
    char *out_filename = argv[optind+1];

    // Initialize the OpenVDB library.  This must be called at least
    // once per program and may safely be called multiple times.
    openvdb::initialize();
    
    openvdb::io::File file(in_filename);

    // Open the file.  This reads the file header, but not any grids.
    file.open();

    std::vector<Vec3s> out_verts;
    std::vector<Vec3I> out_tris;
    std::vector<Vec4I> out_quads;

    openvdb::GridBase::Ptr baseGrid;
    for (openvdb::io::File::NameIterator nameIter = file.beginName();
        nameIter != file.endName(); ++nameIter) {
      baseGrid = file.readGrid(nameIter.gridName());

      openvdb::FloatGrid::Ptr grid = openvdb::gridPtrCast<openvdb::FloatGrid>(baseGrid);
      if(grid != NULL) {
        tools::volumeToMesh<openvdb::FloatGrid>(*grid, out_verts, out_tris, out_quads, 0, 1-detail);
        std::cout << "verts size: " << out_verts.size() << std::endl;
        std::cout << "tris size: " << out_tris.size() << std::endl;
        std::cout << "quads size: " << out_quads.size() << std::endl;
      }
    }

    std::cout << "Writing mesh..." << std::endl;
    FILE *f;

    f = fopen(out_filename, "wb");

    char header[80] = {0};
    strncpy(header, "Created with vdb2stl", 80);
    fwrite(header, 80, 1, f);

    uint32_t num_tris;

    num_tris = out_tris.size()+out_quads.size()*2;
    fwrite(&num_tris, 4, 1, f);

    uint16_t abc = 0;

    for(int i = 0; i < out_tris.size(); i++) {
        vec p1,p2,p3;

        p1.x = out_verts[out_tris[i].x()].x();
        p1.y = out_verts[out_tris[i].x()].y();
        p1.z = out_verts[out_tris[i].x()].z();

        p2.x = out_verts[out_tris[i].y()].x();
        p2.y = out_verts[out_tris[i].y()].y();
        p2.z = out_verts[out_tris[i].y()].z();

        p3.x = out_verts[out_tris[i].z()].x();
        p3.y = out_verts[out_tris[i].z()].y();
        p3.z = out_verts[out_tris[i].z()].z();

        write_tri(f, &p1, &p2, &p3, 1);
    }

    for(int i = 0; i < out_quads.size(); i++) {
        vec p1,p2,p3,p4;

        p1.x = out_verts[out_quads[i].x()].x();
        p1.y = out_verts[out_quads[i].x()].y();
        p1.z = out_verts[out_quads[i].x()].z();

        p2.x = out_verts[out_quads[i].y()].x();
        p2.y = out_verts[out_quads[i].y()].y();
        p2.z = out_verts[out_quads[i].y()].z();

        p3.x = out_verts[out_quads[i].z()].x();
        p3.y = out_verts[out_quads[i].z()].y();
        p3.z = out_verts[out_quads[i].z()].z();

        p4.x = out_verts[out_quads[i].w()].x();
        p4.y = out_verts[out_quads[i].w()].y();
        p4.z = out_verts[out_quads[i].w()].z();

        write_quad(f, &p1, &p2, &p3, &p4, 1);
    }

    fclose(f);
    std::cout << "All done!" << std::endl;
}
