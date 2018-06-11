#include "layout.hpp"
#include "statistic.hpp"


// fill insertion algorition, use find fill region and metal fill
void Layout::fill_insertion(){
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

                    // assign fill with ID = fill.size -1 to bin (s)
                    // to multiple bins if possible
                    // assign_fill increase fill area as well
                    assign_fill(fill_list.size()-1);
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
                assign_fill(fill_list.size()-1);
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


double find_cost(const readprocess& process, Rectangle& _rec, int layer){
    int search_layer = 0;
    double cost;
    net temp = fill_list[fill_id];
    Rectangle search_bin;
    Rectangle temp_rec;


    search_bin.bl_x = _rect.bl_x / bin_size;
    search_bin.bl_y = _rect.bl_y / bin_size;
    search_bin.tr_x = (_rect.tr_x - 1) / bin_size;
    search_bin.tr_y = (_rect.tr_y - 1) / bin_size;

    if(temp.layer == 9){
        return 0;
    }
    else{
        for(int i = search_bin.bl_x; i < search_bin.tr_x; i++){
            for(int j = search_bin.bl_y; j < search_bin.tr_y; j++){
                for(auto k : grid[layer-1][i][j].fill){
                    temp_rec = fill_list[k]
                }
            }
        }
    }
}