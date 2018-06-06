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
    // for testing 6 
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
    
    layout.set_rules(rule.rules);   // input vector<rule>

    cout << endl;
    layout.fill_insertion();
    // dump specific in 
    //layout.dump_bin(9, 0, 105);
    
    layout.window_based_density_check();
    layout.DRC_check_width();
    layout.DRC_check_space();
    layout.dump_statistic();

    cout << "afawgegbreeagar" << endl;
    layout.fill_sort();
    layout.fill_remapping();

    layout.window_based_density_check();
    layout.DRC_check_width();
    layout.DRC_check_space();
    layout.dump_statistic();
    
    layout.dump("fill"); 
    //layout.dump_bin(9, 0, 105);
    return 0;
}