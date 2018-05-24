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

    int _bl_x, _bl_y, _tr_x, _tr_y;
//    sscanf(temp.c_str(),"%d %d %d %d",
//           &net_temp.rect.bl_x, &net_temp.rect.bl_y, 
//            &net_temp.rect.tr_x, &net_temp.rect.tr_y);
//    net_temp.net_id  = 0;
//    net_temp.layer   = 0;
    sscanf(temp.c_str(),"%d %d %d %d", &_bl_x, &_bl_y, &_tr_x, &_tr_y);

    // set offset
    offset_x = _bl_x;
    offset_y = _bl_y;

    // normalize boundary position
    _bl_x -= offset_x;
    _bl_y -= offset_y;
    _tr_x -= offset_x;
    _tr_y -= offset_y;

    // set rectangle with normalized boundaries
    net_temp.rect.set_rectangle(_bl_x, _bl_y, _tr_x, _tr_y);
    net_temp.net_id  = 0;
    net_temp.layer   = 0;

    // insert to the head of vecetor
    net_list.push_back(net_temp);
    // read layout line by line
    while(getline(file,temp)){
        sscanf(temp.c_str(),"%d %d %d %d %d %d %d %s",
            &poly_id_temp, 
            &_bl_x, &_bl_y, &_tr_x, &_tr_y,
            &net_temp.net_id,    &net_temp.layer,
            c_str );
        // set normalize net position 
        _bl_x -= offset_x;
        _bl_y -= offset_y;
        _tr_x -= offset_x;
        _tr_y -= offset_y;
        net_temp.rect.set_rectangle(_bl_x, _bl_y, _tr_x, _tr_y);

        net_list.push_back(net_temp);
    }
    file.close();
}

void Layout::set_bin_size(int size){
    bin_size = size;
}

void Layout::create3Dbin(){
    int range_x = net_list[0].rect.tr_x / bin_size;
    int range_y = net_list[0].rect.tr_y / bin_size;
    
    grid = new bin**[10];
    for(int i = 1; i <= 9; i++){
        grid[i] = new bin * [range_x];
        for(int j = 0; j < range_x; j++){
            grid[i][j] = new bin[range_y];
        }
    }
}

void Layout::bin_mapping(){
    int layer = 0;
    Rectangle bin_index; 
    for(int i = 1; i < (int)net_list.size(); i++){
        layer = net_list[i].layer;
        bin_index.bl_x = net_list[i].rect.bl_x / bin_size;
        bin_index.bl_y = net_list[i].rect.bl_y / bin_size;
        bin_index.tr_x = net_list[i].rect.tr_x / bin_size;
        bin_index.tr_y = net_list[i].rect.tr_y / bin_size;
        for(int j = bin_index.bl_x ; j <= bin_index.tr_x; j++){
            for(int k = bin_index.bl_y; k <= bin_index.tr_y; k++){
                grid[layer][j][k].normal->push_back(i);
            }
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