#include "layout.hpp"

Layout::Layout() {
    net_list.reserve(3E5); 
}

void Layout::read_file(char* filename){
    // here filename should be ./circuit#/circut*.cut
    ifstream file(filename);
    string temp;
    //string line;
    size_t pos;
    int poly_id_temp;
    char c_str[20];
    net net_temp;

    if(!file){  // check file exist or not
        cerr<<"Can't open layout file\n";
        exit(-1);
    }
    // Poly 0: Read layout boundary
    getline(file,temp);
    pos = temp.find(";");
    temp = temp.substr(0, pos);

    //net_temp.poly_id = 0;
    sscanf(temp.c_str(),"%d %d %d %d",
            &net_temp.rect.bl_x, &net_temp.rect.bl_y, 
            &net_temp.rect.tr_x, &net_temp.rect.tr_y);
    net_temp.net_id  = 0;
    net_temp.layer   = 0;
    
    // set offset 
    offset_x = net_temp.rect.bl_x;
    offset_y = net_temp.rect.bl_y;
    // normalize boundary position
    net_temp.rect.bl_x -= offset_x;
    net_temp.rect.bl_y -= offset_y;
    net_temp.rect.tr_x -= offset_x;
    net_temp.rect.tr_y -= offset_y;
    // insert to the head of vecetor
    net_list.push_back(net_temp);
    // read layout line by line
    while(getline(file,temp)){
        sscanf(temp.c_str(),"%d %d %d %d %d %d %d %s",
            &poly_id_temp, 
            &net_temp.rect.bl_x, &net_temp.rect.bl_y,
            &net_temp.rect.tr_x, &net_temp.rect.tr_y,
            &net_temp.net_id,    &net_temp.layer,
            c_str );
        // normalize net position 
        net_temp.rect.bl_x -= offset_x;
        net_temp.rect.bl_y -= offset_y;
        net_temp.rect.tr_x -= offset_x;
        net_temp.rect.tr_y -= offset_y;

        net_list.push_back(net_temp);
    }
    file.close();
}

void Layout::set_bin_size(int size){
    bin_size = size;
}

void Layout::create3Dmatrix(){
    int x_length = net_list[0].rect.tr_x / bin_size;
    int y_length = net_list[0].rect.tr_y / bin_size;
    
    grid = new bin**[10];
    for(int i = 1; i <= 9; i++){
        grid[i] = new bin * [x_length];
        for(int j = 0; j < x_length; j++){
            grid[i][j] = new bin[y_length];
        }
    }
}

void Layout::dump(){
    cout<<"----------------------\n";
    cout<<"     Layout file\n";
    cout<<"----------------------\n";  

    for (auto v: net_list) {
        cout << v.rect.bl_x << " " 
            << v.rect.bl_y << " "
            << v.rect.tr_x << " "
            << v.rect.tr_y << " "
            << v.net_id << " "
            << v.layer << endl; 
    }
}