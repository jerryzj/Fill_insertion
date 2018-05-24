#ifndef LAYOUT_HPP
#define LAYOUT_HPP
#include <iostream>
#include <fstream>
#include <vector>
#include <string.h>
#include "rectangle.hpp"

using namespace std;

// create another file rectangle
/*
struct rectangle{
        int bl_x;   // buttom left X
        int bl_y;   // buttom left y
        int tr_x;   // top right x
        int tr_y;   // top right y
};
*/
// TODO: calculate overlapped area
// TODO: calculate distance between two rectangles(DRC)

class Layout{
public:    
    struct net{
        Rectangle rect; 
        int net_id;
        int layer;      // layer 1~9
    };
    struct bin{
        vector<int>* normal;
        vector<int>* fill;
        int normal_area;
        int fill_area;
        bin(){  // structure constructor
            normal = new vector<int>;
            fill = new vector<int>;
            normal_area = 0;
            fill_area = 0;
        }
    };
     
    Layout();
    // read layout file
    void read_file(char* filename);
    // print layout file (for debugging)
    void dump();
    // create 3D bin 
    void create3Dbin();
    // set bin size
    void set_bin_size(int size);
    // Map raw layout file to 3D bin 
    void bin_mapping();

    // this updates the normal and fill area of grid[_l][_x][_y] 
    void bin_normal_area(int _l, int _x, int _y); 

    // list of nets 
    // normal_list[0] stores "layout boundary"
    // both boundary and all nets are normalized by offset
    // offset = buttom-left corner of (original) boundary
    vector<net> normal_list;
    vector<net> fill_list;

    // 3D bin layer*row*col
    // layer = 9 (L1 to L9), L0 stores nothing 
    bin ***grid;

    private:
    // need window size to set bin size: bin = 0.5 * window
    int bin_size;
    // initial layout boundary offset
    int offset_x; 
    int offset_y;
};

#endif