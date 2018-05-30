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

void Layout::find_fill_region()
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

    int range_x = normal_list[0].rect.tr_x / bin_size;
    int range_y = normal_list[0].rect.tr_y / bin_size;
    int init_fill_count = 0;
    
    bool not_poly;

    cout << "//=== Find fill region ===//" << endl;
    for (int layer = 1; layer < 10; layer++)
    {
        for (int i = 0; i < range_x; i++)
        {
            for (int j = 0; j < range_y; j++)
            {

                poly_bin_instersect.clear();
                intersect_x.clear();
                intersect_y.clear();
                bin_rect.set_rectangle(i * bin_size, j * bin_size, (i + 1) * bin_size, (j + 1) * bin_size);
                //cout << "***** (" << layer << " " << i << " " << j << " " << ")******" << endl;
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
                /*
                // dump poly_bin intersect
                if (layer == 4 && i == 47 && j == 21)
                {
                    cout << "dump poly_bin intersect" << endl;
                    for (auto v : poly_bin_instersect)
                    {
                        cout << v.bl_x << " "
                             << v.bl_y << " "
                             << v.tr_x << " "
                             << v.tr_y << endl;
                    }
                    cout << "-----------------------" << endl;
                }
                */

                //cout << "//=== sort intersection x, y point individually ===// " << endl;
                for (auto x : poly_bin_instersect)
                {
                    intersect_x.push_back(x.bl_x);
                    intersect_x.push_back(x.tr_x);
                }

                if (intersect_x.size() == 0)
                {
                    intersect_x.push_back(bin_rect.bl_x);
                    intersect_x.push_back(bin_rect.tr_x);
                }
                else
                {
                    if (intersect_x[0] > bin_rect.bl_x)
                        intersect_x.insert(intersect_x.begin(), bin_rect.bl_x);
                    if (intersect_x.back() < bin_rect.tr_x)
                        intersect_x.push_back(bin_rect.tr_x);
                }
                sort(intersect_x.begin(), intersect_x.end());
                intersect_x.erase(unique(intersect_x.begin(), intersect_x.end()), intersect_x.end());

                //cout << "//=== Set initialize fill region ===// " << endl;
                net_temp.net_id = 0;
                net_temp.layer = layer;

                for (int in_x = 0; in_x < intersect_x.size() - 1; in_x++)
                {
                    for (auto v : poly_bin_instersect)
                    {
                        // find poly intersect with line x = intersect[in_x] and x = intersect[in_x+1]
                        if ((intersect_x[in_x] >= v.bl_x && intersect_x[in_x] <= v.tr_x) || 
                        (intersect_x[in_x+1] >= v.bl_x && intersect_x[in_x+1] <= v.tr_x)
                        )
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
                    else
                    {
                        // add bin boundary as intersection when available
                        if (intersect_y[0] > bin_rect.bl_y)
                            intersect_y.insert(intersect_y.begin(), bin_rect.bl_y);
                        if (intersect_y.back() < bin_rect.tr_y)
                            intersect_y.push_back(bin_rect.tr_y);
                    }
                    sort(intersect_y.begin(), intersect_y.end());
                    intersect_y.erase(unique(intersect_y.begin(), intersect_y.end()), intersect_y.end());

                    /*
                    // dump intersect x y
                    if (layer == 4 && i == 47 && j == 21)
                    {
                        cout << "intersect x size = " << intersect_x.size()
                             << " intersect y size = " << intersect_y.size() << endl;
                        cout << "intersect x = " << intersect_x[in_x] << " " << intersect_x[in_x+1] << endl;
                        cout << "intersect y = ";
                        for (auto v : intersect_y)
                        {
                            cout << v << " ";
                        }
                        cout << endl;
                    }
                    */

                    
                    for (int in_y = 0; in_y < intersect_y.size() - 1; in_y++)
                    {
                        not_poly = 1;
                        // for intersection points, filter out normal poly
                        for (auto v : poly_bin_instersect)
                        {
                            // intersection point = normal bl and tr
                            if ((intersect_y[in_y] == v.bl_y) && (intersect_y[in_y + 1] == v.tr_y))
                            {
                                not_poly = 0;
                            }

                            // intersection in normal poly, filter out overlap normal poly
                            if((intersect_y[in_y] > v.bl_y) && (intersect_y[in_y] < v.tr_y))
                            {
                                not_poly = 0;
                            }

                            // intersection in normal poly, filter out overlap normal poly
                            if((intersect_y[in_y+1] > v.bl_y) && (intersect_y[in_y+1] < v.tr_y))
                            {
                                not_poly = 0;
                            }
                        }

                        if (not_poly)
                        {
                            net_temp.rect.bl_x = intersect_x[in_x];
                            net_temp.rect.bl_y = intersect_y[in_y];
                            net_temp.rect.tr_x = intersect_x[in_x + 1];
                            net_temp.rect.tr_y = intersect_y[in_y + 1];

                            /*
                            // dump for debug
                            if (layer == 4 && i == 47 && j == 21)
                            {
                                cout << "dump init fill region for debug" << endl;
                                net_temp.rect.dump();
                            }
                            */

                            init_fill_list.push_back(net_temp);
                            grid[layer][i][j].init_fill->push_back(init_fill_count);
                            init_fill_count++;

                        }
                    }

                    intersect_y.clear();
                }
            }
        }
    }
}

void Layout::metal_fill()
{
    cout << "//=== Start metal Fill ===//" << endl;

    Rectangle temp;
    net net_temp;
    int range_x = normal_list[0].rect.tr_x / bin_size;
    int range_y = normal_list[0].rect.tr_y / bin_size;

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
    int metal_fill_cout = 0;

    for (int layer = 1; layer < 10; layer++)
    {
        for (int i = 0; i < range_x; i++)
        {
            for (int j = 0; j < range_y; j++)
            {

                // Calculate dennsity
                bin_normal_area(layer, i, j);
                poly_density = (double)(grid[layer][i][j].normal_area / (double)(bin_size * bin_size));

                //cout << "//=== Start fill insertion ===// " << endl;
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
                                /*** 5/29 should use set_rectangle ***/
                                net_temp.rect.set_rectangle(fill_bl_x[a], fill_bl_y[b], 
                                                            fill_tr_x[a], fill_tr_y[b]);
                                fill_list.push_back(net_temp);

                                /*************************************/

                                // dump for debugging
                                /*
                                if (layer == 4 && i == 47 && j == 21)
                                {
                                    //cout << "dump fill region for debug" << endl;
                                    net_temp.rect.dump();
                                }
                                */
                                area_temp = net_temp.rect.area();
                                fill_area_sum += area_temp;
                                grid[layer][i][j].fill->push_back(metal_fill_cout);
                                metal_fill_cout++;
                            }
                        }
                    }
                    fill_region_used_count++;
                    no_more_fill_region = (fill_region_used_count > grid[layer][i][j].init_fill->size());
                }

                curr_density += ((double)fill_area_sum / (double)(bin_size * bin_size));
                grid[layer][i][j].fill_area = fill_area_sum;
                // run time density check

                if (curr_density >= min_density[layer])
                {
                    density_pass_count++;
                }
                else
                {
                    //cout << "num: " << layer << " " << i << " " << j << " ";
                    //cout << "fail ";
                    density_fail_count++;
                    //cout << " total density " << curr_density << " ";
                    if (curr_density == poly_density)
                    {
                        //cout << "no fill, poly density = " << poly_density << endl;
                        no_fill_count++;
                    }
                    else
                    {
                        //cout << endl;
                    }
                }
            }
        }
    }
    cout << "bin density check: " << endl;
    cout << "density pass count " << density_pass_count << endl;
    cout << "density fail count " << density_fail_count << endl;
    cout << "no fill count " << no_fill_count << endl;
    cout << "pass = " << (double)(density_pass_count) / double(density_pass_count + density_fail_count) << endl;
}

/***************************************************************/
/**** 5/29 window-based check                               ****/
/**** Re-calculate normal and fill areas in each window     ****/
/**** need to do verification to check if                   ****/
/**** window-based areas match the areas found in each bin  ****/ 
/**** during filling functions                              ****/ 
void Layout::window_based_density_check()
{
    cout << "//==== Window based density check ===//" << endl;
    cout << "window size = " << bin_size * 2 << endl;
    int range_x = normal_list[0].rect.tr_x / bin_size;
    int range_y = normal_list[0].rect.tr_y / bin_size;
    // bin size = window size / 2
    // step = bin_size

    // metal_area sum up normal and fill area in one window
    long int metal_area;
    long int fill_area_sum;
    long int normal_area_sum;
    double window_density;
    int density_check_fail_count = 0;
    int density_check_pass_count = 0;
    int density_check_larger_than_max_count = 0;

    for (int layer = 1; layer <= 9; layer++) // 5/29 modified 
    {
        for (int i = 0; i < range_x - 1; i++)
        {
            for (int j = 0; j < range_y - 1; j++)
            { 
                metal_area = 0;
                fill_area_sum = 0;
                normal_area_sum = 0;

                fill_area_sum += grid[layer][i][j].fill_area;
                fill_area_sum += grid[layer][i + 1][j].fill_area;
                fill_area_sum += grid[layer][i][j + 1].fill_area;
                fill_area_sum += grid[layer][i + 1][j + 1].fill_area;

                normal_area_sum += grid[layer][i][j].normal_area;
                normal_area_sum += grid[layer][i + 1][j].normal_area;
                normal_area_sum += grid[layer][i][j + 1].normal_area;
                normal_area_sum += grid[layer][i + 1][j + 1].normal_area;

                metal_area = normal_area_sum + fill_area_sum;

                window_density = ((double)(metal_area) / (double)(bin_size * bin_size * 4));
                if (window_density < min_density[layer])
                {
                    cout << "fail window_density: " << window_density << endl;
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
    cout << "density pass count " << density_check_pass_count << endl;
    cout << "density fail count " << density_check_fail_count << endl;
    cout << "density larger than max density count " << density_check_larger_than_max_count << endl;
    cout << "pass = " << (double)(density_check_pass_count) / double(density_check_pass_count + density_check_fail_count) << endl;
}

// check min_width, max_fill_width
/*** 5/29 should modify to window-based test ***/
/***********************************************/
void Layout::DRC_check_width()
{
    cout << "//=== start DRC check ===//" << endl;

    bool min_space_pass;
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
            max_fill_width_fail_count++;
        }
    }
    cout << "total fill: " << fill_list.size() << endl;
    cout << "min_width check fail " << min_width_fail_count << endl;
    cout << "max fill width check fail " << max_fill_width_fail_count << endl;
    cout << "----------------------------" << endl;
}
/***********************************************/
/***********************************************/

// check min_space btw fill & normal, fill & fill
/*** 5/29 should modify to window-based test ***/
/***********************************************/
void Layout::DRC_check_space()
{
    int normal_normal_fail_count = 0;
    int fill_normal_fail_count = 0;
    int fill_fill_fail_count = 0;
    bool check_space_pass;
    int range_x = normal_list[0].rect.tr_x / bin_size;
    int range_y = normal_list[0].rect.tr_y / bin_size;
    int test_x_size, test_y_size;
    int a, b; // loop index
    vector<int> fill_layer_split_index;
    fill_layer_split_index.reserve(11);
    //cout << "//=== check space between normal and normal ===//" << endl;

    cout << "//=== check space between fill and normal ===//" << endl;
    for (int layer = 1; layer <= 9; layer++)  // 5/29 modified 
    {
        //cout << "layer " << layer << endl;
        for (int i = 0; i < range_x; i++)
        {
            for (int j = 0; j < range_y; j++)
            {
                test_x_size = grid[layer][i][j].fill->size();
                test_y_size = grid[layer][i][j].normal->size();
                //cout << test_x_size << " " << test_y_size << endl;
                for (int fill_x = 0; fill_x < test_x_size; fill_x++)
                {
                    a = grid[layer][i][j].fill->at(fill_x);
                    for (int normal_y = 0; normal_y < test_y_size; normal_y++)
                    {
                        b = grid[layer][i][j].normal->at(normal_y);
                        //cout << "min space = " << min_space[layer] << endl;
                        check_space_pass = check_space(fill_list[a].rect, normal_list[b].rect, min_space[layer]);
                        if (!check_space_pass)
                        {   /*
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
    for (int layer = 1; layer < 10; layer++)
    {
        for (int i = 0; i < range_x; i++)
        {
            for (int j = 0; j < range_y; j++)
            {
                test_x_size = grid[layer][i][j].fill->size();
                test_y_size = grid[layer][i][j].fill->size();
                //cout << test_x_size << " " << test_y_size << endl;
                for (int fill_x = 0; fill_x < test_x_size; fill_x++)
                {
                    a = grid[layer][i][j].fill->at(fill_x);
                    for (int fill_y = fill_x + 1; fill_y < test_y_size; fill_y++)
                    {
                        b = grid[layer][i][j].fill->at(fill_y);
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
/***********************************************/
/***********************************************/

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
void Layout::dump_bin(int layer, int x, int y){
    fstream density_file;    
    fstream normal_file;
    fstream fill_file;
    string temp;
    int lower_bound_x = x * bin_size;
    int lower_bound_y = y * bin_size;
    int upper_bound_x = (x + 1) * bin_size;
    int upper_bound_y = (y + 1) * bin_size;
    
    double density = ((double) grid[layer][x][y].normal_area + 
                        (double) grid[layer][x][y].fill_area) / (bin_size*bin_size);

    string filename("density.txt");
    density_file.open(filename.c_str(),ios::out);
    if(!density_file){
        cerr<<"Error create density file\n";
        exit(-1);
    }    
    temp.assign(to_string(density)+"\n");
    density_file.write(temp.c_str(),temp.length());
    density_file.close();

    filename.assign("normal.cut");
    normal_file.open(filename.c_str(),ios::out);
    if(!normal_file){
        cerr<<"Error create bin_normal file\n";
        exit(-1);
    }
    temp.assign(to_string(lower_bound_x)+" "
               +to_string(lower_bound_y)+" "
               +to_string(upper_bound_x)+" "
               +to_string(upper_bound_y)+
               "; chip boundary\n");
    
    // write chip boundary to normal file
    normal_file.write(temp.c_str(),temp.length());
    // write normal poly info to normal_file
    for(auto i : *(grid[layer][x][y].normal)){
        Rectangle temp = normal_list[i].rect;
        if(temp.bl_x < lower_bound_x) 
            temp.bl_x = lower_bound_x;
        if(temp.bl_x > upper_bound_x){
            cerr<<"Error! poly bl_x bigger than bin upper bound\n";
            exit(-1);
        }
        if(temp.tr_x > upper_bound_x){
            temp.tr_x = upper_bound_x;
        }
        if(temp.tr_x < lower_bound_x){
            cerr<<"Error! poly tr_x smaller than bin lower bound\n";
            exit(-1);
        }        
        if(temp.bl_y < lower_bound_y){
            temp.bl_y = lower_bound_y;
        }
        if(temp.bl_y > upper_bound_y){
            cerr<<"Error! poly bl_y bigger than bin upper bound\n";
            exit(-1);
        }
        if(temp.tr_y > upper_bound_y){
            temp.tr_y = upper_bound_y;
        }
        if(temp.tr_y < lower_bound_y){
            cerr<<"Error! poly tr_y smaller than bin lower bound\n";
            exit(-1);
        }
        string s(to_string(temp.bl_x) + " " + 
                 to_string(temp.bl_y) + " " +
                 to_string(temp.tr_x) + " " +
                 to_string(temp.tr_y) + " " +
                 to_string(normal_list[i].net_id) + " " +
                 "normal\n"
        );
        normal_file.write(s.c_str(),s.length());
    }
    normal_file.close();

    filename.assign("fill.cut");
    fill_file.open(filename.c_str(),ios::out);
    if(!fill_file){
        cerr<<"Error create bin_fill file\n";
        exit(-1);
    }
    temp.assign(to_string(lower_bound_x)+" "
               +to_string(lower_bound_y)+" "
               +to_string(upper_bound_x)+" "
               +to_string(upper_bound_y)+
               "; chip boundary\n");
    
    // write chip boundary to normal file
    fill_file.write(temp.c_str(),temp.length());
    for(auto i : *(grid[layer][x][y].fill)){
        Rectangle temp = fill_list[i].rect;
        string s(to_string(temp.bl_x) + " " + 
                 to_string(temp.bl_y) + " " +
                 to_string(temp.tr_x) + " " +
                 to_string(temp.tr_y) + " " +
                 to_string(fill_list[i].net_id) + " " +
                 "fill\n"
        );
        fill_file.write(s.c_str(),s.length());
    }
    fill_file.close();
}
