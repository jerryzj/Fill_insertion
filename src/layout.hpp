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
        double cost;
        net(){
            net_id = 0;
            layer = 0;
            cost = 0;
        }
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
     
    Layout(){
        normal_list.reserve(3E5);
        fill_list.reserve(3E5);
        // pos 0 is empty
        min_density.reserve(10);

        min_width.reserve(10);

        max_fill_width.reserve(10);

        min_space.reserve(10);
    }
    //*************************
    //     I/O, print info
    //*************************

    // read layout file
    void read_file(char* filename);
    // print layout file content (for debugging)
    //      all : dump normal list and fill list
    //      normal: dump normal list only
    //      fill : dump fill list only
    void dump(string mode = "all");
    // print layer statistics
    void dump_statistic();
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
    // assign normal poly to Bin (named "grid")
    void assign_normal(int i);
    // assign fill metal to Bin (named "grid")
    void assign_fill(int i);
    // delete fill with index = i from bins 
    // also note that fill net_id will be set to -1
    void delete_fill(int i);
    // update fill of index = i to new rectangle r_new
    void resize_fill(int i, const Rectangle& r_new);

    //*************************
    //     Fill reordering
    //*************************  

    // remove invalid fill metal and sort 
    void fill_sort();
    // remap the sorted fill list to Bin (named "grid")
    void fill_remapping();
    // check if a net is critical net
    bool Is_critical(int net_id, vector<int>& list);
    
    //*************************
    //Fill insertion Algorithms  
    //*************************

    // find available fill region = region that is not normal poly
    // 6/01: add available space merge 
    vector<Rectangle> find_fill_region_x(int layer, int i, int j, int s = 1);
    vector<Rectangle> find_fill_region_y(int layer, int i, int j, int s = 1);
    // Insert fill by given available region
    void fill_insertion();
    // insert fill in availiable fill region, only called by fill_insertion
    // 0605: check, when width > max_fill_width, 
    //       add fill which size <= max_fill_width 
    //       by calculating width_left and lenfth_left
    void metal_fill(int layer, const vector<Rectangle>& fill_regions);
    // randomly added a fill
    //      s = 1, bin based filling
    //      s = 2, window based filling
    void random_fill(int layer, int i, int j, int s=1);
    // Expand fill area, only called by random
    void random_expand(net& _net, int layer, int i, int j, int s, int step, string mode);
    // find cost of an added fill metal
    double find_cost(const readprocess& process, const Rectangle& _rec);

    //*************************
    //      Rule checking  
    //*************************

    // set rule information from rule file
    void set_rules(const vector<rule>& _rules);
    // check min_width, max_fill_width 
    void DRC_check_width();
    // 0605: add, construct based on one_window_DRC_check_space
    bool DRC_check_space();
    // 0605: add, in one window 
    // check min_space between fill and fill, fill and normal
    bool one_window_DRC_check_space(int layer, int i, int j, int s = 2);
    // one net check space
    bool one_net_DRC_check_space(const net& _net, int index = -1);
    // 0605: add, check density for one window and verify area calculation
    //      s = 1 : bin based check
    //      s = 2 : window based check
    bool one_window_density_check(int layer, int i, int j, int s = 2);       
    // 0605: add, construct based on one_window_density_check
    void window_based_density_check();

    //*************************
    //         Variables 
    //*************************
    
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
};

#endif
