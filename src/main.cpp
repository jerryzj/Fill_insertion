#include "parser.hpp"
#include "layout.hpp"
#include "rectangle.hpp"

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
    readprocess process;
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
    // read process file
    pos = filename.find_last_of('/');
    filename.replace(pos+1,filename.length()-pos,config.process);
    process.read_file((char*) filename.c_str());
   
    // 0613 : bin_size is set to window_size / 2
    layout.set_bin_size(process.window_size / 2);
    // set critical net list in layout class 
    layout.set_critical(config.critical_nets);
    // set DRC rules
    layout.set_rules(rule.rules);
    // create 3D bin structure
    layout.create3Dbin();
    layout.bin_mapping();
    layout.fill_insertion();
    // Checking
    cout << "**************Find cost before Opt***************" << endl;
    layout.find_cost_all(process); 
    layout.window_based_density_check();
    layout.DRC_check_width();
    layout.DRC_check_space();
    layout.dump_statistic();
    
    cout << "**************Find cost after Opt***************" << endl;
    for (int layer = 1; layer <= 8; layer++)
        layout.layer_optimization(process, layer);
    layout.dump_statistic();

    
    for (int epoch = 0; epoch < 20; epoch++) {
        cout << "****epoch = " << epoch << endl;
        layout.fill_insertion();
        for (int layer = 1; layer <= 8; layer++)
            layout.layer_optimization(process, layer);
        layout.dump_statistic();
        layout.fill_sort();
        layout.fill_remapping();
    }
    


    cout << "**************Final Check***************" << endl;
    layout.window_based_density_check();
    layout.DRC_check_width();
    layout.DRC_check_space();
    //layout.bin_optimization(process, 1, 50, 50);

    //layout.dump_bin(7, 50, 50);
    layout.dump_result();
    return 0;
}