#include "layout.hpp"
#include "statistic.hpp"

Layout::Layout()
{
    normal_list.reserve(3E5);
    fill_list.reserve(3E5);
    // pos 0 is empty
    min_density.reserve(10);

    min_width.reserve(10);

    max_fill_width.reserve(10);

    min_space.reserve(10);
}




void Layout::read_file(char *filename)
{
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

void Layout::set_bin_size(int size)
{
    bin_size = size;
}

void Layout::create3Dbin()
{
    // since tr_x or tr_y / (bin_size = 5000) is always integer
    // the range right
    int range_x = normal_list[0].rect.tr_x / bin_size;
    int range_y = normal_list[0].rect.tr_y / bin_size;

    grid = new bin **[10];
    // 0606 layer 0 is used to store ground
    for (int i = 0; i <= 9; i++)   
    {
        grid[i] = new bin *[range_x];
        for (int j = 0; j < range_x; j++)
        {
            grid[i][j] = new bin[range_y];
        }
    }
}

void Layout::bin_mapping()
{
    // 5/24 revise bin assignment
    // bl, tr = (0, 5000), (0, 5000) will only be
    // assigned to grid[layer][0][0]
    
    // 0606 normal index 0 (layout boundary) will be assigned 
    // to all bins in layer 0  
    for (int i = 0; i < (int)normal_list.size(); i++)
        assign_normal(i);
}


// stable_sort fill list and remove deleted fills
void Layout::fill_sort() {
    // remove deleted nets (net_id = -1)
    fill_list.erase(
        remove_if(fill_list.begin(), fill_list.end(),
                    [](const net & o) {return (o.net_id == -1);}), 
                    fill_list.end()
    );
    // do sorting on bl_x
    stable_sort(fill_list.begin(), fill_list.end(),
          [](const net& lhs, const net& rhs) {
                return lhs.rect.bl_x < rhs.rect.bl_x;
            });
    
    // do sorting on layer
    stable_sort(fill_list.begin(), fill_list.end(),
          [](const net& lhs, const net& rhs) {
                return lhs.layer < rhs.layer;
            });





}

void Layout::fill_remapping() {

    // clean fill area and bin assignment
    int range_x = normal_list[0].rect.tr_x / bin_size;
    int range_y = normal_list[0].rect.tr_y / bin_size;    

    for (int layer = 0; layer <=9; layer++) {
        for (int x = 0; x < range_x; x++) {
            for (int y = 0; y < range_y; y++) {
                grid[layer][x][y].fill_area = 0;
                grid[layer][x][y].fill->clear();
            }
        }
    }

    for (int i = 0; i < (int)fill_list.size(); i++)
        assign_fill(i);    

}

// assign normal with index = i to bins
void Layout::assign_normal(int i)
{
    int layer;
    Rectangle temp_rect;
    Rectangle bin_index;

    layer = normal_list[i].layer;
    bin_index.bl_x = normal_list[i].rect.bl_x / bin_size;
    bin_index.bl_y = normal_list[i].rect.bl_y / bin_size;
    bin_index.tr_x = (normal_list[i].rect.tr_x - 1) / bin_size;
    bin_index.tr_y = (normal_list[i].rect.tr_y - 1) / bin_size;

    for (int j = bin_index.bl_x; j <= bin_index.tr_x; j++)
        for (int k = bin_index.bl_y; k <= bin_index.tr_y; k++)
        {
            grid[layer][j][k].normal->push_back(i);
            temp_rect.set_rectangle(j * bin_size, k * bin_size,
                                    (j + 1) * bin_size, (k + 1) * bin_size);
            grid[layer][j][k].normal_area +=
                area_overlap(normal_list[i].rect, temp_rect);
        }
}

// assign fill with index = i to bins
void Layout::assign_fill(int i)
{
    int layer;
    Rectangle temp_rect;
    Rectangle bin_index;

    layer = fill_list[i].layer;
    bin_index.bl_x = fill_list[i].rect.bl_x / bin_size;
    bin_index.bl_y = fill_list[i].rect.bl_y / bin_size;
    bin_index.tr_x = (fill_list[i].rect.tr_x - 1) / bin_size;
    bin_index.tr_y = (fill_list[i].rect.tr_y - 1) / bin_size;

    for (int j = bin_index.bl_x; j <= bin_index.tr_x; j++)
        for (int k = bin_index.bl_y; k <= bin_index.tr_y; k++)
        {
            grid[layer][j][k].fill->push_back(i);
            temp_rect.set_rectangle(j * bin_size, k * bin_size,
                                    (j + 1) * bin_size, (k + 1) * bin_size);
            grid[layer][j][k].fill_area +=
                area_overlap(fill_list[i].rect, temp_rect);
        }
}

// delete fill with index = i from bins 
// also set that fill net_id = -1
void Layout::delete_fill(int i)
{
    int layer;
    Rectangle temp_rect;
    Rectangle bin_index;

    layer = fill_list[i].layer;
    bin_index.bl_x = fill_list[i].rect.bl_x / bin_size;
    bin_index.bl_y = fill_list[i].rect.bl_y / bin_size;
    bin_index.tr_x = (fill_list[i].rect.tr_x - 1) / bin_size;
    bin_index.tr_y = (fill_list[i].rect.tr_y - 1) / bin_size;

    for (int j = bin_index.bl_x; j <= bin_index.tr_x; j++) {
        for (int k = bin_index.bl_y; k <= bin_index.tr_y; k++) {
            grid[layer][j][k].fill->erase(
                remove(grid[layer][j][k].fill->begin(), 
                        grid[layer][j][k].fill->end(), i), 
                        grid[layer][j][k].fill->end());

            temp_rect.set_rectangle(j * bin_size, k * bin_size,
                                    (j + 1) * bin_size, (k + 1) * bin_size);
            grid[layer][j][k].fill_area -=
                area_overlap(fill_list[i].rect, temp_rect);
        }
    }

    // set net_id = -1 to mark deletion
    fill_list[i].net_id = -1;
}

// fill index and the destinate rectangle
void Layout::resize_fill(int i, const Rectangle& r_new)
{
    int layer;
    int area_org;
    int area_new;

    Rectangle temp_rect;
    Rectangle bin_index;

    layer = fill_list[i].layer;
    bin_index.bl_x = fill_list[i].rect.bl_x / bin_size;
    bin_index.bl_y = fill_list[i].rect.bl_y / bin_size;
    bin_index.tr_x = (fill_list[i].rect.tr_x - 1) / bin_size;
    bin_index.tr_y = (fill_list[i].rect.tr_y - 1) / bin_size;    


    for (int j = bin_index.bl_x; j <= bin_index.tr_x; j++) {
        for (int k = bin_index.bl_y; k <= bin_index.tr_y; k++) {

            temp_rect.set_rectangle(j * bin_size, k * bin_size,
                                    (j + 1) * bin_size, (k + 1) * bin_size);

            area_org = area_overlap(fill_list[i].rect, temp_rect);
            area_new = area_overlap(r_new, temp_rect);

            grid[layer][j][k].fill_area += area_new - area_org;
        }
    }

    fill_list[i].rect = r_new;
}

void Layout::dump(string mode)
{
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
// untest, assign normal_area of a particular bin at grid[_l][_x][_y]
void Layout::bin_normal_area(int _l, int _x, int _y)
{
    int temp_area;
    Rectangle bin_rect(_x * bin_size, _y * bin_size,
                       (_x + 1) * bin_size, (_y + 1) * bin_size);
    // calculate normal area

    temp_area = 0;
    for (auto i : *(grid[_l][_x][_y].normal))
    {
        temp_area += area_overlap(bin_rect, normal_list[i].rect);
    }

    grid[_l][_x][_y].normal_area = temp_area;
}

void Layout::set_rules(const vector<rule> &_rules)
{

    // reserve vector[0] (dummy)
    min_density.push_back(0);
    min_width.push_back(0);
    max_fill_width.push_back(0);
    min_space.push_back(0);

    // push rule from rule 1 to 9
    for (auto r : _rules)
    {
        min_density.push_back(r.min_density);
        min_width.push_back(r.min_width);
        max_fill_width.push_back(r.max_fill_width);
        min_space.push_back(r.min_space);
    }
}



void Layout::dump_statistic()
{
    double density;
    vector<double> data;

    int range_x = normal_list[0].rect.tr_x / bin_size;
    int range_y = normal_list[0].rect.tr_y / bin_size;

    cout << "----------------------------" << endl;
    cout << "Dump Statistics" << endl;
    for (int layer = 0; layer <= 9; layer++) {
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
        cout << "Stdev = " << getStdDev(data) << endl;
    }

}





/***************************************************************/
// window_based_density_check also check if metal fill or normal fill is zero
// overlap area to the window as well
// 0605 add
bool Layout::one_window_density_check(int layer, int i, int j, int s)
{
    Rectangle window_rect;

    // record normal index in each window
    vector<int> normal_idx;
    // record fill index in each window
    vector<int> fill_idx;

    // sum of normal area in each window
    int normal_area_window = 0;
    // sum of fill area in each window
    int fill_area_window = 0;

    // sum of normal area in each window, calculated by sum of normal area in fours bin
    int normal_area_bin = 0;
    // sum of fill area in each window, calculated by sum of fill area in fours bin
    int fill_area_bin = 0;

    // sum of normal and fill area
    int metal_area = 0;

    double window_density;

    // For each window, Calculate Window boundary
    window_rect.set_rectangle(i * bin_size, j * bin_size, (i + 2) * bin_size, (j + 2) * bin_size);

    for (int i1 = 0; i1 < s; i1++)
    {
        for (int j1 = 0; j1 < s; j1++)
        {
            for (auto idx : *(grid[layer][i + i1][j + j1].normal))
            {
                normal_idx.push_back(idx);
            }
        }
    }

    // erase dumplicate normal that overlap with more than one bin
    stable_sort(normal_idx.begin(), normal_idx.end());
    normal_idx.erase(unique(normal_idx.begin(), normal_idx.end()), normal_idx.end());

    // sum area of normal overlapped with window
    for (auto idx : normal_idx)
    {
        normal_area_window += area_overlap(window_rect, normal_list[idx].rect);
    }

    for (int i1 = 0; i1 < s; i1++)
    {
        for (int j1 = 0; j1 < s; j1++)
        {
            normal_area_bin += grid[layer][i + i1][j + j1].normal_area;
        }
    }

    // Verification: Does area of normal overlapped with window
    // equal to sum of area of normal in four bins within the window?
    if (normal_area_bin != normal_area_window) {
        cout << "normal area verification fail" << endl;
    }

    for (int i1 = 0; i1 < s; i1++)
    {
        for (int j1 = 0; j1 < s; j1++)
        {
            for (auto idx : *(grid[layer][i + i1][j + j1].fill))
            {
                fill_idx.push_back(idx);
            }
        }
    }

    // erase dumplicate fill that overlap with more than one bin
    stable_sort(fill_idx.begin(), fill_idx.end());
    fill_idx.erase(unique(fill_idx.begin(), fill_idx.end()), fill_idx.end());

    // sum area of fill overlapped with window
    for (auto idx : fill_idx)
    {
        fill_area_window += area_overlap(window_rect, fill_list[idx].rect);
    }

    for (int i1 = 0; i1 < s; i1++)
    {
        for (int j1 = 0; j1 < s; j1++)
        {
            fill_area_bin += grid[layer][i + i1][j + j1].fill_area;
        }
    }

    // Verification: Does area of fill overlapped with window
    // equal to sum of area of fill in four bins within the window?
    if (fill_area_bin == fill_area_window)
    {
        //cout << "fill area verification pass" << endl;
    }
    else
    {
        cout << "fill area verification fail" << endl;
        //cout << "fill area fail " << layer << " " << i << " " << j << endl;
        //cout << "window area = " << fill_area_window << "bin area = " << fill_area_bin;
    }

    // Check whether window density > min density and window density > max density
    metal_area = fill_area_window + normal_area_window;
    window_density = ((double)(metal_area) / (double)(bin_size * bin_size * 4));

    if (window_density < min_density[layer])
    {
        cout << "fail window_density: " << layer
             << "_" << i << "_" << j << " " << window_density << endl;
        return 0;
    }
    else
    {
        if (window_density > 1)
        {
            cout << "density larger than max" << endl;
            return 0;
        }
        return 1;
    }

    fill_idx.clear();
    normal_idx.clear();
}

// 0605 modify
void Layout::window_based_density_check()
{
    int range_x = normal_list[0].rect.tr_x / bin_size;
    int range_y = normal_list[0].rect.tr_y / bin_size;
    bool density_pass;
    int density_check_fail_count = 0;
    int density_check_pass_count = 0;

    cout << "//==== Window based density check ===//" << endl;
    for (int layer = 1; layer <= 9; layer++)
    {
        for (int i = 0; i < range_x - 1; i++)
        {
            for (int j = 0; j < range_y - 1; j++)
            {
                density_pass = one_window_density_check(layer, i, j);
                if (density_pass)
                {
                    density_check_pass_count++;
                }
                else
                {
                    density_check_fail_count++;
                }
            }
        }
    }

    cout << "density pass count " << density_check_pass_count << endl;
    cout << "density fail count " << density_check_fail_count << endl;
    cout << "pass = " << (double)(density_check_pass_count) / double(density_check_pass_count + density_check_fail_count) << endl;
}

// check min_width, max_fill_width for all fills in fill_list
/***********************************************/
void Layout::DRC_check_width()
{
    cout << "//=== start DRC check ===//" << endl;

    int width_check_fail_count = 0;
    int layer;

    cout << "//=== DRC: check min_width and max_fill_width of fill ===//" << endl;
    for (int i = 0; i < fill_list.size(); i++)
    {
        layer = fill_list[i].layer;

        if (fill_list[i].rect.check_width(min_width[layer], 
                                max_fill_width[layer]) == false) {
            width_check_fail_count++;
            cout << i << " " << layer << " ";
            fill_list[i].rect.dump();
        }                            
    }
    cout << "total fill: " << fill_list.size() << endl;
    cout << "width check fail " << width_check_fail_count << endl;
    cout << "----------------------------" << endl;
}



bool Layout::one_net_DRC_check_space(const Layout::net& _net)
{
    // return value of check space value
    bool check_space_pass;
    
    // store index of fill in each window
    vector<int> fill_idx;

    // store index of normal in each window
    vector<int> normal_idx;

    Rectangle temp_rect;
    Rectangle bin_index;

    int layer = _net.layer;
    bin_index.bl_x = _net.rect.bl_x / bin_size;
    bin_index.bl_y = _net.rect.bl_y / bin_size;
    bin_index.tr_x = (_net.rect.tr_x - 1) / bin_size;
    bin_index.tr_y = (_net.rect.tr_y - 1) / bin_size;

    fill_idx.clear();
    normal_idx.clear();

    for (int x = bin_index.bl_x; x <= bin_index.tr_x; x++) {
        for (int y = bin_index.bl_y; y <= bin_index.tr_y; y++) {
            for (auto idx : *(grid[layer][x][y].fill))        
                fill_idx.push_back(idx);      
            for (auto idx : *(grid[layer][x][y].normal))
                normal_idx.push_back(idx);
        }
    }

    // erase duplicate fill index 
    stable_sort(fill_idx.begin(), fill_idx.end());
    fill_idx.erase(unique(fill_idx.begin(), fill_idx.end()), fill_idx.end());


    // check min space bte new rectangle and normal 
    for (auto n: normal_idx) {
        check_space_pass = check_space(_net.rect, normal_list[n].rect, min_space[layer]);
        if (!check_space_pass) {
            return false;
        }
    }

    // check min space bte new rectangle and fill 
    for (auto f: fill_idx) {
        check_space_pass = check_space(_net.rect, fill_list[f].rect, min_space[layer]);
        if (!check_space_pass) {
            return false;
        }
    }

    return true;
}


// check min_space btw fill & normal, fill & fill
/***********************************************/
bool Layout::one_window_DRC_check_space(int layer, int i, int j, int s)
{
    // return value of check space value
    bool check_space_pass;

    // store index of fill and normal
    //int a, b;

    // store index of fill in each window
    vector<int> fill_idx;

    // store index of normal in each window
    vector<int> normal_idx;

    fill_idx.clear();
    normal_idx.clear();

    // Read the index of all fill overlap with window
    for (int i1 = 0; i1 < s; i1++)
    {
        for (int j1 = 0; j1 < s; j1++)
        {
            for (auto idx : *(grid[layer][i+i1][j+j1].fill))
            {
                fill_idx.push_back(idx);
            }
        }
    }

    stable_sort(fill_idx.begin(), fill_idx.end());
    fill_idx.erase(unique(fill_idx.begin(), fill_idx.end()), fill_idx.end());

    for (auto n: fill_idx) {
        check_space_pass = one_net_DRC_check_space(fill_list[n]);
        if (check_space_pass == false) {
            cout << n <<" fill space voilate" << endl;    
            return false;
        }
    }


    return true;
    /*
    // Read the index of all normal overlap with window
    for (int i1 = 0; i1 < s; i1++)
    {
        for (int j1 = 0; j1 < s; j1++)
        {
            for (auto idx : *(grid[layer][i+i1][j+j1].normal))
            {
                normal_idx.push_back(idx);
            }
        }
    }

    // check min_space btw fill & normal
    for (int fill_x = 0; fill_x < fill_idx.size(); fill_x++)
    {

        a = fill_idx[fill_x];
        for (int normal_y = 0; normal_y < normal_idx.size(); normal_y++)
        {
            b = normal_idx[normal_y];
            //cout << "min space = " << min_space[layer] << endl;
            check_space_pass = check_space(fill_list[a].rect, normal_list[b].rect, min_space[layer]);
            if (!check_space_pass)
            {
                cout << " fill and normal check space voilate" << endl;
                return 0;
            }
        }
    }
    // check min_space btw fill & fill
    for (int fill_x = 0; fill_x < fill_idx.size(); fill_x++)
    {
        a = fill_idx[fill_x];
        for (int fill_y = fill_x + 1; fill_y < fill_idx.size(); fill_y++)
        {
            b = fill_idx[fill_y];
            check_space_pass = check_space(fill_list[a].rect, fill_list[b].rect, min_space[layer]);
            if (!check_space_pass)
            {
                //cout << " fill and fill check space voilate" << endl;
                cout << "num" << layer << " " << i << " " << j << " ";
                cout << "this two violate min sapce = " << min_space[layer] << endl;
                fill_list[fill_x].rect.dump();
                fill_list[fill_y].rect.dump();

                return 0;
            }
        }
    }*/

}

bool Layout::DRC_check_space()
{
    bool one_pass;
    bool all_pass;
    int range_x = normal_list[0].rect.tr_x / bin_size;
    int range_y = normal_list[0].rect.tr_y / bin_size;

    all_pass = 1;
    cout << "//=== DRC check space ===//" << endl;
    for (int layer = 1; layer <= 9; layer++)
    {
        for (int i = 0; i < range_x - 1; i++)
        {
            for (int j = 0; j < range_y - 1; j++)
            {
                one_pass = one_window_DRC_check_space(layer, i, j);
                if (!one_pass)
                {
                    all_pass = 0;
                }
            }
        }
    }

    if (all_pass == 0)
        cout << "DRC check space fail" << endl;
    else
        cout << "DRC check space pass" << endl;

    return all_pass;
}




void Layout::dump_fill_list()
{
    cout << "dump fill list" << endl;
    for (auto v : fill_list)
    {
        cout << v.rect.bl_x << " "
             << v.rect.bl_y << " "
             << v.rect.tr_x << " "
             << v.rect.tr_y << " "
             << v.net_id << " "
             << v.layer << endl;
    }
}

// dump bin into two files
// Inputs: layer index, and bin index x and y
void Layout::dump_bin(int layer, int x, int y)
{
    fstream density_file;
    fstream normal_file;
    fstream fill_file;
    string temp;

    Rectangle bound(x * bin_size, y * bin_size,
                    (x + 1) * bin_size, (y + 1) * bin_size);

    double density = ((double)grid[layer][x][y].normal_area +
                      (double)grid[layer][x][y].fill_area) /
                     (bin_size * bin_size);

    string filename("density.txt");
    density_file.open(filename.c_str(), ios::out);
    if (!density_file)
    {
        cerr << "Error create density file\n";
        exit(-1);
    }
    temp.assign(to_string(density) + "\n");
    density_file.write(temp.c_str(), temp.length());
    density_file.close();

    filename.assign("normal.cut");
    normal_file.open(filename.c_str(), ios::out);
    if (!normal_file)
    {
        cerr << "Error create bin_normal file\n";
        exit(-1);
    }

    temp.assign(bound.dump_string() + "; chip boundary\n");

    // write chip boundary to normal file
    normal_file.write(temp.c_str(), temp.length());
    // write normal poly info to normal_file
    for (auto i : *(grid[layer][x][y].normal))
    {
        Rectangle temp = normal_list[i].rect;
        temp = rect_overlap(temp, bound);

        string s(temp.dump_string() + " " +
                 to_string(normal_list[i].net_id) + " " +
                 "normal\n");
        normal_file.write(s.c_str(), s.length());
    }
    normal_file.close();

    filename.assign("fill.cut");
    fill_file.open(filename.c_str(), ios::out);
    if (!fill_file)
    {
        cerr << "Error create bin_fill file\n";
        exit(-1);
    }
    temp.assign(bound.dump_string() + "; chip boundary\n");

    // write chip boundary to fill file
    fill_file.write(temp.c_str(), temp.length());
    for (auto i : *(grid[layer][x][y].fill))
    {
        Rectangle temp = fill_list[i].rect;
        temp = rect_overlap(temp, bound);

        string s(temp.dump_string() + " " +
                 to_string(fill_list[i].net_id) + " " +
                 "fill\n");
        fill_file.write(s.c_str(), s.length());
    }
    fill_file.close();
}

// fill insertion algorition, use find fill region and metal fill
void Layout::fill_insertion()
{
    int range_x = normal_list[0].rect.tr_x / bin_size;
    int range_y = normal_list[0].rect.tr_y / bin_size;
    vector<Rectangle> fill_regions;

    // for each bin
    for (int layer = 1; layer <= 9; layer++) // 5/29 modified
    {
        //cout << "layer " << layer << endl;
        for (int i = 0; i < range_x; i++)
        {
            for (int j = 0; j < range_y; j++)
            {
                // if layer = 1,3,5,7,9 insert fill by x (vertical)
                if (layer % 2 == 1)
                {
                    fill_regions = find_fill_region_x(layer, i, j);
                }
                // else, layer = 2,4,6,8 insert fill by y (horizontal)
                else
                {
                    fill_regions = find_fill_region_y(layer, i, j);
                }
                metal_fill(layer, fill_regions);
            }
        }
    }

    // 6/4 test  window based fill on layer 9
    
    cout << "Window Based Fill Start" << endl;
    int layer = 9;
    for (int i = 0; i < range_x - 1; i++)
    {
        for (int j = 0; j < range_y - 1; j++)
        {
            fill_regions = find_fill_region_x(layer, i, j, 2);
            metal_fill(layer, fill_regions);

            double density = ((double)grid[layer][i][j].normal_area +
                              (double)grid[layer][i][j].fill_area) /
                             (bin_size * bin_size);

            if (density <= min_density[layer])
                cout << layer << " " << i << " " << j << ": " << density << endl;
        }
    }

    // 6/6 test random based fill on layer 9

    cout << "Random Fill Start" << endl;
    layer = 9;
    for (int i = 0; i < range_x; i++)
    {
        for (int j = 0; j < range_y; j++)
        {
            random_fill(layer, i, j, 1);

            double density = ((double)grid[layer][i][j].normal_area +
                              (double)grid[layer][i][j].fill_area) /
                             (bin_size * bin_size);

            if (density <= min_density[layer])
                cout << layer << " " << i << " " << j << ": " << density << endl;
        }
    }

}

void Layout::metal_fill(int layer, const vector<Rectangle> &fill_regions)
{

    Rectangle temp;
    net net_temp;

    int fill_width, fill_length;
    int fill_width_ratio, fill_length_ratio;

    vector<int> fill_bl_x;
    fill_bl_x.reserve(20);
    vector<int> fill_bl_y;
    fill_bl_y.reserve(20);
    vector<int> fill_tr_x;
    fill_tr_x.reserve(20);
    vector<int> fill_tr_y;
    fill_tr_y.reserve(20);

    int width_left, length_left;

    for (auto r : fill_regions)
    {
        fill_bl_x.clear();
        fill_bl_y.clear();
        fill_tr_x.clear();
        fill_tr_y.clear();

        temp.bl_x = r.bl_x + min_space[layer];
        temp.bl_y = r.bl_y + min_space[layer];
        temp.tr_x = r.tr_x - min_space[layer];
        temp.tr_y = r.tr_y - min_space[layer];

        fill_width = temp.tr_x - temp.bl_x;
        fill_length = temp.tr_y - temp.bl_y;
        fill_width_ratio = fill_width / max_fill_width[layer];
        fill_length_ratio = fill_length / max_fill_width[layer];

        if (fill_width >= min_width[layer] && fill_length >= min_width[layer])
        {
            // if width ratio > 0, means width > max_width, cut width into 1300-130 = 1170
            if (fill_width_ratio > 0)
            {
                for (int a = 0; a < fill_width_ratio; a++)
                {
                    fill_bl_x.push_back(temp.bl_x + (a * max_fill_width[layer]));
                    fill_tr_x.push_back(temp.bl_x + (a * max_fill_width[layer]) +
                                        max_fill_width[layer] - min_space[layer]);
                }

                width_left = temp.tr_x - (temp.bl_x + (fill_width_ratio * max_fill_width[layer]));
                if (width_left > min_width[layer])
                {
                    fill_bl_x.push_back(temp.bl_x + (fill_width_ratio * max_fill_width[layer]));
                    fill_tr_x.push_back(temp.tr_x);
                    fill_width_ratio++; // use as loop index later
                }
            }
            else
            {
                fill_bl_x.push_back(temp.bl_x);
                fill_tr_x.push_back(temp.tr_x);
                fill_width_ratio = 1; // for loop used
            }

            // if length ratio > 0, means length > max_width, cut length into 1300-130 = 1170
            if (fill_length_ratio > 0)
            {
                for (int a = 0; a < fill_length_ratio; a++)
                {
                    fill_bl_y.push_back(temp.bl_y + (a * max_fill_width[layer]));
                    fill_tr_y.push_back(temp.bl_y + (a * max_fill_width[layer]) +
                                        max_fill_width[layer] - min_space[layer]);
                }
                length_left = temp.tr_y - (temp.bl_y + (fill_length_ratio * max_fill_width[layer]));
                if (length_left > min_width[layer])
                {
                    fill_bl_y.push_back(temp.bl_y + (fill_length_ratio * max_fill_width[layer]));
                    fill_tr_y.push_back(temp.tr_y);
                    fill_length_ratio++; // use as loop index later
                }
            }
            else
            {
                fill_bl_y.push_back(temp.bl_y);
                fill_tr_y.push_back(temp.tr_y);
                fill_length_ratio = 1; // for loop used
            }

            //push fill to fill_list and update density
            for (int a = 0; a < fill_width_ratio; a++)
            {
                for (int b = 0; b < fill_length_ratio; b++)
                {
                    net_temp.layer = layer;
                    net_temp.net_id = 0;
                    net_temp.rect.set_rectangle(fill_bl_x[a], fill_bl_y[b],
                                                fill_tr_x[a], fill_tr_y[b]);
                    fill_list.push_back(net_temp);

                    // assign fill with ID = metal_fill_count to bin (s)
                    // to multiple bins if possible
                    // assign_fill increase fill area as well
                    assign_fill(metal_fill_count);
                    metal_fill_count++;
                }
            }
        }
    }
}

// 6/4 if s = 1, bin based find fill is performed
// if s = 2, window based find fill is performed
vector<Rectangle> Layout::find_fill_region_x(int layer, int i, int j, int s)
{
    Rectangle temp;
    net net_temp;
    Rectangle bin_rect;

    vector<Rectangle> poly_bin_instersect;
    poly_bin_instersect.reserve(10);

    vector<int> intersect_x;
    vector<int> intersect_y;
    intersect_x.reserve(10);
    intersect_y.reserve(10);

    bool not_poly;

    vector<Rectangle> no_merge_list;
    vector<Rectangle> fill_regions;

    poly_bin_instersect.clear();
    intersect_x.clear();
    intersect_y.clear();
    no_merge_list.clear();

    // 6/4 add s if window rect is needed
    bin_rect.set_rectangle(i * bin_size, j * bin_size, (i + s) * bin_size, (j + s) * bin_size);

    // 6/4 change poly_bin_instersect for window based
    for (int i1 = 0; i1 < s; i1++)
        for (int j1 = 0; j1 < s; j1++)
        {
            for (auto poly : *(grid[layer][i + i1][j + j1].normal))
            {
                temp = rect_overlap(normal_list[poly].rect, bin_rect);
                poly_bin_instersect.push_back(temp);
            }
            for (auto poly : *(grid[layer][i + i1][j + j1].fill))
            {
                temp = rect_overlap(fill_list[poly].rect, bin_rect);
                poly_bin_instersect.push_back(temp);
            }
        }

    // store x point in intersect_x
    for (auto x : poly_bin_instersect)
    {
        intersect_x.push_back(x.bl_x);
        intersect_x.push_back(x.tr_x);
    }

    // add bin boundary as intersection point
    // when no normal in bin
    if (intersect_x.size() == 0)
    {
        intersect_x.push_back(bin_rect.bl_x);
        intersect_x.push_back(bin_rect.tr_x);
    }

    // add bin boundary as intersection
    // when normal did not overlap with bin boundary
    else
    {
        if (intersect_x[0] > bin_rect.bl_x)
            intersect_x.insert(intersect_x.begin(), bin_rect.bl_x);
        if (intersect_x.back() < bin_rect.tr_x)
            intersect_x.push_back(bin_rect.tr_x);
    }

    // stable_sort and erase duplicate x
    stable_sort(intersect_x.begin(), intersect_x.end());
    intersect_x.erase(unique(intersect_x.begin(), intersect_x.end()), intersect_x.end());

    //cout << "//=== Set initialize fill region ===// " << endl;
    net_temp.net_id = 0;
    net_temp.layer = layer;

    for (int in_x = 0; in_x < intersect_x.size() - 1; in_x++)
    {
        // Given x , find intersection point y
        for (auto v : poly_bin_instersect)
        {
            // find poly intersect with line x = intersect[in_x] and x = intersect[in_x+1]
            // add y to intersecty when line x = intersect_x cross normal
            if ((intersect_x[in_x] >= v.bl_x && intersect_x[in_x] <= v.tr_x) ||
                (intersect_x[in_x + 1] >= v.bl_x && intersect_x[in_x + 1] <= v.tr_x))
            {
                // store intersection y point
                intersect_y.push_back(v.tr_y);
                intersect_y.push_back(v.bl_y);
            }
        }

        // if no poly in this region, set bin as intersection point
        if (intersect_y.size() == 0)
        {
            intersect_y.push_back(bin_rect.tr_y);
            intersect_y.push_back(bin_rect.bl_y);
        }
        // add bin boundary as intersection
        // when normal did not overlap with bin boundary
        else
        {
            if (intersect_y[0] > bin_rect.bl_y)
                intersect_y.insert(intersect_y.begin(), bin_rect.bl_y);
            if (intersect_y.back() < bin_rect.tr_y)
                intersect_y.push_back(bin_rect.tr_y);
        }
        // stable_sort and erase duplicate y
        stable_sort(intersect_y.begin(), intersect_y.end());
        intersect_y.erase(unique(intersect_y.begin(), intersect_y.end()), intersect_y.end());

        for (int in_y = 0; in_y < intersect_y.size() - 1; in_y++)
        {
            not_poly = 1;
            // for intersection points, filter out normal poly
            for (auto v : poly_bin_instersect)
            {
                if ((intersect_x[in_x] >= v.bl_x && intersect_x[in_x] <= v.tr_x) &&
                    (intersect_x[in_x + 1] >= v.bl_x && intersect_x[in_x + 1] <= v.tr_x))
                {
                    // intersection point = normal bl and tr
                    if ((intersect_y[in_y] == v.bl_y) && (intersect_y[in_y + 1] == v.tr_y))
                    {
                        not_poly = 0;
                        //cout << "intersection point = normal bl and tr " << endl;
                    }

                    // intersection in normal poly, filter out overlap normal poly
                    if ((intersect_y[in_y] > v.bl_y) && (intersect_y[in_y] < v.tr_y))
                    {
                        not_poly = 0;
                        //cout << " y intersection in normal poly, filter out overlap normal poly" << endl;
                    }

                    // intersection in normal poly, filter out overlap normal poly
                    if ((intersect_y[in_y + 1] > v.bl_y) && (intersect_y[in_y + 1] < v.tr_y))
                    {
                        not_poly = 0;
                        //cout << "y + 1 intersection in normal poly, filter out overlap normal poly" << endl;
                    }
                }
            }

            if (not_poly)
            {
                temp.bl_x = intersect_x[in_x];
                temp.bl_y = intersect_y[in_y];
                temp.tr_x = intersect_x[in_x + 1];
                temp.tr_y = intersect_y[in_y + 1];

                //init_fill_list.push_back(net_temp);
                no_merge_list.push_back(temp);
                //grid[layer][i][j].init_fill->push_back(init_fill_count);
                //init_fill_count++;
            }
        }
        intersect_y.clear();
    }

    // merge init_fill_list based on x

    vector<bool> not_merge;
    // for each retangle in no_merge_list, not_merge = 1
    for (int c = 0; c < no_merge_list.size(); c++)
    {
        not_merge.push_back(1);
    }

    // Given bl_y and tr_y, store all x with the same bl_y and tr_y
    // so that we can merge later
    vector<int> before_merge_x_list;
    vector<int> after_merge_x_list;
    vector<Rectangle> merge_x_rect_list;
    net_temp.net_id = 0;
    net_temp.layer = layer;

    for (int c = 0; c < no_merge_list.size(); c++)
    {
        // if this rectangle did not merge yet
        before_merge_x_list.clear();
        after_merge_x_list.clear();

        if (not_merge[c])
        {
            // set not_merge[c] = 0
            not_merge[c] = 0;

            before_merge_x_list.push_back(no_merge_list[c].bl_x);
            before_merge_x_list.push_back(no_merge_list[c].tr_x);

            // find rectangle with the same bl_y and t_y
            for (int d = c + 1; d < no_merge_list.size(); d++)
            {
                // if no_merge_list[d] and no_merge_list[c] have same bl_y and tr_y
                if ((no_merge_list[d].bl_y == no_merge_list[c].bl_y) &&
                    (no_merge_list[d].tr_y == no_merge_list[c].tr_y))
                {
                    // set not_merge[d] = 0
                    not_merge[d] = 0;

                    // add x point into merge_x_list to merge later
                    before_merge_x_list.push_back(no_merge_list[d].bl_x);
                    before_merge_x_list.push_back(no_merge_list[d].tr_x);
                }
            }

            // for no_merge_list[c], merge with other rect that has same y
            // merge x point
            stable_sort(before_merge_x_list.begin(), before_merge_x_list.end());

            for (int d = 0; d < before_merge_x_list.size() - 1;)
            {
                //cout << "hihi 1" << endl;
                // if not duplicate, push into after_merge_x_list
                if (before_merge_x_list[d] != before_merge_x_list[d + 1])
                {
                    after_merge_x_list.push_back(before_merge_x_list[d]);
                    d++;
                }
                // if before_merge_x_list[d] == before_merge_x_list[d+1], skip this two points
                else
                {
                    d += 2;
                }
            }
            // add last element into afte_merge_list
            after_merge_x_list.push_back(before_merge_x_list.back());

            // push merge rectangle into init_fill_list
            for (int d = 0; d < after_merge_x_list.size() - 1; d += 2)
            {
                net_temp.rect.bl_x = after_merge_x_list[d];
                net_temp.rect.tr_x = after_merge_x_list[d + 1];
                net_temp.rect.bl_y = no_merge_list[c].bl_y;
                net_temp.rect.tr_y = no_merge_list[c].tr_y;
                //init_fill_list.push_back(net_temp);
                //grid[layer][i][j].init_fill->push_back(init_fill_count);
                //init_fill_count++;
                fill_regions.push_back(net_temp.rect);
            }
        }
    }
    poly_bin_instersect.clear();
    intersect_x.clear();
    intersect_y.clear();
    no_merge_list.clear();
    before_merge_x_list.clear();
    after_merge_x_list.clear();

    return fill_regions;
}

// 6/4 if s = 1, bin based find fill is performed
// if s = 2, window based find fill is performed
vector<Rectangle> Layout::find_fill_region_y(int layer, int i, int j, int s)
{
    Rectangle temp;
    net net_temp;
    Rectangle bin_rect;

    vector<Rectangle> poly_bin_instersect;
    poly_bin_instersect.reserve(10);

    vector<int> intersect_x;
    vector<int> intersect_y;
    intersect_x.reserve(10);
    intersect_y.reserve(10);

    bool not_poly;

    vector<Rectangle> no_merge_list;
    vector<Rectangle> fill_regions;

    poly_bin_instersect.clear();
    intersect_x.clear();
    intersect_y.clear();
    no_merge_list.clear();

    // 6/4 add s if window rect is needed
    bin_rect.set_rectangle(i * bin_size, j * bin_size, (i + s) * bin_size, (j + s) * bin_size);

    // 6/4 change poly_bin_instersect for window based
    for (int i1 = 0; i1 < s; i1++)
        for (int j1 = 0; j1 < s; j1++)
        {
            for (auto poly : *(grid[layer][i + i1][j + j1].normal))
            {
                temp = rect_overlap(normal_list[poly].rect, bin_rect);
                poly_bin_instersect.push_back(temp);
            }
            for (auto poly : *(grid[layer][i + i1][j + j1].fill))
            {
                temp = rect_overlap(fill_list[poly].rect, bin_rect);
                poly_bin_instersect.push_back(temp);
            }
        }

    // store y point in intersect_y
    for (auto y : poly_bin_instersect)
    {
        intersect_y.push_back(y.bl_y);
        intersect_y.push_back(y.tr_y);
    }

    // add bin boundary as intersection point
    // when no normal in bin
    if (intersect_y.size() == 0)
    {
        intersect_y.push_back(bin_rect.bl_y);
        intersect_y.push_back(bin_rect.tr_y);
    }

    // add bin boundary as intersection
    // when normal did not overlap with bin boundary
    else
    {
        if (intersect_y[0] > bin_rect.bl_y)
            intersect_y.insert(intersect_y.begin(), bin_rect.bl_y);
        if (intersect_y.back() < bin_rect.tr_y)
            intersect_y.push_back(bin_rect.tr_y);
    }

    // stable_sort and erase duplicate y
    stable_sort(intersect_y.begin(), intersect_y.end());
    intersect_y.erase(unique(intersect_y.begin(), intersect_y.end()), intersect_y.end());

    //cout << "//=== Set initialize fill region ===// " << endl;
    net_temp.net_id = 0;
    net_temp.layer = layer;

    for (int in_y = 0; in_y < intersect_y.size() - 1; in_y++)
    {
        // Given y , find intersection point x
        for (auto v : poly_bin_instersect)
        {
            // find poly intersect with line y = intersect[in_y] and y = intersect[in_y+1]
            // add x to intersecty when line y = intersect_y cross normal
            if ((intersect_y[in_y] >= v.bl_y && intersect_y[in_y] <= v.tr_y) ||
                (intersect_y[in_y + 1] >= v.bl_y && intersect_y[in_y + 1] <= v.tr_y))
            {
                // store intersection y point
                intersect_x.push_back(v.tr_x);
                intersect_x.push_back(v.bl_x);
            }
        }

        // if no poly in this region, set bin as intersection point
        if (intersect_x.size() == 0)
        {
            intersect_x.push_back(bin_rect.tr_x);
            intersect_x.push_back(bin_rect.bl_x);
        }
        // add bin boundary as intersection
        // when normal did not overlap with bin boundary
        else
        {
            if (intersect_x[0] > bin_rect.bl_x)
                intersect_x.insert(intersect_x.begin(), bin_rect.bl_x);
            if (intersect_x.back() < bin_rect.tr_x)
                intersect_x.push_back(bin_rect.tr_x);
        }
        // stable_sort and erase duplicate x
        stable_sort(intersect_x.begin(), intersect_x.end());
        intersect_x.erase(unique(intersect_x.begin(), intersect_x.end()), intersect_x.end());

        for (int in_x = 0; in_x < intersect_x.size() - 1; in_x++)
        {
            not_poly = 1;
            // for intersection points, filter out normal poly
            for (auto v : poly_bin_instersect)
            {
                if ((intersect_y[in_y] >= v.bl_y && intersect_y[in_y] <= v.tr_y) &&
                    (intersect_y[in_y + 1] >= v.bl_y && intersect_y[in_y + 1] <= v.tr_y))
                {
                    // intersection point = normal bl and tr
                    if ((intersect_x[in_x] == v.bl_x) && (intersect_x[in_x + 1] == v.tr_x))
                    {
                        not_poly = 0;
                        //cout << "intersection point = normal bl and tr " << endl;
                    }

                    // intersection in normal poly, filter out overlap normal poly
                    if ((intersect_x[in_x] > v.bl_x) && (intersect_x[in_x] < v.tr_x))
                    {
                        not_poly = 0;
                        //cout << " x intersection in normal poly, filter out overlap normal poly" << endl;
                    }

                    // intersection in normal poly, filter out overlap normal poly
                    if ((intersect_x[in_x + 1] > v.bl_x) && (intersect_x[in_x + 1] < v.tr_x))
                    {
                        not_poly = 0;
                        //cout << "x + 1 intersection in normal poly, filter out overlap normal poly" << endl;
                    }
                }
            }

            if (not_poly)
            {
                temp.bl_x = intersect_x[in_x];
                temp.bl_y = intersect_y[in_y];
                temp.tr_x = intersect_x[in_x + 1];
                temp.tr_y = intersect_y[in_y + 1];

                no_merge_list.push_back(temp);
            }
        }
        intersect_x.clear();
    }

    // merge y point init_fill_list

    vector<bool> not_merge;

    // for each retangle in no_merge_list, not_merge = 1
    for (int c = 0; c < no_merge_list.size(); c++)
    {
        not_merge.push_back(1);
    }

    // Given bl_y and tr_y, store all x with the same bl_x and tr_x
    // so that we can merge later
    vector<int> before_merge_y_list;
    vector<int> after_merge_y_list;
    net_temp.net_id = 0;
    net_temp.layer = layer;

    for (int c = 0; c < no_merge_list.size(); c++)
    {
        // if this rectangle did not merge yet
        before_merge_y_list.clear();
        after_merge_y_list.clear();

        if (not_merge[c])
        {
            // set not_merge[c] = 0
            not_merge[c] = 0;

            before_merge_y_list.push_back(no_merge_list[c].bl_y);
            before_merge_y_list.push_back(no_merge_list[c].tr_y);

            // find rectangle with the same bl_y and t_y
            for (int d = c + 1; d < no_merge_list.size(); d++)
            {
                // if no_merge_list[d] and no_merge_list[c] have same bl_y and tr_y
                if ((no_merge_list[d].bl_x == no_merge_list[c].bl_x) &&
                    (no_merge_list[d].tr_x == no_merge_list[c].tr_x))
                {
                    // set not_merge[d] = 0
                    not_merge[d] = 0;

                    // add x point into merge_x_list to merge later
                    before_merge_y_list.push_back(no_merge_list[d].bl_y);
                    before_merge_y_list.push_back(no_merge_list[d].tr_y);
                }
            }

            // for no_merge_list[c], merge with other rect that has same y
            // merge x point
            stable_sort(before_merge_y_list.begin(), before_merge_y_list.end());

            for (int d = 0; d < before_merge_y_list.size() - 1;)
            {
                //cout << "hihi 1" << endl;
                // if not duplicate, push into after_merge_x_list
                if (before_merge_y_list[d] != before_merge_y_list[d + 1])
                {
                    after_merge_y_list.push_back(before_merge_y_list[d]);
                    d++;
                }
                // if before_merge_x_list[d] == before_merge_x_list[d+1], skip this two points
                else
                {
                    d += 2;
                }
            }
            // add last element into afte_merge_list
            after_merge_y_list.push_back(before_merge_y_list.back());

            // push merge rectangle into init_fill_list
            for (int d = 0; d < after_merge_y_list.size() - 1; d += 2)
            {
                net_temp.rect.bl_x = no_merge_list[c].bl_x;
                net_temp.rect.tr_x = no_merge_list[c].tr_x;
                net_temp.rect.bl_y = after_merge_y_list[d];
                net_temp.rect.tr_y = after_merge_y_list[d + 1];
                //init_fill_list.push_back(net_temp);
                //grid[layer][i][j].init_fill->push_back(init_fill_count);
                //init_fill_count++;
                fill_regions.push_back(net_temp.rect);
            }
        }
    }

    poly_bin_instersect.clear();
    intersect_x.clear();
    intersect_y.clear();
    no_merge_list.clear();
    before_merge_y_list.clear();
    after_merge_y_list.clear();

    return fill_regions;
}


// 6/6b new random fill
// int s: number of bins s = 1: bin based, s = 2 window based
// this function directly fill the possible filling regions 
// no metal call is needed after this function
void Layout::random_fill(int layer, int i, int j, int s)
{
    bool check;
    Rectangle bin_rect;
    bin_rect.set_rectangle(i * bin_size, j * bin_size, (i+s) * bin_size, (j+s) * bin_size);
    
    int step = 10; // 06/06 will add random seed to determin step

    net net_temp;
    net_temp.layer = layer;
    net_temp.net_id = 0;

    int x_bl_low = bin_rect.bl_x + min_width[layer];
    int x_bl_high = bin_rect.tr_x-2*min_width[layer];
    int y_bl_low = bin_rect.bl_y + min_width[layer];
    int y_bl_high = bin_rect.tr_y-2*min_width[layer];

    // iterate all possible bl_x and bl_y to fill in the bin_rect
    for (int x = x_bl_low; x <= x_bl_high; x = x + step) {
        for (int y = y_bl_low; y <= y_bl_high; y = y + step) {
            // create a temp net
            net_temp.rect.set_rectangle(x, y, x+min_width[layer], y+min_width[layer]);

            // check if it can fill in layout 
            check = one_net_DRC_check_space(net_temp);

            // cannot fill, give up random expand
            if (check == true ) {
                random_expand(net_temp, layer, i, j, s, step, "lf");
                random_expand(net_temp, layer, i, j, s, step, "rt");
                random_expand(net_temp, layer, i, j, s, step, "dw");
                random_expand(net_temp, layer, i, j, s, step, "up");
                
                fill_list.push_back(net_temp);
                assign_fill(metal_fill_count);
                metal_fill_count++;
            }
        }
    }   
}

void Layout::random_expand(Layout::net& _net, int layer, int i, int j, int s, int step, string mode)
{
    net net_expand;
    bool check_expand;

    Rectangle bin_rect( i * bin_size + min_width[layer], 
                        j * bin_size + min_width[layer], 
                        (i+s) * bin_size - min_width[layer], 
                        (j+s) * bin_size - min_width[layer]);

    net_expand = _net;
    check_expand = true;
    while (check_expand == true) {
        if (mode == "lf") net_expand.rect.bl_x -= step;
        if (mode == "rt") net_expand.rect.tr_x += step;
        if (mode == "dw") net_expand.rect.bl_y -= step;
        if (mode == "up") net_expand.rect.tr_y += step;
        
        // check spaceing 
        check_expand = one_net_DRC_check_space(net_expand);

        // check if inside bin        
        if (area_overlap(net_expand.rect, bin_rect) != net_expand.rect.area())
            check_expand = false;

        // check max width
        if (net_expand.rect.check_width(min_width[layer], 
                                        max_fill_width[layer]) == false)
            check_expand = false;

        if (check_expand == true) 
            _net = net_expand;
    }
}
