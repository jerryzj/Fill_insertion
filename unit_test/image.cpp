#include "bitmap_image.hpp"
#include <sstream>

using namespace std;

int main(int argc, char* argv[]){
    ifstream file(argv[1]);
    string temp;
    size_t pos;
    int layer = 0;
    int offset_x,offset_y;
    int width,height;
    
    int line_num;
    int bl_x,bl_y;
    int tr_x,tr_y;
    int wire_num;
    int layer_num;
    char normal[7];

    cout<<"Which layer do you want to generate? ";
    cin>>layer;

    if(!file){  // check file exist or not
        cerr<<"Can't open layout file\n";
        exit(-1);
    }
    // Read layout boundary
    getline(file,temp);
    pos = temp.find(";");
    temp = temp.substr(0,pos);
    sscanf(temp.c_str(),"%d %d %d %d",&offset_x,&offset_y,&width,&height);
    width  -= offset_x;
    height -= offset_y;
    cout<<"Offset_x = "<<offset_x<<" Offset_y = "<<offset_y<<endl;
    cout<<"Width = "<<width<<" Height = "<<height<<endl;
    // image constructor
    bitmap_image image(width,height);
    // set background to white
    image.set_all_channels(255,255,255);
    image_drawer draw(image);

    cout<<"image set"<<endl;
    while(getline(file,temp)){
        sscanf(temp.c_str(),"%d %d %d %d %d %d %d %s",
                    &line_num, &bl_x, &bl_y, &tr_x, &tr_y, 
                    &wire_num, &layer_num, normal);
        if(layer != layer_num) break;
        else{
            cout<<bl_x<<" "<<bl_y<<" "<<tr_x<<" "<<tr_y<<endl;
            bl_x -= offset_x;
            tr_x -= offset_x;
            bl_y -= offset_y;
            tr_y -= offset_y;
            cout<<bl_x<<" "<<bl_y<<" "<<tr_x<<" "<<tr_y<<endl;
            draw.pen_width(1);
            draw.pen_color(0,0,255);
            draw.rectangle(bl_x,bl_y,tr_x,tr_y);
        }
    }
    image.save_image("test.bmp");

    return 0;
}