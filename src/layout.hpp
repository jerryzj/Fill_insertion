#ifndef LAYOUT_HPP
#define LAYOUT_HPP
#include <iostream>
#include <fstream>
#include <vector>
#include <string.h>

using namespace std;

class Layout{
public:
    //enum NetType {Normal, Fill, Bound, NA};
    struct rectangle{
        int bl_x;   // buttom left X
        int bl_y;   // buttom left y
        int tr_x;   // top right x
        int tr_y;   // top right y
    };
    struct net{
        rectangle rect;
        int net_id;
        int layer;
    };
    struct bin{
        vector<int>* normal;
        vector<int>* fill;
        bin(){
            normal = new vector<int>;
            fill = new vector<int>;
        }
    };
    
    
    Layout();
    void read_file(char* filename);
    void dump();
    void create3Dmatrix();
    void set_bin_size(int size);
    // list of nets 
    // net_list[0] stores "layout boundary"
    // both boundary and all nets are normalized by offset
    // offset = buttom-left corner of (original) boundary
    vector<net> net_list;
    // 3D array layer*row*col
    // layer = 9 (L1 to L9), L0 stores nothing 
    bin ***grid;

    private:
     // need window size to set bin size: bin = 0.5 * window
    int bin_size; 
    int offset_x; 
    int offset_y;
};

#endif