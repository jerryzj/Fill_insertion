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