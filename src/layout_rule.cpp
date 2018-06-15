#include "layout.hpp"

void Layout::set_rules(const vector<rule> &_rules){

    // reserve vector[0] (dummy)
    min_density.push_back(0);
    min_width.push_back(0);
    max_fill_width.push_back(0);
    min_space.push_back(0);

    // push rule from rule 1 to 9
    for (auto r : _rules){
        min_density.push_back(r.min_density);
        min_width.push_back(r.min_width);
        max_fill_width.push_back(r.max_fill_width);
        min_space.push_back(r.min_space);
    }
}

// check min_width, max_fill_width for all fills in fill_list
void Layout::DRC_check_width(){
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

bool Layout::DRC_check_space(){
    bool one_pass;
    bool all_pass;
    int range_x = normal_list[0].rect.tr_x / bin_size;
    int range_y = normal_list[0].rect.tr_y / bin_size;

    all_pass = 1;
    cout << "//=== DRC check space ===//" << endl;
    for (int layer = 1; layer <= 9; layer++){
        for (int i = 0; i < range_x - 1; i++){
            for (int j = 0; j < range_y - 1; j++){
                one_pass = one_window_DRC_check_space(layer, i, j);
                if (!one_pass){
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

// check min_space btw fill & normal, fill & fill
bool Layout::one_window_DRC_check_space(int layer, int i, int j, int s){
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
        check_space_pass = one_net_DRC_check_space(fill_list[n], n);
        if (check_space_pass == false) {
            cout << n <<" fill space voilate" << endl;    
            return false;
        }
    }


    return true;
}

bool Layout::one_net_DRC_check_space(const Layout::net& _net, int index){
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

    // remove same net id if not zero
    if (index != -1) {
        fill_idx.erase(
            remove(fill_idx.begin(), fill_idx.end(), index), 
                    fill_idx.end());
    }


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

// window_based_density_check also check if metal fill or normal fill is zero
// overlap area to the window as well
bool Layout::one_window_density_check(int layer, int i, int j, int s){
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

void Layout::window_based_density_check(){
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
