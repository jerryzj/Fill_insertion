#include <iostream>
#include <string>
using namespace std;

int main(){
    int x, y;
    string key;
    string key1("*");

    cout<<"input two indices:";
    cin>>x>>y;
    if(y == 0){
        cerr<<"The range of the second parameter is 1~9\n";
        exit(-1);
    }
    if(x > 9 || x < 0){
        cerr<<"Parameter out of range\n";
        exit(-1);
    }
    if(y > 9 || y < 1){
        cerr<<"Parameter out of range\n";
        exit(-1);
    }

    // handle diagonal lateral cases
    if (x == y){
        key.assign("lateral_table_"+to_string(x));
    }
    // handle area and fringe cases
    else{
        if(x == 0){
            key.assign("area_table_"+to_string(y)+"_"+to_string(x));
        }
        else if(x == 1){
            key.assign("area_table_"+to_string(x)+"_"+to_string(y));
            key1.assign("fringe_table_"+to_string(x)+"_"+to_string(y));
            
        }
        else if (x > y){
            key.assign("area_table_"+to_string(y)+"_"+to_string(x));
            key1.assign("fringe_table_"+to_string(x)+"_"+to_string(y));
        }
        else{
            key.assign("area_table_"+to_string(x)+"_"+to_string(y));
            key1.assign("fringe_table_"+to_string(x)+"_"+to_string(y));
        }
    }
    cout<<key<<endl;
    cout<<key1<<endl;
    return 0;
}