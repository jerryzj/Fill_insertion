#include "layout.hpp"

void Layout::create3Dbin(){
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

void Layout::set_bin_size(int size){
    bin_size = size;
}

void Layout::bin_mapping(){
    // 5/24 revise bin assignment
    // bl, tr = (0, 5000), (0, 5000) will only be
    // assigned to grid[layer][0][0]
    
    // 0606 normal index 0 (layout boundary) will be assigned 
    // to all bins in layer 0  
    for (int i = 0; i < (int)normal_list.size(); i++)
        assign_normal(i);
}

// assign normal with index = i to bins
void Layout::assign_normal(int i){
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
void Layout::assign_fill(int i){
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
void Layout::delete_fill(int i){
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
void Layout::resize_fill(int i, const Rectangle& r_new){
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