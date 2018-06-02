#include "parser.hpp"
#include "layout.hpp"


int main(int argc ,char *argv[]){
    // Cin/Cout Optimization
    // Ref: https://tinyurl.com/z4pprcc
    // Turn off sync between cin/cout and stdin/stdout
    // Therefore we can't use scanf/printf anymore
    ios::sync_with_stdio(false);
    // Untie cin and cout
    // Therefore we can't prompt user to input sth.
    cin.tie(0);

    string filename = argv[1];   
    // Parser class 
    readconfig config;  // circuit config file
    readrule rule;      // rule.dat file
    Layout layout;
    // Variables
    size_t pos;         // position of certain char

    if(argc != 2){
        cerr<<"Use ./a.out (config relative path)\n";
        return -1;
    }
    // read config file
    config.read_file((char*)filename.c_str());
    //config.dump();
    // read rule file
    pos = filename.find_last_of('/');
    filename.replace(pos+1,filename.length()-pos,config.rule);
    // here filename should be ./circuit#/rule.dat
    //cout<<filename;
    // for testing 
    rule.read_file((char*)filename.c_str());
    //rule.dump();
    // read layout file
    pos = filename.find_last_of('/');
    filename.replace(pos+1,filename.length()-pos,config.input);
    // here filename should be ./circuit#/layout*.cut
    // cout<<filename;
    // for testing 
    layout.read_file((char*)filename.c_str());
    //layout.dump();
    // Temporary set bin size = 5000
    layout.set_bin_size(5000);
    layout.create3Dbin();
    layout.bin_mapping();
    
    int layer = 1;
    for(auto i: rule.rules){
        layout.set_min_density(layer, i.min_density);
        layout.set_min_width(layer, i.min_width);
        layout.set_max_fill_width(layer, i.max_fill_width);
        layout.set_min_space(layer, i.min_space);
        layer++;    
    }
    cout << endl;
    layout.fill_insertion();
    // dump specific in
    //layout.dump_bin(1, 24, 67);
    layout.window_based_density_check();
    layout.DRC_check_width();
    layout.DRC_check_space();
    return 0;
}