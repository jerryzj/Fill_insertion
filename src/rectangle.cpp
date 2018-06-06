#include"rectangle.hpp"

void Rectangle::set_rectangle(int _bl_x, int _bl_y, int _tr_x, int _tr_y){
    bl_x = _bl_x;
    bl_y = _bl_y;
    tr_x = _tr_x;
    tr_y = _tr_y;
}

int Rectangle::area(){
    int area = (tr_x - bl_x) * (tr_y - bl_y);

    if (area <= 0){
        cerr << "The rectangle has area <=0 " << endl;
    }
    return area;
}

int area_overlap(const Rectangle &_r1, const Rectangle &_r2){
    int area = 0;

    area = max(0, min(_r1.tr_x, _r2.tr_x) - max(_r1.bl_x, _r2.bl_x)) * max(0, min(_r1.tr_y, _r2.tr_y) - max(_r1.bl_y, _r2.bl_y));
    
    return area;
}

bool check_space(const Rectangle &_r1, const Rectangle &_r2, int margin){
    int area = 0;
    // Extend _r1 by margin and check the overlapped area
    // If area is not zero means there exist a rule violation
    Rectangle temp( _r1.bl_x-margin, _r1.bl_y-margin,
                    _r1.tr_x+margin, _r1.tr_y+margin);
    // check this if we will insert fill at (0,0)
    //if(temp.bl_x < 0) temp.bl_x = 0;
    //if(temp.bl_y < 0) temp.bl_y = 0;
        
    area = area_overlap(temp, _r2);
    if(area != 0){
        return false;
    }
    else{
        return true;
    }
}

void Rectangle::dump()
{
    cout << "rect " 
         << bl_x << " "
         << bl_y << " " 
         << tr_x << " " 
         << tr_y << endl;
}

string Rectangle::dump_string()
{
    string rect_str;


    rect_str = to_string(bl_x) + " " + 
                to_string(bl_y) + " " + 
                to_string(tr_x) + " " + 
                to_string(tr_y);

    return rect_str;
}

Rectangle rect_overlap(const Rectangle &_r1, const Rectangle &_r2)
{

    Rectangle r_out;
    
    r_out.bl_x = max(_r1.bl_x, _r2.bl_x);
    r_out.bl_y = max(_r1.bl_y, _r2.bl_y);
    r_out.tr_x = min(_r1.tr_x, _r2.tr_x);
    r_out.tr_y = min(_r1.tr_y, _r2.tr_y);

    return r_out;
}

bool Rectangle::check_width(int min_width, int max_width)
{

    int width_x = tr_x - bl_x;
    int length_y = tr_y - bl_y;

    if (width_x < min_width || length_y < min_width) 
        return false;
    if (width_x > max_width || length_y > max_width)
        return false;
    return true;
}

// resize and return a resized rectangle
// enlarge if parameter is negative
Rectangle rect_resize(const Rectangle &_in, double lf, double dw, double rt, double up)
{
    Rectangle _out;

    int width_x = _in.tr_x - _in.bl_x;
    int length_y = _in.tr_y - _in.bl_y;   

    _out = _in;

    _out.bl_x += lf * width_x;
    _out.bl_y += dw * length_y;
    _out.tr_x -= rt * width_x;
    _out.tr_y -= up * length_y;

    return _out;
}