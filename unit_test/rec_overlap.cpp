#include <iostream>
#include <algorithm>
using namespace std;
// Test program: Calculating overlapped area
// Input 4 parameters for each rectangle 
int main(){
    int a_bl_x,a_bl_y,a_tr_x,a_tr_y;
    int b_bl_x,b_bl_y,b_tr_x,b_tr_y;
    int overlap = 0;
    int area_a = 0;
    int area_b = 0;
    int union_area = 0;

    cout<<"Input the 1st rctangle parameters\n";
    cin>>a_bl_x>>a_bl_y>>a_tr_x>>a_tr_y;
    cout<<"Input the 2nd rctangle parameters\n";
    cin>>b_bl_x>>b_bl_y>>b_tr_x>>b_tr_y;

    area_a = (a_tr_x-a_bl_x) * (a_tr_y - a_bl_y);
    area_b = (b_tr_x-b_bl_x) * (b_tr_y - b_bl_y);
    overlap = max(0, min(a_tr_x, b_tr_x) - max(a_bl_x, b_bl_x)) * max(0, min(a_tr_y, b_tr_y) - max(a_bl_y, b_bl_y));
    union_area = (area_a + area_b) - overlap;
    
    cout<<"Area of a = "<<area_a<<"\n";
    cout<<"Area of b = "<<area_b<<"\n";    
    cout<<"Overlapped area = "<<overlap<<"\n";
    cout<<"Union area = "<<union_area<<endl;
    
    return 0;
}