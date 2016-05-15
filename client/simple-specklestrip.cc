// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; -*-
//
// Simple example how to write a client.
// This sets two points. A red at (0,0); a blue dot at (5,5)
//
// By default, connects to the installation at Noisebridge. If using a
// different display (e.g. a local terminal display)
// pass the hostname as parameter:
//
//  ./simple-example localhost
//
// .. or set the environment variable FT_DISPLAY to not worry about it
//
//  export FT_DISPLAY=localhost
//  ./simple-example

#include "udp-flaschen-taschen.h"

#include <stdio.h>
#include <unistd.h>

#define DISPLAY_WIDTH 1  
#define DISPLAY_HEIGHT 810
#define BIN_SIZE 81 

int main(int argc, char *argv[]) {
    const char *hostname = NULL;   // Will use default if not set otherwise.
    if (argc > 1) {
        hostname = argv[1];        // Hostname can be supplied as first arg
    }

    // Open socket and create our canvas.
    const int socket = OpenFlaschenTaschenSocket(hostname);
    UDPFlaschenTaschen canvas(socket, DISPLAY_WIDTH, DISPLAY_HEIGHT);
   
    const Color pink(28, 0, 45);
    const Color red(28 , 0, 0);
    for (int j = BIN_SIZE ; j > 0 ; j--) {
        for (int i = 0; i < DISPLAY_HEIGHT ; i++) {
            if (i % j == 0 ) {
         //for (int j = 0; j  <  BIN_SIZE ; j++){
	 //   fprintf(stdout, "light on  i + j * 10 = %d \n", i + j * BIN_SIZE );
         //   int light_address = i + j * 10 ;
	 //    if (light_address < DISPLAY_HEIGHT){
	    if (i % 2 == 0 ){
             canvas.SetPixel(0, i  , red);  // Sample with color variable.
	     }
	    else  {
	      canvas.SetPixel(0, i , pink);
	    }
            canvas.Send();                           // Send the framebuffer.
	    }
	}
        usleep(60000);
    }
}
