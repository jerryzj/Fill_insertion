#include "parser.hpp"

readconfig::readconfig(){
    critical_nets.reserve(120);
    input.reserve(13);
    output.reserve(14);
    rule.reserve(9);
    process.reserve(12);
    power_nets.reserve(1);
    ground_nets.reserve(1);
}

void readconfig::read_file(char* filename){
    ifstream file(filename);
    string temp;
    string num;
    int n = 0;
    size_t pos;

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
    cout<<"----------------------\n";
    cout<<"     Config file\n";
    cout<<"----------------------\n";
    
    cout<<"Input = "<<input<<"\n";
    cout<<"Output = "<<output<<"\n";
    cout<<"Rule = "<<rule<<"\n";
    cout<<"Process = "<<process<<"\n";
    cout<<"Critical = ";
    for(auto i:critical_nets){
        cout<<i<<" ";
    }
    cout<<"\n";
    cout<<"Total Number of critical nets:"<<critical_nets.size()<<"\n";
    cout<<"Power = ";
    for(auto i:power_nets){
        cout<<i<<" ";
    }
    cout<<"\n";
    cout<<"Ground = ";
    for(auto i:ground_nets){
        cout<<i<<" ";
    }
    cout<<endl;
}

readrule::readrule(){
    rule_num = 0;
    rules.reserve(9);
}

void readrule::read_file(char* filename){
    // here filename should be ./circuit#/rule.dat
    ifstream file(filename);
    string temp;
    rule buf;
    char arr[15];

    if(!file){  // check file exist or not
        cerr<<"Can't open config file\n";
        exit(-1);
    }
    while(getline(file,temp)){
        if(temp.length() > 1){ // eliminate only \n lines
            ++rule_num;
            sscanf(temp.c_str(),"%d %s %d %d %d %lf %lf",
            &buf.layer_id, arr, &buf.min_width, &buf.min_space, 
            &buf.max_fill_width, &buf.min_density, &buf.max_density);
            buf.layer_type.assign(arr);
            rules.push_back(buf);
        }
    }
    file.close();
}

void readrule::dump(){
    cout<<"----------------------\n";
    cout<<"       Rule file\n";
    cout<<"     Total rules:"<<rule_num<<"\n";
    cout<<"----------------------\n";
    
    for(auto i : rules){
        cout<<i.layer_id<<" ";
        cout<<i.layer_type<<" ";
        cout<<i.min_width<<" ";
        cout<<i.min_space<<" ";
        cout<<i.max_fill_width<<" ";
        cout<<i.min_density<<" ";
        cout<<i.max_density<<endl;
    }
}