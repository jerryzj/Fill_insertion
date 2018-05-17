#include <iostream>
#include <cstdio>
#include <fstream>
#include <vector>
#include <sstream>

using namespace std;

class readconfig{
    public:
        void read_file(char* filename);
        void dump();
    private:
        string input;   // input filename
        string output;  // output filename
        string rule;    // rule filename
        string process; // process filename
        vector<int> critical_nets;
        vector<int> power_nets;
        vector<int> ground_nets;
};

void readconfig::read_file(char* filename){
    fstream file;
    string temp;
    int n = 0;
    size_t pos;
    string num;

    file.open(filename,ios::in);
    getline(cin,temp);
    pos = temp.find("design: ");
    input = temp.substr(pos);
    getline(cin,temp);
    pos = temp.find("output: ");
    output = temp.substr(pos);
    getline(cin,temp);
    pos = temp.find("rule_file: ");
    rule = temp.substr(pos);
    getline(cin,temp);
    pos = temp.find("process_file: ");
    process = temp.substr(pos);
    // read critical nets
    getline(cin,temp);
    pos = temp.find("critical_nets: ");
    num = temp.substr(pos);
    stringstream stream(num);
    while(stream >> n){
        critical_nets.push_back(n);
    }
    stream.clear();
    // read power nets
    getline(cin,temp);
    pos = temp.find("power_nets: ");
    num = temp.substr(pos);
    stream.str(num);
    while(stream >> n){
        critical_nets.push_back(n);
    }
    stream.clear();
    // read ground nets
    getline(cin,temp);
    pos = temp.find("ground_nets: ");
    num = temp.substr(pos);
    stream.str(num);
    while(stream >> n){
        critical_nets.push_back(n);
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
        cout<<i;
    }
    cout<<endl;
    cout<<"Power = ";
    for(auto i:power_nets){
        cout<<i;
    }
    cout<<endl;
    cout<<"Ground = ";
    for(auto i:ground_nets){
        cout<<i;
    }
    cout<<endl;
}
// ./a.out (config filename)
int main(int argc ,char *argv[]){
    readconfig config;

    if(argc != 2){
        cerr<<"Use ./a.out (config filename)\n";
        return -1;
    }
    config.read_file(argv[1]);
    config.dump();
    return 0;
}