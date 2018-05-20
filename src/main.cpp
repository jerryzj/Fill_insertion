#include "parser.hpp"

int main(int argc ,char *argv[]){
    string filename = argv[1];    
    readconfig config;  // circuit config file
    readrule rule;      // rule.dat file
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
    
    return 0;
}