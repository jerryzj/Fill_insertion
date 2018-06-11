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
        cerr<<"Can't open rule file\n";
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

readprocess::readprocess(){
    window_size = 0;
}

string readprocess::key_gen(int x,int y, table_type type){
    string lateral_key("*");
    string area_key("*");
    string fringe_key("*");

    if(y == 0){
        cerr<<"The range of the second parameter is 1~9\n";
        exit(-1);
    }
    if(x > 9 || x < 0){
        cerr<<"Parameter out of range\n";
        exit(-1);
    }
    if(y > 9 || y < 1){
        cerr<<"Parameter out of range\n";
        exit(-1);
    }
    // handle diagonal lateral cases
    if (x == y){
        lateral_key.assign("lateral_table_"+to_string(x));
    }
    // handle area and fringe cases
    else{
        if(x == 0){
            area_key.assign("area_table_"+to_string(y)+"_"+to_string(x));
        }
        else if(x == 1){
            area_key.assign("area_table_"+to_string(x)+"_"+to_string(y));
            fringe_key.assign("fringe_table_"+to_string(x)+"_"+to_string(y));
            
        }
        else if (x > y){
            area_key.assign("area_table_"+to_string(y)+"_"+to_string(x));
            fringe_key.assign("fringe_table_"+to_string(x)+"_"+to_string(y));
        }
        else{
            area_key.assign("area_table_"+to_string(x)+"_"+to_string(y));
            fringe_key.assign("fringe_table_"+to_string(x)+"_"+to_string(y));
        }
    }
    switch(type){
        case area: return area_key; break;
        case lateral: return lateral_key; break;
        case fringe: return fringe_key; break;
    }
}

void readprocess::read_file(char* filename){
    ifstream file(filename);
    string temp;
    double n = 0;
    double alpha = 0;
    double beta = 0;
    size_t pos;

    // lateral table only has 9 
    // fringe has 9*9 - 9 = 72
    if(!file){  // check file exist or not
        cerr<<"Can't open process file\n";
        exit(-1);
    }
    // 1. read first commented line and ignore it
    getline(file, temp);
    // 2. read window size
    getline(file, temp);
    pos = temp.find(' ');
    temp = temp.substr(++pos);
    window_size = stoi(temp);
    // 3. read two commented lines
    getline(file, temp);
    getline(file, temp);
    // 4. skip table (use key_gen to reconstruct it)
    for(int i = 0; i < 12; i++){
        getline(file, temp);
    }
    // 5. skip table definitions
    getline(file, temp);
    if(temp[0] != ';') cerr<<"This line is not commented!\n";
    getline(file, temp);
    if(temp[0] != ';') cerr<<"This line is not commented!\n";    
    getline(file, temp);        
    if(temp[0] != ';') cerr<<"This line is not commented!\n";
    getline(file, temp);        
    // 6. read tables
    // 6-1 read table name
    while(getline(file, temp)){
        if(temp[0] == ';'){ // ignore commented line
            continue;
        }
        else if(temp.length() == 0){ // ignore blank line
            continue;
        }
        else{   // read table
            table t;
            pos = temp.find(' ');
            t.name = temp.substr(++pos);
            //debug
            //cout<<t.name<<endl;
            // 6-2 read commented line
            //ex: ; unit area cap table
            getline(file, temp);
            if(temp[0] != ';') cerr<<"This line is expected to be commented!\n";
            // 6-3 read range
            getline(file, temp);
            //cout<<temp<<endl;
            stringstream stream(temp);
            while(stream >> n){
                t.ranges.push_back((int)n);
            }
            // 6-4 read parameters
            getline(file, temp);
            istringstream ss(temp);
            string token;    
            while(getline(ss, token, ')')){
                //cout<<token<<endl;
                pos = token.find('(');
                token = token.substr(++pos);
                //cout<<token<<endl;
                pos = token.find(',');
                alpha = stod(token.substr(0, pos));
                //cout<<alpha<<endl;
                beta = stod(token.substr(++pos));
                //cout<<beta<<endl;
                t.alpha.push_back(alpha);
                t.beta.push_back(beta);
            }
            // 6-5 insert table t to map
            cap_table.insert(pair<string, table>(t.name,t));
            // 6-6 read blank line
            getline(file, temp);
        }
        temp.clear();
    }
    // 7. close file 
    file.close();
}

void readprocess::dump(){
    map<string, table>::iterator iter;    
    int counter = 0;
    for(iter = cap_table.begin(); iter != cap_table.end();iter++){
        ++counter;
        cout<<"Map key :"<<iter->first<<endl;
        cout<<"Map value :"<<endl;
        cout<<"  name : "<<iter->second.name<<endl;
        cout<<"  ranges"<<"\n  ";
        for(auto i : iter->second.ranges){
            cout<<i<<" ";
        }
        cout<<endl;
        cout<<"  alpha"<<"\n  ";
        for(auto i : iter->second.alpha){
            cout<<i<<" ";
        }
        cout<<endl;
        cout<<"  beta"<<"\n  ";
        for(auto i : iter->second.beta){
            cout<<i<<" ";
        }
        cout<<endl;
    }
    cout<<"Number of table : "<<counter<<endl;
}

double readprocess::find_area(int x, int y, int area){
    double alpha = 0;
    double beta = 0;
    double c_init = 0;
    double ans = 0;
    string key;
    map<string,table>::iterator iter;
    
    // generate key
    if(x == 0){
        key.assign("area_table_"+to_string(y)+"_"+to_string(x));
    }
    else if(x == 1){
        key.assign("area_table_"+to_string(x)+"_"+to_string(y));
    }
    else if (x > y){
        key.assign("area_table_"+to_string(y)+"_"+to_string(x));
    }
    else{
        key.assign("area_table_"+to_string(x)+"_"+to_string(y));
    }
    //debug
    //cout<<key<<endl;

    iter = cap_table.find(key);
    if(iter == cap_table.end()){
        cerr<<"Can't find this table in cap_table\n";
        cerr<<"Key :"<<key<<endl;
        exit(-1);
    }
    else{
        if(key != iter->second.name){
            cerr<<"Error! Key mismatches table name\n";
            exit(-1);
        }
        if(area >= iter->second.ranges.back()){
            alpha = iter->second.alpha.back();
            beta  = iter->second.beta.back();
            c_init = (alpha * iter->second.ranges.back()) + beta;
            ans = c_init * ((double)area / iter->second.ranges.back());
        }
        else if(area <= iter->second.ranges.front()){
            alpha = iter->second.alpha.front();
            beta  = iter->second.beta.front();
            c_init = (alpha * iter->second.ranges.front()) + beta;
            ans = c_init * ((double)area / iter->second.ranges.front());
        }
        else{
            for(int i = 0; i < iter->second.ranges.size() - 1; i++){
                if(iter->second.ranges[i] <= area && iter->second.ranges[i+1] > area){
                    alpha = iter->second.alpha[i];
                    beta  = iter->second.beta[i];
                    break; 
                }
            }
            c_init = (alpha * area) + beta;
            ans = c_init * area;
        }
    }
    // debug
    //cout<<"Alpha = "<<alpha<<endl;
    //cout<<"Beta = "<<beta<<endl;
    //cout<<"C-init = "<<c_init<<endl;
    return ans;
}

double readprocess::find_lateral(int x, int distance, int parallel_edge){
    double alpha = 0;
    double beta = 0;
    double c_init = 0;
    double ans = 0;
    string key;
    map<string,table>::iterator iter;

    // generate key
    key.assign("lateral_table_"+to_string(x));
    // search map
    iter = cap_table.find(key);
    if(iter == cap_table.end()){
        cerr<<"Can't find this table in cap_table\n";
        cerr<<"Key :"<<key<<endl;
        exit(-1);
    }
    else{
        if(distance < iter->second.ranges.front() || distance > iter->second.ranges.back()){
            return 0;
        }
        else{
            for(int i = 0; i < iter->second.ranges.size() - 1; i++){
                if(iter->second.ranges[i] <= distance && iter->second.ranges[i+1] >= distance){
                    alpha = iter->second.alpha[i];
                    beta  = iter->second.beta[i]; 
                    break;
                }
            }
            c_init = (alpha * distance) + beta;
            ans = c_init * parallel_edge;
        }
    }
    // debug
    //cout<<"Alpha = "<<alpha<<endl;
    //cout<<"Beta = "<<beta<<endl;
    //cout<<"C-init = "<<c_init<<endl;
    return ans;
}

double readprocess::find_fringe(int x, int y, int distance, int parallel_edge){
    double alpha = 0;
    double beta = 0;
    double c_init = 0;
    double ans = 0;
    string key;
    map<string,table>::iterator iter;
    
    // generate key; 
    key.assign("fringe_table_"+to_string(x)+"_"+to_string(y));
    // search map using key
    iter = cap_table.find(key);
    if(iter == cap_table.end()){
        cerr<<"Can't find this table in cap_table\n";
        cerr<<"Key :"<<key<<endl;
        exit(-1);
    }
    else{
        if(distance < iter->second.ranges.front() || distance > iter->second.ranges.back()){
            ans += 0;
        }
        else{
            for(int i = 0; i < iter->second.ranges.size() - 1; i++){
                if(iter->second.ranges[i] <= distance && iter->second.ranges[i+1] > distance){
                    alpha = iter->second.alpha[i];
                    beta  = iter->second.beta[i]; 
                    break;
                }
            }
            c_init = (alpha * distance) + beta;
            ans += c_init * parallel_edge;
        }
    }
    // debug
    cout<<"Alpha = "<<alpha<<endl;
    cout<<"Beta = "<<beta<<endl;
    cout<<"C-init = "<<c_init<<endl;

    //search map using another key
    key.assign("fringe_table_"+to_string(y)+"_"+to_string(x));
    iter = cap_table.find(key);
    if(iter == cap_table.end()){
        cerr<<"Can't find this table in cap_table\n";
        cerr<<"Key :"<<key<<endl;
        exit(-1);
    }
    else{
        if(distance < iter->second.ranges.front() || distance > iter->second.ranges.back()){
            ans += 0;
        }
        else{
            for(int i = 0; i < iter->second.ranges.size() - 1; i++){
                if(iter->second.ranges[i] <= distance && iter->second.ranges[i+1] > distance){
                    alpha = iter->second.alpha[i];
                    beta  = iter->second.beta[i]; 
                    break;
                }
            }
            c_init = (alpha * distance) + beta;
            ans += c_init * parallel_edge;
        }
    }
    // debug
    //cout<<"Alpha = "<<alpha<<endl;
    //cout<<"Beta = "<<beta<<endl;
    //cout<<"C-init = "<<c_init<<endl;    
    return ans;
}