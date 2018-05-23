#include "parser.hpp"

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
    readconfig config;  // circuit config file
    readrule rule;      // rule.dat file
    readlayout layout;
    size_t pos;         // position of certain char

    if(argc != 2){
        cerr<<"Use ./a.out (config relative path)\n";
        return -1;
    }
    config.read_file((char*)filename.c_str());
    config.dump();
    pos = filename.find_last_of('/');
    filename.replace(pos+1,filename.length()-pos,config.rule);
    // here filename should be ./circuit#/rule.dat
    //cout<<filename;
    // for testing 
    rule.read_file((char*)filename.c_str());
    rule.dump();
    pos = filename.find_last_of('/');
    filename.replace(pos+1,filename.length()-pos,config.input);
    layout.read_file((char*)filename.c_str());
    
    layout.dump();
    return 0;
}