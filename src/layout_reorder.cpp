#include "layout.hpp"

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