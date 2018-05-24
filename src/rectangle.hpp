#ifndef RECTANGLE_HPP
#define RECTANGLE_HPP
#include <iostream>
#include <fstream>
#include <vector>
#include <string.h>
#include <algorithm>

using namespace std;

class Rectangle{
public:
    // constructer
    Rectangle(){}  
    // constructer with initialize 
    Rectangle(int _bl_x, int _bl_y, int _tr_x, int _tr_y)
            {bl_x = _bl_x; bl_y = _bl_y; tr_x = _tr_x; tr_y = _tr_y;}
    // copy constructer
    Rectangle(const Rectangle &_r) 
            {bl_x = _r.bl_x; bl_y = _r.bl_y; tr_x = _r.tr_x; tr_y = _r.tr_y;}

    void set_rectangle(int _bl_x, int _bl_y, int _tr_x, int _tr_y);
    int area();

    friend class Layout;   // class Layout can access private member
    friend int area_overlap(const Rectangle &_r1, const Rectangle &_r2);
    friend int rect_distance(const Rectangle &_r1, const Rectangle &_r2);
    
private: 
    int bl_x;   // buttom left X
    int bl_y;   // buttom left y
    int tr_x;   // top right x
    int tr_y;   // top right y
};
#endif