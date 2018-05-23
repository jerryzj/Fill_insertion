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

readlayout::readlayout() {
    net_list.reserve(4E5);
}

void readlayout::read_file(char* filename){
    // here filename should be ./circuit#/circut*.cut
    ifstream file(filename);
    string temp;
    //string line;
    size_t pos;
    net net_temp;

    // Poly 0: Read layout boundary
    getline(file,temp);
    pos = temp.find(";");
    temp = temp.substr(0, pos);

    net_temp.poly_id = 0;
    sscanf(temp.c_str(),"%d %d %d %d",
            &net_temp.rect.bl_x, &net_temp.rect.bl_y, 
            &net_temp.rect.tr_x, &net_temp.rect.tr_y);
    net_temp.net_id  = 0;
    net_temp.layer   = 0;
    net_temp.type    = Bound;
    
    // set offset 
    offset_x = net_temp.rect.bl_x;
    offset_y = net_temp.rect.bl_y;
    // normalize boundary position
    net_temp.rect.bl_x -= offset_x;
    net_temp.rect.bl_y -= offset_y;
    net_temp.rect.tr_x -= offset_x;
    net_temp.rect.tr_y -= offset_y;


    net_list.push_back(net_temp);

    char c_str[20];
    while(getline(file,temp)){
        sscanf(temp.c_str(),"%d %d %d %d %d %d %d %s",
            &net_temp.poly_id, 
            &net_temp.rect.bl_x, &net_temp.rect.bl_y,
            &net_temp.rect.tr_x, &net_temp.rect.tr_y,
            &net_temp.net_id,    &net_temp.layer,
            c_str );

        // normalize net position 
        net_temp.rect.bl_x -= offset_x;
        net_temp.rect.bl_y -= offset_y;
        net_temp.rect.tr_x -= offset_x;
        net_temp.rect.tr_y -= offset_y;

        if (strcmp(c_str,"normal") == 0) {
            net_temp.type = Normal;
        }
        else {
            net_temp.type = NA;
        }
        net_list.push_back(net_temp);
    }

    file.close();
}

void readlayout::dump(){
    cout<<"----------------------\n";
    cout<<"       Layout file\n";
    cout<<"----------------------\n";  

    for (auto v: net_list) {
        cout << v.poly_id << " " 
            << v.rect.bl_x << " " 
            << v.rect.bl_y << " "
            << v.rect.tr_x << " "
            << v.rect.tr_y << " "
            << v.net_id << " "
            << v.layer << endl; 

    }
}