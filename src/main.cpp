#include <iostream>
#include <cstdio>
#include <fstream>
#include <vector>
#include <sstream>

using namespace std;

// Class/ Structure declarations
class readconfig{
    public:
        void read_file(char* filename);
        void dump();
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
    enum layer_type{
        CONDUCTOR,VIA
    };
    // Note the data type used here
    // may need to use double(?)
    int min_width;
    int min_space;
    int max_fill_width;
    double min_density;
    double max_density;
};

class readrule{
    public:
        void read_file(char* filename);
        void dump();
    private:
        int rule_num;   // number of rules
        vector<rule> rules;

};

// Memeber function implementations
void readconfig::read_file(char* filename){
    ifstream file(filename);
    string temp;
    int n = 0;
    size_t pos;
    string num;

    if(!file){
        cerr<<"Can't open config file\n";
        exit(-1);
    }
    // read input filename
    getline(file,temp);
    pos = temp.find(" ");
    input = temp.substr(++pos);
    // read output filename
    getline(file,temp);
    pos = temp.find(" ");
    output = temp.substr(++pos);
    // read design rule filename
    getline(file,temp);
    pos = temp.find(" ");
    rule = temp.substr(++pos);
    // read process filename
    getline(file,temp);
    pos = temp.find(" ");
    process = temp.substr(++pos);
    // read critical nets
    getline(file,temp);
    pos = temp.find(" ");
    num = temp.substr(++pos);
    stringstream stream(num);
    while(stream >> n){
        critical_nets.push_back(n);
    }
    // read power nets
    getline(file,temp);
    pos = temp.find(" ");
    num = temp.substr(++pos);
    stream.str("");     // clean sstream buffer
    stream.clear();     // reset sstream flag 
    stream.str(num);    // assign new string
    while(stream >> n){
        power_nets.push_back(n);
    }
    // read ground nets
    getline(file,temp);
    pos = temp.find(" ");
    num = temp.substr(++pos);
    stream.str("");     // clean sstream buffer
    stream.clear();     // reset sstream flag 
    stream.str(num);    // assign new string
    while(stream >> n){
        ground_nets.push_back(n);
    }
    file.close();
}

void readconfig::dump(){
    cout<<"Input = "<<input<<endl;
    cout<<"Output = "<<output<<endl;
    cout<<"Rule = "<<rule<<endl;
    cout<<"Process = "<<process<<endl;
    cout<<"Critical = ";
    for(auto i:critical_nets){
        cout<<i<<" ";
    }
    cout<<endl;
    cout<<"Power = ";
    for(auto i:power_nets){
        cout<<i<<" ";
    }
    cout<<endl;
    cout<<"Ground = ";
    for(auto i:ground_nets){
        cout<<i<<" ";
    }
    cout<<endl;
}

void readrule::read_file(char* filename){

}
// ./a.out (config filename)
int main(int argc ,char *argv[]){
    readconfig config;
    readrule rule;

    if(argc != 2){
        cerr<<"Use ./a.out (config filename)\n";
        return -1;
    }
    config.read_file(argv[1]);
    config.dump();
    // need to reuse argv[1] or prompt a nother argument
    //rule.read_file(" ");
    return 0;
}