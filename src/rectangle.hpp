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
    // copy constructor
    Rectangle(const Rectangle &_r) 
            {bl_x = _r.bl_x; bl_y = _r.bl_y; tr_x = _r.tr_x; tr_y = _r.tr_y;}

    void set_rectangle(int _bl_x, int _bl_y, int _tr_x, int _tr_y);
    void dump();
    string dump_string();

    int area();
    int width();
    int length();

    bool check_width(int min_width, int max_width);


    friend class Layout;   // class Layout can access private member
    friend class readprocess;
    // calculate overlapped area between two rectangles
    friend int area_overlap(const Rectangle &_r1, const Rectangle &_r2);
    friend bool check_space(const Rectangle &_r1, const Rectangle &_r2, int margin);
    friend Rectangle rect_overlap(const Rectangle &_r1, const Rectangle &_r2); 
    friend Rectangle rect_resize(const Rectangle &_in, double lf, double dw, double rt, double up);
    

private: 
    int bl_x;   // buttom left X
    int bl_y;   // buttom left y
    int tr_x;   // top right x
    int tr_y;   // top right y
};

int area_overlap(const Rectangle &_r1, const Rectangle &_r2);
bool check_space(const Rectangle &_r1, const Rectangle &_r2, int margin);
Rectangle rect_overlap(const Rectangle &_r1, const Rectangle &_r2); 
Rectangle rect_resize(const Rectangle &_in, double lf=0.0, double dw=0.0, double rt=0.0, double up=0.0);

#endif