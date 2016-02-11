// 
// flaschen taschen string to font decoder 
// expand w/ info how to write and manipulate a font 
//

#include "udp-flaschen-taschen.h"
#include "flaschen-taschen-font.h"
#include <unistd.h>
#include <stdio.h>
#include <map> 
#include <string>

#define DISPLAY_WIDTH  40
#define DISPLAY_HEIGHT 40

int main(int argc, char *argv[]) {
    const char *hostname = "127.0.0.1";
    if (argc > 1) {
        hostname = argv[1];     // Single command line argument.
    }
    fprintf(stderr, "Sending to %s\n", hostname);

    // Open socket and create our canvas.
    const int socket = OpenFlaschenTaschenSocket(hostname);
    UDPFlaschenTaschen canvas(socket, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    canvas.SetOffset(10,5);
    const Color white(255, 255,  255);
    const Color red(255, 0, 0);
    const Color blue(0, 0, 255);
    printf("clearing the canvas\n");  
    canvas.Clear();                               //  empty framebuffer. 
    canvas.Send();                                //  Send the framebuffer.
    for(int incr=0; incr < sizeof(glyph)/sizeof(*glyph);++incr){
        canvas.Clear();                               //  empty framebuffer. 
        for(int y=0; y<5; y++){
            for(int x=0; x<5; x++){
                if((glyph[incr][y]) & (1<<(4-x)) ){ // cmp glyph val & left shifted 1
                  canvas.SetPixel(x , y, red);        //  fill sample with color variable.
                  printf("set a red pixel at (%d,%d)\n", x, y);  
                }
            }
        }
        printf("send and sleep 500,000uS \n");  
        canvas.Send();                          //  Send the framebuffer.
        usleep(100000);
     }
}
