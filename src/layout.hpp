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
    //*************************
    //        File I/O
    //*************************
    // read layout file
    void read_file(char* filename);
    // print layout file (for debugging)
    void dump(string mode = "all");

    void dump_statistic();

    void dump_fill_list();
    // select a bin and dump it into two files(normal.cut, fill.cut)
    void dump_bin(int layer, int offset_x, int offset_y);

    //*************************
    //     Bin structure
    //*************************
    // create 3D bin 
    void create3Dbin();
    // set bin size
    void set_bin_size(int size);
    // Map raw layout file to 3D bin 
    void bin_mapping();
    // this updates the normal and fill area of grid[_l][_x][_y] 
    void bin_normal_area(int _l, int _x, int _y); 
    void assign_normal(int i);
    void assign_fill(int i);

    void fill_sort();
    void fill_remapping();
    
    // delete fill with index = i from bins 
    // also set that fill net_id = -1
    void delete_fill(int i);

    // update fill of index = i to new rectangle r_new
    void resize_fill(int i, const Rectangle& r_new);


    // set rule information
    void set_rules(const vector<rule>& _rules);

    // find available fill region = region that is not normal poly
    // 6/01 add merge inside
    vector<Rectangle> find_fill_region_x(int layer, int i, int j, int s = 1);
    vector<Rectangle> find_fill_region_y(int layer, int i, int j, int s = 1);

    // insert fill in availiable fill region 
    // 0605 check, when width > max_fill_width, add fill which size <= max_fill_width by calculating width_left and lenfth_left
    void metal_fill(int layer, const vector<Rectangle>& fill_regions);
    
    // Insert fill
    void fill_insertion();

    // 0605 add, construct based on one_window_density_check
    void window_based_density_check();

    // 0605 add, check density for one window and verify area calculation
    // s = 2 means window based check
    bool one_window_density_check(int layer, int i, int j, int s = 2);   

    // 0605 add, in one window check min_space between fill and fill, fill and normal
    bool one_window_DRC_check_space(int layer, int i, int j, int s = 2);

    // 0605 add, construct based on one_window_DRC_check_space
    bool DRC_check_space();

    // check min_width, max_fill_width 
    void DRC_check_width();

    

    // random fill
    void random_fill(int layer, int i, int j, int s);
    void random_expand(net& _net, int layer, int i, int j, int s, int step, string mode);

    // no net check space
    bool one_net_DRC_check_space(const net& _net, int index = -1);


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
    vector<double> min_density;
    vector<int> min_width;
    vector<int> max_fill_width;
    vector<int> min_space;

    // 6/01
    int metal_fill_count;

    
};

#endif
