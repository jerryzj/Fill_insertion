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