#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdint.h>


#include "color.h"
#include "parameters.h"
#include "DrawFunctions.h"
#include "outdrivers.h"
#include "notefinder.h"

/***
 *  Display Driver to output raw RGB Bytes to a Serial USB
 * 	Like an Arduino.
 *
 * 	Originally Copyed from DisplayNetwork.
 * 	Originall Copyright:
 * 	Copyright 2015 <>< Charles Lohr under the ColorChord License.
 */


#if defined(WIN32) || defined(WINDOWS)
#include <windows.h>
#ifdef TCC
#include <winsock2.h>
#endif
#define MSG_NOSIGNAL 0
#else
#define closesocket( x ) close( x )
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#define MAX_BUFFER 1487*2

struct DPODriver
{
	int leds;
	int skipfirst;
	int fliprg;
	int firstval;
	int is_rgby;
	int skittlequantity; //When LEDs are in a ring, backwards and forwards  This is the number of LEDs in the ring.

	char serialPort[PARAM_BUFF];
	char oldSerialPort[PARAM_BUFF];

	int baudRate;
	int oldBaudRate;

	int socket;		// File-Descriptor of the socket
};


int set_interface_attribs (int fd, int speed)
{
    int parity = 0;
    struct termios tty;
    memset (&tty, 0, sizeof tty);
    if (tcgetattr (fd, &tty) != 0)
    {
        printf("error %d from tcgetattr", errno);
        return -1;
    }

    cfsetospeed (&tty, speed);
    cfsetispeed (&tty, speed);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
    // disable IGNBRK for mismatched speed tests; otherwise receive break
    // as \000 chars
    tty.c_iflag &= ~IGNBRK;         // disable break processing
    tty.c_lflag = 0;                // no signaling chars, no echo,
    // no canonical processing
    tty.c_oflag = 0;                // no remapping, no delays
    tty.c_cc[VMIN]  = 0;            // read doesn't block
    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

    tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
    // enable reading
    tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
    tty.c_cflag |= parity;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr (fd, TCSANOW, &tty) != 0)
    {
        printf("error %d from tcsetattr", errno);
        return -1;
    }
    return 0;
}



static void DPOUpdate(void * id, struct NoteFinder*nf)
{
#ifndef WIN32

	//int i, j;


	struct DPODriver * d = (struct DPODriver*)id;

	if( strcmp( d->oldSerialPort, d->serialPort ) != 0 || d->socket == -1 || d->oldBaudRate != d->baudRate )
	{
		if( d->socket >= 0 )
		{
			d->oldBaudRate = d->baudRate;
			memcpy( d->oldSerialPort, d->serialPort, PARAM_BUFF );
            close(d->socket);
            d->socket = -1;
		}

        //d->socket = open(d->serialPort, O_NOCTTY | O_SYNC | O_WRONLY | O_NONBLOCK);
        d->socket = open(d->serialPort, O_NOCTTY |  O_WRONLY );

        if (d->socket < 0) {
            printf("DisplayUSB Error opening %s: %s\n", d->serialPort, strerror(errno));
        }
        int error = set_interface_attribs(d->socket, d->baudRate);
        if(error < 0){
            printf("DisplayUSB Error setting interface attributes %s: %s  Baud: %d\n", d->serialPort, strerror(errno), d->baudRate);
        }

        // First time on this socket,  send a delimiter to declare a new update cycle.
        
	}

	if( d->socket > 0 )
	{

	    /*
		uint8_t buffer[MAX_BUFFER];
		uint8_t lbuff[MAX_BUFFER];

		d->firstval = 0;
		i = 0;
		while( i < d->skipfirst )
		{
			lbuff[i] = d->firstval;
			buffer[i++] = d->firstval;
		}
	     */
		/*
		if( d->is_rgby )
		{
			i = d->skipfirst;
			int k = 0;
			if( d->leds * 4 + i >= MAX_BUFFER )
				d->leds = (MAX_BUFFER-1)/4;

			//Copy from OutLEDs[] into buffer, with size i.
			for( j = 0; j < d->leds; j++ )
			{
				int r = OutLEDs[k++];
				int g = OutLEDs[k++];
				int b = OutLEDs[k++];
				int y = 0;
				int rg_common;
				if( r/2 > g ) rg_common = g;
				else        rg_common = r/2;

				if( rg_common > 255 ) rg_common = 255;
				y = rg_common;
				r -= rg_common;
				g -= rg_common;
				if( r < 0 ) r = 0;
				if( g < 0 ) g = 0;

				//Conversion from RGB to RAGB.  Consider: A is shifted toward RED.
				buffer[i++] = g; //Green
				buffer[i++] = r; //Red
				buffer[i++] = b; //Blue
				buffer[i++] = y; //Amber
			}
		}
		 */

		// Idea, delimiter can be X 0Bytes because runLength should never be 0. and RL being 0 makes no fucken sense. So if we send for example 5 0 Bytes,





        // 2 bytes anzahl, 3 Bytes Color.
        int bytesWritten, j;
        uint16_t runLength = 0;
        uint8_t color[3], oldColor[3];

        // first color = oldColor as preset to start from and never have a RL 0
        oldColor[0] = OutLEDs[0];
        oldColor[1] = OutLEDs[1];
        oldColor[2] = OutLEDs[2];

        // Complememnt to serial.
        bytesWritten = write(d->socket, oldColor, 3);
        if( bytesWritten < 2 )
        {
            fprintf( stderr, "Send fault.\n" );
            close(d->socket);
            d->socket = -1;
            return;
        }

		for( j = 0; j < d->leds; j++ )
		{
			color[0] = OutLEDs[j*3+0]; //GREEN
            color[1] = OutLEDs[j*3+1]; //RED
            color[2] = OutLEDs[j*3+2]; //BLUE

            // did the color change?
			if(color[0] != oldColor[0] || color[1] != oldColor[1] || color[2] != oldColor[2]){

			    printf("RL: %d , Color %d %d %d ", runLength, oldColor[0], oldColor[1], oldColor[2]);

                //build and output runLength

                bytesWritten = write(d->socket, &runLength, 2);

                if( bytesWritten < 2 )
                {
                    fprintf( stderr, "Send fault.\n" );
                    close(d->socket);
                    d->socket = -1;
                    break;
                }
                //Output Old Color wtih 3 bytes per Color, as in RGB
                bytesWritten = write(d->socket, oldColor, 3);

                if( bytesWritten < 3 )
				{
                    fprintf( stderr, "Send fault.\n" );
                    close(d->socket);
                    d->socket = -1;
                    break;
                }

                // reset RL
				runLength = 1;

				// make oldColor = color
				int i;
				for(i = 0; i <3; i++){
				    oldColor[i] = color[i];
				}
			}else{
				runLength ++;
			}
		}

        printf("RL: %d , Color %d %d %d ", runLength, oldColor[0], oldColor[1], oldColor[2]);

        //build and output runLength

        bytesWritten = write(d->socket, &runLength, 2);

        if( bytesWritten < 2 )
        {
            fprintf( stderr, "Send fault.\n" );
            close(d->socket);
            d->socket = -1;
        }
        //Output Old Color wtih 3 bytes per Color, as in RGB
        bytesWritten = write(d->socket, oldColor, 3);

        if( bytesWritten < 3 )
        {
            fprintf( stderr, "Send fault.\n" );
            close(d->socket);
            d->socket = -1;
        }
        // End Delimiter.
        uint8_t delimiter[7] = {0,0,0,0,0,0,255};
        bytesWritten = write(d->socket, delimiter, 7);

        if( bytesWritten < 3 )
        {
            fprintf( stderr, "Send fault.\n" );
            close(d->socket);
            d->socket = -1;
        }

        printf("\n");

/*
			if( d->fliprg )
			{
				for( j = 0; j < d->leds; j++ )
				{
					lbuff[i++] = OutLEDs[j*3+1]; //GREEN??
					lbuff[i++] = OutLEDs[j*3+0]; //RED??
					lbuff[i++] = OutLEDs[j*3+2]; //BLUE
				}
			}
			else
			{
				for( j = 0; j < d->leds; j++ )
				{
					lbuff[i++] = OutLEDs[j*3+0];  //RED
					lbuff[i++] = OutLEDs[j*3+2];  //BLUE
					lbuff[i++] = OutLEDs[j*3+1];  //GREEN
				}
			}
*/
			/*
			if( d->skittlequantity )
			{
				i = d->skipfirst;
				for( j = 0; j < d->leds; j++ )
				{
					int ledw = j;
					int ledpor = ledw % d->skittlequantity;
					int ol;

					if( ledw >= d->skittlequantity )
					{
						ol = ledpor*2-1;
						ol = d->skittlequantity*2 - ol -2;
					}
					else
					{
						ol = ledpor*2;
					}

					buffer[i++] = lbuff[ol*3+0+d->skipfirst];
					buffer[i++] = lbuff[ol*3+1+d->skipfirst];
					buffer[i++] = lbuff[ol*3+2+d->skipfirst];
				}
			}
			else
			{
				memcpy( buffer, lbuff, i );
			}
			*/

		/*
		// 3 bytes per LED.
		int bytesWritten = write(d->socket, buffer, d->leds * 3);
		//tcdrain(d->socket);
		 */
	}
#endif
}

static void DPOParams(void * id )
{
	struct DPODriver * d = (struct DPODriver*)id;

	strcpy(d->serialPort, "/dev/ttyACM0"); RegisterValue(  "serialport", PABUFFER, d->serialPort, sizeof( d->serialPort ) );
	d->oldSerialPort[0] = 0;

	d->baudRate = 115200; RegisterValue("serialbaudrate", PAINT, &d->baudRate, sizeof(d->baudRate));
	d->oldBaudRate = 0;

	d->leds = 10;		RegisterValue(  "leds", PAINT, &d->leds, sizeof( d->leds ) );
	d->skipfirst = 1;	RegisterValue(  "skipfirst", PAINT, &d->skipfirst, sizeof( d->skipfirst ) );

	d->firstval = 0;	RegisterValue(  "firstval", PAINT, &d->firstval, sizeof( d->firstval ) );
	d->fliprg = 0;		RegisterValue(  "fliprg", PAINT, &d->fliprg, sizeof( d->fliprg ) );
	d->is_rgby = 0;		RegisterValue(  "rgby", PAINT, &d->is_rgby, sizeof( d->is_rgby ) );
	d->skittlequantity=0;RegisterValue(  "skittlequantity", PAINT, &d->skittlequantity, sizeof( d->skittlequantity ) );
	d->socket = -1;
}

static struct DriverInstances * DisplayUSB(const char * parameters)
{
	struct DriverInstances * ret = malloc( sizeof( struct DriverInstances ) );
	struct DPODriver * d = ret->id = malloc( sizeof( struct DPODriver ) );
	memset( d, 0, sizeof( struct DPODriver ) );
	ret->Func = DPOUpdate;
	ret->Params = DPOParams;
	DPOParams( d );
	return ret;
}

REGISTER_OUT_DRIVER(DisplayUSB);


