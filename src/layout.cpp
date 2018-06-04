#include "layout.hpp"

Layout::Layout()
{
    normal_list.reserve(3E5);
    fill_list.reserve(3E5);
    init_fill_list.reserve(3E5);
    // pos 0 is empty
    min_density.reserve(10);
    min_density.push_back(0);

    min_width.reserve(10);
    min_width.push_back(0);

    max_fill_width.reserve(10);
    max_fill_width.push_back(0);

    min_space.reserve(10);
    min_space.push_back(0);
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
    for (int i = 1; i <= 9; i++)
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
    int layer = 0;

    Rectangle bin_index;
    for (int i = 1; i < (int)normal_list.size(); i++)
    {
        layer = normal_list[i].layer;
        bin_index.bl_x = normal_list[i].rect.bl_x / bin_size;
        bin_index.bl_y = normal_list[i].rect.bl_y / bin_size;
        bin_index.tr_x = (normal_list[i].rect.tr_x - 1) / bin_size;
        bin_index.tr_y = (normal_list[i].rect.tr_y - 1) / bin_size;
        for (int j = bin_index.bl_x; j <= bin_index.tr_x; j++)
        {
            for (int k = bin_index.bl_y; k <= bin_index.tr_y; k++)
            {
                grid[layer][j][k].normal->push_back(i);
            }
        }
    }
}

void Layout::dump()
{
    cout << "----------------------\n";
    cout << "     Layout file\n";
    cout << "----------------------\n";

    for (auto v : normal_list)
    {
        cout << v.rect.bl_x << " "
             << v.rect.bl_y << " "
             << v.rect.tr_x << " "
             << v.rect.tr_y << " "
             << v.net_id << " "
             << v.layer << endl;
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

/**** 5/29 Not necessary !!!!! ****/
void Layout::set_min_density(int layer, double density)
{
    //cout << "set min density = " << density << endl;
    min_density.push_back(density);
}

void Layout::set_min_width(int layer, int width)
{
    //cout << "set min width = " << width << endl;
    min_width.push_back(width);
}

void Layout::set_max_fill_width(int layer, int width)
{
    //cout << "set max width = " << width << endl;
    max_fill_width.push_back(width);
}

void Layout::set_min_space(int layer, int space)
{
    //cout << "set min space = " << space << endl;
    min_space.push_back(space);
}
/***********************************/


void Layout::metal_fill(int layer, int i, int j)
{

    Rectangle temp;
    net net_temp;

    double poly_density; // normal metal density
    double curr_density; // current bin density
    bool no_more_fill_region;
    int fill_region_used_count;
    int fill_index;
    int fill_width, fill_length;
    int fill_width_ratio, fill_length_ratio;

    vector<int> fill_bl_x;
    vector<int> fill_bl_y;
    vector<int> fill_tr_x;
    vector<int> fill_tr_y;
    fill_bl_x.reserve(20);
    fill_bl_y.reserve(20);
    fill_tr_x.reserve(20);
    fill_tr_y.reserve(20);

    int area_temp;
    int density_pass_count = 0;
    int density_fail_count = 0;
    int no_fill_count = 0;
    int fill_area_sum;

    int width_left, length_left;

    // Calculate dennsity
    bin_normal_area(layer, i, j);
    poly_density = (double)(grid[layer][i][j].normal_area / (double)(bin_size * bin_size));

    //Start fill insertion
    curr_density = poly_density;
    fill_region_used_count = 1;
    no_more_fill_region = (fill_region_used_count > grid[layer][i][j].init_fill->size());
    net_temp.layer = layer;
    net_temp.net_id = 0;
    fill_area_sum = 0;
    //while (curr_density <= min_density[layer] && (!no_more_fill_region)) 5/29
    while (!no_more_fill_region)
    {
        fill_bl_x.clear();
        fill_bl_y.clear();
        fill_tr_x.clear();
        fill_tr_y.clear();
        fill_index = grid[layer][i][j].init_fill->at(fill_region_used_count - 1);

        // for init fill region, shrink min_space
        temp.bl_x = init_fill_list[fill_index].rect.bl_x + min_space[layer];
        temp.bl_y = init_fill_list[fill_index].rect.bl_y + min_space[layer];
        temp.tr_x = init_fill_list[fill_index].rect.tr_x - min_space[layer];
        temp.tr_y = init_fill_list[fill_index].rect.tr_y - min_space[layer];
        fill_width = temp.tr_x - temp.bl_x;
        fill_length = temp.tr_y - temp.bl_y;
        fill_width_ratio = fill_width / max_fill_width[layer];
        fill_length_ratio = fill_length / max_fill_width[layer];

        //cout << "fill_w " << fill_width << " fill_L = " << fill_length << endl;
        // if width and length both larger than min_width, start fill
        if (fill_width >= min_width[layer] && fill_length >= min_width[layer])
        {
            // if width ratio > 0, means width > max_width, cut width into 1300-130 = 1170
            if (fill_width_ratio > 0)
            {
                for (int a = 0; a < fill_width_ratio; a++)
                {
                    fill_bl_x.push_back(temp.bl_x + (a * max_fill_width[layer]));
                    fill_tr_x.push_back(temp.bl_x + (a * max_fill_width[layer]) + max_fill_width[layer] - min_space[layer]);
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
                    fill_tr_y.push_back(temp.bl_y + (a * max_fill_width[layer]) + max_fill_width[layer] - min_space[layer]);
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
                    net_temp.rect.set_rectangle(fill_bl_x[a], fill_bl_y[b],
                                                fill_tr_x[a], fill_tr_y[b]);
                    fill_list.push_back(net_temp);
                    area_temp = net_temp.rect.area();
                    fill_area_sum += area_temp;
                    grid[layer][i][j].fill->push_back(metal_fill_count);
                    metal_fill_count++;
                }
            }
        }
        fill_region_used_count++;
        no_more_fill_region = (fill_region_used_count > grid[layer][i][j].init_fill->size());
    }

    curr_density += ((double)fill_area_sum / (double)(bin_size * bin_size));
    grid[layer][i][j].fill_area = fill_area_sum;
    // run time density check

    if (curr_density < min_density[layer])
    {
        //cout << "fail ";
        //cout << "Fail: " << layer << " " << i << " " << j << " " << endl;
        //cout << " total density " << curr_density << " " << endl;
        if (curr_density == poly_density)
        {
            //cout << "Fail: " << layer << " " << i << " " << j << " " << endl;
            //cout << "no fill, poly density = " << poly_density << endl;
        }
        else
        {
            //cout << endl;
        }
    }
}

/***************************************************************/
void Layout::window_based_density_check()
{
    // 5/30 version
    cout << "//==== Window based density check ===//" << endl;
    cout << "window size = " << bin_size * 2 << endl;
    int range_x = normal_list[0].rect.tr_x / bin_size;
    int range_y = normal_list[0].rect.tr_y / bin_size;

    Rectangle window_rect;

    // record normal index in each window
    vector<int> normal_idx;
    // sum of normal area in each window
    int normal_area_window;
    // sum of normal area in each window, calculated by sum of normal area in fours bin
    int normal_area_bin;

    int normal_area_ver_fail_count = 0;

    int metal_area;

    double window_density;

    // record fill index in each window
    vector<int> fill_idx;
    // sum of fill area in each window
    int fill_area_window;
    // sum of fill area in each window, calculated by sum of fill area in fours bin
    int fill_area_bin;

    int fill_area_ver_fail_count = 0;

    int density_check_fail_count = 0;
    int density_check_pass_count = 0;
    int density_check_larger_than_max_count = 0;

    for (int layer = 1; layer <= 9; layer++)
    //for (int layer = 2; layer <= 2; layer++)
    {
        for (int i = 0; i < range_x - 1; i++)
        //for (int i = 43; i < 44; i++)
        {
            for (int j = 0; j < range_y - 1; j++)
            //for (int j = 10; j < 11; j++)
            {
                // Initialize parameter for each window
                normal_area_window = 0;
                normal_area_bin = 0;
                fill_area_window = 0;
                fill_area_bin = 0;
                metal_area = 0;
                fill_idx.clear();
                normal_idx.clear();

                // For each window, Calculate Window boundary
                window_rect.set_rectangle(i * bin_size, j * bin_size, (i + 2) * bin_size, (j + 2) * bin_size);

                // normal area verification

                // Read the index of all normal overlap with window
                for (auto idx : *(grid[layer][i][j].normal))
                {
                    normal_idx.push_back(idx);
                }
                for (auto idx : *(grid[layer][i + 1][j].normal))
                {
                    normal_idx.push_back(idx);
                }
                for (auto idx : *(grid[layer][i][j + 1].normal))
                {
                    normal_idx.push_back(idx);
                }
                for (auto idx : *(grid[layer][i + 1][j + 1].normal))
                {
                    normal_idx.push_back(idx);
                }

                // erase dumplicate normal that overlap with more than one bin
                sort(normal_idx.begin(), normal_idx.end());
                normal_idx.erase(unique(normal_idx.begin(), normal_idx.end()), normal_idx.end());

                // sum area of normal overlapped with window
                for (auto idx : normal_idx)
                {
                    normal_area_window += area_overlap(window_rect, normal_list[idx].rect);
                }

                // sum area of normal in four bins within the window
                normal_area_bin += grid[layer][i][j].normal_area;
                normal_area_bin += grid[layer][i + 1][j].normal_area;
                normal_area_bin += grid[layer][i][j + 1].normal_area;
                normal_area_bin += grid[layer][i + 1][j + 1].normal_area;

                // Verification: Does area of normal overlapped with window
                // equal to sum of area of normal in four bins within the window?
                if (normal_area_bin != normal_area_window)
                {
                    //cout << "normal area verification fail" << endl;
                    normal_area_ver_fail_count++;
                }

                // Read the index of all fill overlap with window
                for (auto idx : *(grid[layer][i][j].fill))
                {
                    fill_idx.push_back(idx);
                }
                for (auto idx : *(grid[layer][i + 1][j].fill))
                {
                    fill_idx.push_back(idx);
                }
                for (auto idx : *(grid[layer][i][j + 1].fill))
                {
                    fill_idx.push_back(idx);
                }
                for (auto idx : *(grid[layer][i + 1][j + 1].fill))
                {
                    fill_idx.push_back(idx);
                }

                // erase dumplicate fill that overlap with more than one bin
                sort(fill_idx.begin(), fill_idx.end());
                fill_idx.erase(unique(fill_idx.begin(), fill_idx.end()), fill_idx.end());

                // sum area of fill overlapped with window
                for (auto idx : fill_idx)
                {
                    fill_area_window += area_overlap(window_rect, fill_list[idx].rect);
                }

                // sum area of fill in four bins within the window
                fill_area_bin += grid[layer][i][j].fill_area;
                fill_area_bin += grid[layer][i + 1][j].fill_area;
                fill_area_bin += grid[layer][i][j + 1].fill_area;
                fill_area_bin += grid[layer][i + 1][j + 1].fill_area;

                // Verification: Does area of fill overlapped with window
                // equal to sum of area of fill in four bins within the window?
                if (fill_area_bin != fill_area_window)
                {
                    //cout << "fill area verification fail" << endl;
                    fill_area_ver_fail_count++;
                }
                // Check whether window density > min density and window density > max density
                metal_area = fill_area_window + normal_area_window;
                window_density = ((double)(metal_area) / (double)(bin_size * bin_size * 4));
                if (window_density < min_density[layer])
                {
                    cout << "fail window_density: " << layer
                         << "_" << i << "_" << j << " " << window_density << endl;
                    density_check_fail_count++;
                }
                else
                {
                    density_check_pass_count++;
                    if (window_density > 1)
                    {
                        density_check_larger_than_max_count++;
                    }
                }
            }
        }
    }
    cout << "normal area verification fail : " << normal_area_ver_fail_count << endl;
    cout << "fill area verification fail : " << fill_area_ver_fail_count << endl;
    cout << "density pass count " << density_check_pass_count << endl;
    cout << "density fail count " << density_check_fail_count << endl;
    cout << "density larger than max density count " << density_check_larger_than_max_count << endl;
    cout << "pass = " << (double)(density_check_pass_count) / double(density_check_pass_count + density_check_fail_count) << endl;
}

// check min_width, max_fill_width for all fills in fill_list
/***********************************************/
void Layout::DRC_check_width()
{
    cout << "//=== start DRC check ===//" << endl;

    int min_width_fail_count = 0;
    int max_fill_width_fail_count = 0;
    int width_x;
    int length_y;
    int layer;

    cout << "//=== DRC: check min_width and max_fill_width of fill ===//" << endl;
    for (int i = 0; i < fill_list.size(); i++)
    {
        layer = fill_list[i].layer;
        width_x = fill_list[i].rect.tr_x - fill_list[i].rect.bl_x;
        length_y = fill_list[i].rect.tr_y - fill_list[i].rect.bl_y;

        if (width_x < min_width[layer] || length_y < min_width[layer])
        {
            min_width_fail_count++;
        }

        if (width_x > max_fill_width[layer] || length_y > max_fill_width[layer])
        {
            //cout << "width = " << width_x << " ";
            //cout << "length = " << length_y << endl;
            //cout << "max = " << max_fill_width[layer] << endl;
            max_fill_width_fail_count++;
        }
    }
    cout << "total fill: " << fill_list.size() << endl;
    cout << "min_width check fail " << min_width_fail_count << endl;
    cout << "max fill width check fail " << max_fill_width_fail_count << endl;
    cout << "----------------------------" << endl;
}

// check min_space btw fill & normal, fill & fill
/***********************************************/
void Layout::DRC_check_space()
{
    int fill_normal_fail_count = 0;
    int fill_fill_fail_count = 0;
    bool check_space_pass;
    int range_x = normal_list[0].rect.tr_x / bin_size;
    int range_y = normal_list[0].rect.tr_y / bin_size;
    int a, b; // loop index

    // 5/30 modify
    // store index of fill in each window
    vector<int> fill_idx;

    // store index of normal in each window
    vector<int> normal_idx;

    cout << "//=== check space between fill and normal ===//" << endl;
    for (int layer = 1; layer <= 9; layer++) // 5/29 modified
    {
        //cout << "layer " << layer << endl;
        for (int i = 0; i < range_x - 1; i++)
        {
            for (int j = 0; j < range_y - 1; j++)
            {
                fill_idx.clear();
                normal_idx.clear();
                // Read the index of all fill overlap with window
                for (auto idx : *(grid[layer][i][j].fill))
                {
                    fill_idx.push_back(idx);
                }
                for (auto idx : *(grid[layer][i + 1][j].fill))
                {
                    fill_idx.push_back(idx);
                }
                for (auto idx : *(grid[layer][i][j + 1].fill))
                {
                    fill_idx.push_back(idx);
                }
                for (auto idx : *(grid[layer][i + 1][j + 1].fill))
                {
                    fill_idx.push_back(idx);
                }

                // Read the index of all normal overlap with window
                for (auto idx : *(grid[layer][i][j].normal))
                {
                    normal_idx.push_back(idx);
                }
                for (auto idx : *(grid[layer][i + 1][j].normal))
                {
                    normal_idx.push_back(idx);
                }
                for (auto idx : *(grid[layer][i][j + 1].normal))
                {
                    normal_idx.push_back(idx);
                }
                for (auto idx : *(grid[layer][i + 1][j + 1].normal))
                {
                    normal_idx.push_back(idx);
                }

                for (int fill_x = 0; fill_x < fill_idx.size(); fill_x++)
                {

                    a = fill_idx[fill_x];
                    for (int normal_y = 0; normal_y < normal_idx.size(); normal_y++)
                    {
                        b = normal_idx[normal_y];
                        //cout << "min space = " << min_space[layer] << endl;
                        check_space_pass = check_space(fill_list[a].rect, normal_list[b].rect, min_space[layer]);
                        if (!check_space_pass)
                        { /*
                            cout << " (" << i << " " << j << " )" ;
                            if(layer == 4 && i == 47 && j == 21)
                            {
                                
                                cout << "this two violate min sapce = " << min_space[layer] << endl;
                                fill_list[a].rect.dump();
                                normal_list[b].rect.dump();
                                cout << "---------------------" << endl;
                            }
                            */
                            fill_normal_fail_count++;
                        }
                    }
                }
            }
        }
    }
    cout << "fill_normal_fail " << fill_normal_fail_count << endl;

    cout << "//=== check space between fill and fill ===//" << endl;
    for (int layer = 1; layer <= 9; layer++)
    {
        for (int i = 0; i < range_x - 1; i++)
        {
            for (int j = 0; j < range_y - 1; j++)
            {

                fill_idx.clear();
                // Read the index of all fill overlap with window
                for (auto idx : *(grid[layer][i][j].fill))
                {
                    fill_idx.push_back(idx);
                }
                for (auto idx : *(grid[layer][i + 1][j].fill))
                {
                    fill_idx.push_back(idx);
                }
                for (auto idx : *(grid[layer][i][j + 1].fill))
                {
                    fill_idx.push_back(idx);
                }
                for (auto idx : *(grid[layer][i + 1][j + 1].fill))
                {
                    fill_idx.push_back(idx);
                }

                for (int fill_x = 0; fill_x < fill_idx.size(); fill_x++)
                {
                    a = fill_idx[fill_x];
                    for (int fill_y = fill_x + 1; fill_y < fill_idx.size(); fill_y++)
                    {
                        b = fill_idx[fill_y];
                        //cout << "min space = " << min_space[layer] << endl;
                        check_space_pass = check_space(fill_list[a].rect, fill_list[b].rect, min_space[layer]);
                        if (!check_space_pass)
                        {
                            //cout << "num" << layer << " " << i << " " << j << " ";
                            //cout << "this two violate min sapce = " << min_space[layer] << endl;
                            //fill_list[fill_x].rect.dump();
                            //fill_list[fill_y].rect.dump();
                            fill_fill_fail_count++;
                            //cout << "---------------------" << endl;
                        }
                    }
                }
            }
        }
    }
    cout << "fill_fill_fail " << fill_fill_fail_count << endl;
    cout << "total fill " << fill_list.size() << endl;
    //dump_fill_list();
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
    int lower_bound_x = x * bin_size;
    int lower_bound_y = y * bin_size;
    int upper_bound_x = (x + 1) * bin_size;
    int upper_bound_y = (y + 1) * bin_size;

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
    temp.assign(to_string(lower_bound_x) + " " + to_string(lower_bound_y) + " " + to_string(upper_bound_x) + " " + to_string(upper_bound_y) +
                "; chip boundary\n");

    // write chip boundary to normal file
    normal_file.write(temp.c_str(), temp.length());
    // write normal poly info to normal_file
    for (auto i : *(grid[layer][x][y].normal))
    {
        Rectangle temp = normal_list[i].rect;
        if (temp.bl_x < lower_bound_x)
            temp.bl_x = lower_bound_x;
        if (temp.bl_x > upper_bound_x)
        {
            cerr << "Error! poly bl_x bigger than bin upper bound\n";
            exit(-1);
        }
        if (temp.tr_x > upper_bound_x)
        {
            temp.tr_x = upper_bound_x;
        }
        if (temp.tr_x < lower_bound_x)
        {
            cerr << "Error! poly tr_x smaller than bin lower bound\n";
            exit(-1);
        }
        if (temp.bl_y < lower_bound_y)
        {
            temp.bl_y = lower_bound_y;
        }
        if (temp.bl_y > upper_bound_y)
        {
            cerr << "Error! poly bl_y bigger than bin upper bound\n";
            exit(-1);
        }
        if (temp.tr_y > upper_bound_y)
        {
            temp.tr_y = upper_bound_y;
        }
        if (temp.tr_y < lower_bound_y)
        {
            cerr << "Error! poly tr_y smaller than bin lower bound\n";
            exit(-1);
        }
        string s(to_string(temp.bl_x) + " " +
                 to_string(temp.bl_y) + " " +
                 to_string(temp.tr_x) + " " +
                 to_string(temp.tr_y) + " " +
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
    temp.assign(to_string(lower_bound_x) + " " + to_string(lower_bound_y) + " " + to_string(upper_bound_x) + " " + to_string(upper_bound_y) +
                "; chip boundary\n");

    // write chip boundary to normal file
    fill_file.write(temp.c_str(), temp.length());
    for (auto i : *(grid[layer][x][y].fill))
    {
        Rectangle temp = fill_list[i].rect;
        string s(to_string(temp.bl_x) + " " +
                 to_string(temp.bl_y) + " " +
                 to_string(temp.tr_x) + " " +
                 to_string(temp.tr_y) + " " +
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
                    find_fill_region_x(layer, i, j);
                }
                // else, layer = 2,4,6,8 insert fill by y (horizontal)
                else
                {
                    find_fill_region_y(layer, i, j);
                }
                metal_fill(layer, i, j);
                // 3,3 is minimum requirement for not violate DRC
                //random_fill(layer, i, j, 3, 3);

                double density = ((double)grid[layer][i][j].normal_area +
                                    (double)grid[layer][i][j].fill_area) /
                                    (bin_size * bin_size);

                if (density <= 0.4)
                     cout << layer << " " << i << " " << j << ": " << density << endl;

                /*
                if (layer == 9 && (i == 8 || i == 9) && (j == 102 || j == 103 || j == 104 || j == 105))
                {
                    cout << layer << " " << i << " " << j << " random count: " << random_fill_count << endl;
                }
                */

                // add random fill to improve density for all pin
            }
        }
    }
}

void Layout::find_fill_region_x(int layer, int i, int j)
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

    poly_bin_instersect.clear();
    intersect_x.clear();
    intersect_y.clear();
    no_merge_list.clear();
    bin_rect.set_rectangle(i * bin_size, j * bin_size, (i + 1) * bin_size, (j + 1) * bin_size);

    //cout << "***** (" << layer << " " << i << " " << j << " "
    //     << ")******" << endl;

    //cout << "//=== find intersection of bin and poly ===// " << endl;
    for (auto poly : *(grid[layer][i][j].normal))
    {
        if (normal_list[poly].rect.bl_x >= bin_rect.bl_x)
            temp.bl_x = normal_list[poly].rect.bl_x;
        else
            temp.bl_x = bin_rect.bl_x;
        if (normal_list[poly].rect.bl_y >= bin_rect.bl_y)
            temp.bl_y = normal_list[poly].rect.bl_y;
        else
            temp.bl_y = bin_rect.bl_y;
        if (normal_list[poly].rect.tr_x >= bin_rect.tr_x)
            temp.tr_x = bin_rect.tr_x;
        else
            temp.tr_x = normal_list[poly].rect.tr_x;
        if (normal_list[poly].rect.tr_y >= bin_rect.tr_y)
            temp.tr_y = bin_rect.tr_y;
        else
            temp.tr_y = normal_list[poly].rect.tr_y;
        poly_bin_instersect.push_back(temp);
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

    // sort and erase duplicate x
    sort(intersect_x.begin(), intersect_x.end());
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
        // sort and erase duplicate y
        sort(intersect_y.begin(), intersect_y.end());
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
            sort(before_merge_x_list.begin(), before_merge_x_list.end());

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
                init_fill_list.push_back(net_temp);
                grid[layer][i][j].init_fill->push_back(init_fill_count);
                init_fill_count++;
                
            }
        }
    }
    poly_bin_instersect.clear();
    intersect_x.clear();
    intersect_y.clear();
    no_merge_list.clear();
    before_merge_x_list.clear();
    after_merge_x_list.clear();
}




void Layout::find_fill_region_y(int layer, int i, int j)
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

    poly_bin_instersect.clear();
    intersect_x.clear();
    intersect_y.clear();
    no_merge_list.clear();

    bin_rect.set_rectangle(i * bin_size, j * bin_size, (i + 1) * bin_size, (j + 1) * bin_size);

    for (auto poly : *(grid[layer][i][j].normal))
    {
        if (normal_list[poly].rect.bl_x >= bin_rect.bl_x)
            temp.bl_x = normal_list[poly].rect.bl_x;
        else
            temp.bl_x = bin_rect.bl_x;
        if (normal_list[poly].rect.bl_y >= bin_rect.bl_y)
            temp.bl_y = normal_list[poly].rect.bl_y;
        else
            temp.bl_y = bin_rect.bl_y;
        if (normal_list[poly].rect.tr_x >= bin_rect.tr_x)
            temp.tr_x = bin_rect.tr_x;
        else
            temp.tr_x = normal_list[poly].rect.tr_x;
        if (normal_list[poly].rect.tr_y >= bin_rect.tr_y)
            temp.tr_y = bin_rect.tr_y;
        else
            temp.tr_y = normal_list[poly].rect.tr_y;
        poly_bin_instersect.push_back(temp);
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

    // sort and erase duplicate y
    sort(intersect_y.begin(), intersect_y.end());
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
        // sort and erase duplicate x
        sort(intersect_x.begin(), intersect_x.end());
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
            sort(before_merge_y_list.begin(), before_merge_y_list.end());

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
                init_fill_list.push_back(net_temp);
                grid[layer][i][j].init_fill->push_back(init_fill_count);
                init_fill_count++;
            }
        }
    }

    poly_bin_instersect.clear();
    intersect_x.clear();
    intersect_y.clear();
    no_merge_list.clear();
    before_merge_y_list.clear();
    after_merge_y_list.clear();
}

// Random fill is not needed 
void Layout::random_fill(int layer, int i, int j, int x_ratio, int y_ratio)
{
    int range_x = bin_size / (min_space[layer] * x_ratio);
    int range_y = bin_size / (min_space[layer] * y_ratio);
    Rectangle bin_rect;
    Rectangle temp;
    net net_temp;
    int area_temp;
    bool no_normal, no_fill;

    // store random fill without merge
    vector<Rectangle> no_merge_list;
    vector<int> before_merge_y_list;
    vector<int> after_merge_y_list;

    bin_rect.set_rectangle(i * bin_size, j * bin_size, (i + 1) * bin_size, (j + 1) * bin_size);

    // cut one bin into many small region
    // each region size is a square with size = (min_space[layer] * 2)
    // in order to insert a square fill with size = min_space

    net_temp.layer = layer;
    net_temp.net_id = 0;

    for (int x = 0; x < range_x; x++)
    {
        no_merge_list.clear();
        before_merge_y_list.clear();
        after_merge_y_list.clear();

        temp.bl_x = bin_rect.bl_x + (x * min_space[layer] * x_ratio);
        temp.tr_x = bin_rect.bl_x + ((x + 1) * min_space[layer] * x_ratio);

        // find all possible fill region based on between (x * min_space[layer] * 3) and ( (x+1) * min_space[layer] * 3 )
        for (int y = 0; y < range_y; y++)
        {
            // each region size is a square with size = (min_space[layer] * 2)

            temp.bl_y = bin_rect.bl_y + (y * min_space[layer] * y_ratio);
            temp.tr_y = bin_rect.bl_y + ((y + 1) * min_space[layer] * y_ratio);

            // test whether this region is empty
            no_normal = 1;
            no_fill = 1;

            // test whether this region has normal
            for (auto v : *(grid[layer][i][j].normal))
            {
                area_temp = area_overlap(normal_list[v].rect, temp);
                if (area_temp > 0)
                {
                    no_normal = 0;
                }
            }

            // test whether this region has fill
            for (auto v : *(grid[layer][i][j].fill))
            {
                area_temp = area_overlap(fill_list[v].rect, temp);
                if (area_temp > 0)
                {
                    no_fill = 0;
                }
            }

            // if this region does not have normal and fill
            // add possible fill region into no_merge list
            if (no_normal && no_fill)
            {
                //no_merge_list.push_back(temp);

                net_temp.rect.bl_x = temp.bl_x + min_space[layer];
                net_temp.rect.bl_y = temp.bl_y + min_space[layer];
                net_temp.rect.tr_x = temp.tr_x - min_space[layer];
                net_temp.rect.tr_y = temp.tr_y - min_space[layer];
                fill_list.push_back(net_temp);
                grid[layer][i][j].fill->push_back(metal_fill_count);
                metal_fill_count++;
                area_temp = (temp.tr_x - temp.bl_x - 2 * min_space[layer]) * (temp.tr_y - temp.bl_y - 2 * min_space[layer]);
                grid[layer][i][j].fill_area += area_temp;
                
                // calculate for debug
                random_fill_count++;
            }
        }      
    }
}
