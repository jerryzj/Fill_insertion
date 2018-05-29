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