#ifndef PARSER_HPP
#define PARSER_HPP
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

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

class layout{
public:
    enum NetType {Normal, Fill};
    struct rectangle{
        int bl_x;   // buttom left X
        int bl_y;   // buttom left y
        int tr_x;   // top right x
        int tr_y;   // top right y
    };
    struct net{
        int poly_id;
        int net_id;
        int layer;
        rectangle rect;
    };
    layout();
    void read_file(char* filename);

    rectangle boundary;
    // need window size to set bin size: bin = 0.5 * window 
    int bin_size; 
    // 3D array layer*row*col
    // layer = 9 (L0 to L8), need index conversion 
    

};


#endif