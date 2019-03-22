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
#include <openvdb/tools/LevelSetSphere.h>
#include <openvdb/tools/VolumeToMesh.h>
#include <openvdb/tools/MeshToVolume.h>
#include <openvdb/tools/VolumeToMesh.h>
#include <openvdb/math/Transform.h>
#include <openvdb/tools/Composite.h>
#include <iostream>

#include "stl_util.h"

using namespace openvdb;

void ReadMesh(const char *filename, std::vector<Vec3s>& verts,
                std::vector<Vec3I>& tris) {
    std::cout << "reading mesh" << std::endl;
    FILE *f;

    f = fopen(filename, "rb");

    char header[80];
    uint32_t num_tris;
    fread(header, 80, 1, f);
    fread(&num_tris, 4, 1, f);

    uint16_t abc;
    int index = 0;
    for(int i = 0; i < num_tris; i++) {
        float normal[3];
        float p1[3],p2[3],p3[3];
        fread(normal, 12, 1, f);
        fread(p1, 12, 1, f);
        fread(p2, 12, 1, f);
        fread(p3, 12, 1, f);
        fread(&abc, 2, 1, f);
     //   std::cout << p1[0] << ", " << p1[1] << ", " << p1[2] << std::endl;
     //   std::cout << p2[0] << ", " << p2[1] << ", " << p2[2] << std::endl;
     //   std::cout << p3[0] << ", " << p3[1] << ", " << p3[2] << std::endl;
        verts.push_back(Vec3s(p3[0], p3[1], p3[2]));
        verts.push_back(Vec3s(p2[0], p2[1], p2[2]));
        verts.push_back(Vec3s(p1[0], p1[1], p1[2]));
        tris.push_back(Vec3I(index, index+1, index+2));
        index += 3;
     //   std::cout << i << std::endl;
    }
    std::cout << "done reading mesh" << std::endl;

    fclose(f);
}

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
    fprintf(stderr, "usage: stl2vdb [ -v <voxel size> ] <input file> <output file>\n");
    fprintf(stderr, "    Rasterizes an STL mesh into an OpenVDB sparse volume using the specified voxel size. If the voxel size is omitted, it defaults to .5.");
}

int main(int argc, char **argv)
{
    int errflg = 0;
    float voxel_size = .5;

    int c;

    while((c = getopt(argc, argv, "v:")) != -1) {
        switch(c) {
            case 'v':
                voxel_size = atof(optarg);
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
    
    std::vector<Vec3s> vertices;
    std::vector<Vec3I> triangles;

    ReadMesh(in_filename, vertices, triangles);

    openvdb::math::Transform::Ptr transform = openvdb::math::Transform::createLinearTransform(voxel_size);

    std::cout << "Converting to levelset..." << std::endl;
    openvdb::FloatGrid::Ptr grid = tools::meshToLevelSet<openvdb::FloatGrid>(*transform, vertices, triangles,float(openvdb::LEVEL_SET_HALF_WIDTH));
    std::cout << "Conversion complete." << std::endl;

    openvdb::GridPtrVec grids;
    grids.push_back(grid);

    openvdb::io::File file(out_filename);
    file.write(grids);
    file.close();

    std::cout << "All done!" << std::endl;
}
