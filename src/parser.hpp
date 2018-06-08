#ifndef PARSER_HPP
#define PARSER_HPP
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
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

class readprocess{
    public:
    readprocess();
    // read process file
    void read_file(char* filename);
    // find area capacitance
    double find_area(int x, int y, int area);
    // find lateral capacintance
    double find_lateral(int x, int distance, int parallel_edge);
    // find fringe capacintance
    double find_fringe(int x, int y, int distance, int parallel_edge);
    // Debug
    void dump();
    // window size (for DRC)
    int window_size;

    private:
    enum table_type{
        area, lateral, fringe
    };

    struct table{
        string name;
        vector<int> ranges;
        vector<double> alpha;
        vector<double> beta;
    
        table(){
            ranges.reserve(12);
            alpha.reserve(11);
            beta.reserve(11);
        }
    };
    // generate a string as the key to search cap_table
    string key_gen(int x,int y, table_type type);
    map<string, table> cap_table;

}; 

#endif