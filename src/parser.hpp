#ifndef PARSER_HPP
#define PARSER_HPP
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string.h>

using namespace std;

class readconfig{
    public:
        // member functions    
        readconfig();
        void read_file(char* filename);
        void dump();
        // variables        
        string input;   // input filename
        string output;  // output filename
        string rule;    // rule filename
        string process; // process filename
        vector<int> critical_nets;
        vector<int> power_nets;
        vector<int> ground_nets;
};

struct rule{
    int layer_id;
    string layer_type;  // COUDUCTOR or VIA
    int min_width;
    int min_space;
    int max_fill_width;
    double min_density;
    double max_density;
    // constructor
    rule(){
        layer_type.reserve(10);
    }
};

class readrule{
    public:
        // member functions
        readrule();
        void read_file(char* filename);
        void dump();
        // variables
        int rule_num;   // number of rules
        vector<rule> rules;
}; 

class readlayout{
public:
    enum NetType {Normal, Fill, Bound, NA};
    struct rectangle{
        int bl_x;   // buttom left X
        int bl_y;   // buttom left y
        int tr_x;   // top right x
        int tr_y;   // top right y
    };
    struct net{
        int poly_id;
        rectangle rect;
        int net_id;
        int layer;
        NetType type;
    };
    readlayout();
    void read_file(char* filename);
    void dump();
    
    // list of nets 
    // net_list[0] stores "layout boundary"
    // both boundary and all nets are normalized by offset
    // offset = buttom-left corner of (original) boundary
    vector<net> net_list;
    int offset_x; 
    int offset_y;
    // need window size to set bin size: bin = 0.5 * window 
    int bin_size; 
    // 3D array layer*row*col
    // layer = 9 (L1 to L9), L0 stores nothing 
    

};


#endif