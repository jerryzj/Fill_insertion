#ifndef LAYOUT_HPP
#define LAYOUT_HPP
#include <iostream>
#include <fstream>
#include <vector>
#include <string.h>
#include <algorithm>
#include "rectangle.hpp"
#include "parser.hpp"   // in order to read rule file


using namespace std;

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
        vector<int>* init_fill;
        int normal_area;
        int fill_area;
        bin(){  // structure constructor
            normal = new vector<int>;
            fill = new vector<int>;
            init_fill = new vector<int>;
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

    
    // set rule information
    void set_rules(const vector<rule>& _rules);

    // find available fill region = region that is not normal poly
    // 6/01 add merge inside
    void find_fill_region_x(int layer, int i, int j);
    void find_fill_region_y(int layer, int i, int j);

    // insert fill in availiable fill region 
    void metal_fill(int layer, int i, int j);

    // random fill to improve density
    // ratio define square size = ratio * min_space 
    void random_fill(int layer, int i, int j, int x_ratio, int y_ratio);


    // Insert fill
    void fill_insertion();

    void window_based_density_check();

    // check min_width, max_fill_width 
    void DRC_check_width();

    // check min_space between fill and fill, fill and normal
    void DRC_check_space();

    void dump_fill_list();
    // select a bin and dump it into two files 
    void dump_bin(int layer, int offset_x, int offset_y);

    // list of nets 
    // normal_list[0] stores "layout boundary"
    // both boundary and all nets are normalized by offset
    // offset = buttom-left corner of (original) boundary
    vector<net> normal_list;
    vector<net> fill_list;
    vector<net> init_fill_list;
    // 3D bin layer*row*col
    // layer = 9 (L1 to L9), L0 stores nothing 
    bin ***grid;

private:
    // need window size to set bin size: bin = 0.5 * window
    int bin_size;
    // initial layout boundary offset
    int offset_x; 
    int offset_y;
    vector<double> min_density;
    vector<int> min_width;
    vector<int> max_fill_width;
    vector<int> min_space;

    // 6/01
    int init_fill_count = 0;
    int metal_fill_count = 0;
    int random_fill_count = 0;

    
};

#endif