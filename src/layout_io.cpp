#include "layout.hpp"
#include "statistic.hpp"

void Layout::read_file(char *filename){
    // here filename should be ./circuit#/circut*.cut
    ifstream file(filename);
    string temp;
    //string line;
    size_t pos;
    int poly_id_temp;
    char c_str[20];
    net net_temp;

    if (!file)
    { // check file exist or not
        cerr << "Can't open layout file\n";
        exit(-1);
    }
    // Poly 0: Read layout boundary
    getline(file, temp);
    pos = temp.find(";");
    temp = temp.substr(0, pos);

    int _bl_x, _bl_y, _tr_x, _tr_y;
    sscanf(temp.c_str(), "%d %d %d %d", &_bl_x, &_bl_y, &_tr_x, &_tr_y);

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
    net_temp.net_id = 0;
    net_temp.layer = 0;

    // insert to the head of vecetor
    normal_list.push_back(net_temp);
    // read layout line by line
    while (getline(file, temp))
    {
        sscanf(temp.c_str(), "%d %d %d %d %d %d %d %s",
               &poly_id_temp,
               &_bl_x, &_bl_y, &_tr_x, &_tr_y,
               &net_temp.net_id, &net_temp.layer,
               c_str);
        // set normalize net position
        _bl_x -= offset_x;
        _bl_y -= offset_y;
        _tr_x -= offset_x;
        _tr_y -= offset_y;
        net_temp.rect.set_rectangle(_bl_x, _bl_y, _tr_x, _tr_y);

        normal_list.push_back(net_temp);
    }
    file.close();
}

void Layout::dump(string mode){
    cout << "----------------------\n";
    cout << "     Layout file\n";
    cout << "----------------------\n";

    if (mode == "normal" || mode == "all") {
        cout << "Normal List" << endl;
        for (auto v : normal_list)
            cout << v.rect.bl_x << " " << v.rect.bl_y << " "
                << v.rect.tr_x << " " << v.rect.tr_y << " "
                << v.net_id << " " << v.layer << endl;
    }

    if (mode == "fill"  || mode == "all") {
        cout << "Fill List" << endl;
        for (auto v : fill_list)
            cout << v.rect.bl_x << " " << v.rect.bl_y << " "
                << v.rect.tr_x << " " << v.rect.tr_y << " "
                << v.net_id << " " << v.layer << endl;
    }
}

void Layout::dump_statistic(){
    double density;
    vector<double> data;

    int range_x = normal_list[0].rect.tr_x / bin_size;
    int range_y = normal_list[0].rect.tr_y / bin_size;

    cout << "----------------------------" << endl;
    cout << "Dump Statistics" << endl;
    for (int layer = 1; layer <= 9; layer++) {
        data.clear();   // clean vector
        cout << "Layer " << layer << " ";
        for (int x = 0; x < range_x; x++) {
            for (int y = 0; y < range_y; y++) {
                density = ((double)grid[layer][x][y].normal_area +
                            (double)grid[layer][x][y].fill_area) /
                            (bin_size * bin_size);        
                data.push_back(density);
                //cout << density << endl;
            }
        }

        cout << "Mean = " << getMean(data) << " ";
        cout << "Stdev = " << getStdDev(data) << " ";
        cout << "Cost = "  << layer_cost(layer) << endl;
    }   
}
void Layout::find_cost_all(readprocess& process){
    for (int i = 0; i < fill_list.size(); i++){
        if (fill_list[i].net_id != -1) {
            fill_list[i].cost = find_cost(process, fill_list[i].rect, fill_list[i].layer);
        }        
    }
}


double Layout::layer_cost(int layer)
{
    // enumerate all fill with layer = layer, and net_id != -1 
    // all cost are assumed updated
    double total_cost = 0.0; 
    for (int i = 0; i < fill_list.size(); i++){
        if (fill_list[i].net_id != -1 && fill_list[i].layer == layer) {
            total_cost += fill_list[i].cost;
        }        
    }

    return total_cost;  
}

// dump bin into two files
// Inputs: layer index, and bin index x and y
void Layout::dump_bin(int layer, int x, int y){
    fstream density_file;
    fstream normal_file;
    fstream fill_file;
    string temp;

    Rectangle bound(x * bin_size, y * bin_size,
                    (x + 1) * bin_size, (y + 1) * bin_size);

    double density = ((double)grid[layer][x][y].normal_area +
                      (double)grid[layer][x][y].fill_area) /
                     (bin_size * bin_size);

    
    // Dump bin density file
    string filename("statistics.txt");
    density_file.open(filename.c_str(), ios::out);
    if (!density_file){
        cerr << "Error create density file\n";
        exit(-1);
    }
    temp.assign(to_string(density) + "\n");
    density_file.write(temp.c_str(), temp.length());
    for(auto i : critical_list){
        temp.assign(to_string(i) + "\n");
        density_file.write(temp.c_str(), temp.length());
    }
    density_file.close();
    // Dump normal list
    filename.assign("normal.cut");
    normal_file.open(filename.c_str(), ios::out);
    if (!normal_file){
        cerr << "Error create bin_normal file\n";
        exit(-1);
    }
    temp.assign(bound.dump_string() + "; chip boundary\n");
    // write chip boundary to normal file
    normal_file.write(temp.c_str(), temp.length());
    // write normal poly info to normal_file
    for (auto i : *(grid[layer][x][y].normal)){
        Rectangle temp = normal_list[i].rect;
        temp = rect_overlap(temp, bound);

        string s(temp.dump_string() + " " +
                 to_string(normal_list[i].net_id) + " " +
                 "normal\n");
        normal_file.write(s.c_str(), s.length());
    }
    normal_file.close();
    // Dump fill list
    filename.assign("fill.cut");
    fill_file.open(filename.c_str(), ios::out);
    if (!fill_file){
        cerr << "Error create bin_fill file\n";
        exit(-1);
    }
    temp.assign(bound.dump_string() + "; chip boundary\n");
    // write chip boundary to fill file
    fill_file.write(temp.c_str(), temp.length());
    for (auto i : *(grid[layer][x][y].fill)){
        Rectangle temp = fill_list[i].rect;
        temp = rect_overlap(temp, bound);
        string s(temp.dump_string() + " " +
                 to_string(fill_list[i].net_id) + " " +
                 "fill\n");
        fill_file.write(s.c_str(), s.length());
    }
    fill_file.close();
}


void Layout::dump_result(char* filename){
    fstream fill_file;
    string name, temp;
    int index = 1;

    Rectangle bound( 
        normal_list[0].rect.bl_x + offset_x,
        normal_list[0].rect.bl_y + offset_y,
        normal_list[0].rect.tr_x + offset_x,
        normal_list[0].rect.tr_y + offset_y);

    name.assign(filename);
    fill_file.open(name.c_str(), ios::out);
    temp.assign(bound.dump_string() + "; chip boundary\n");
    fill_file.write(temp.c_str(), temp.length());

    for (auto n : fill_list){
        Rectangle temp(
            n.rect.bl_x + offset_x,
            n.rect.bl_y + offset_y,
            n.rect.tr_x + offset_x,
            n.rect.tr_y + offset_y);
        string s(to_string(index++) + " " + temp.dump_string() + " " +
                 to_string(n.net_id) + " " + to_string(n.layer) + " " +
                 "fill\n");
        fill_file.write(s.c_str(), s.length());
    }

    fill_file.close();
}