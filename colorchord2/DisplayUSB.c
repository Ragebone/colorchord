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

int set_interface_attribs(int fd, int speed)
{
    struct termios tty;

    if (tcgetattr(fd, &tty) < 0) {
        printf("Error from tcgetattr: %s\n", strerror(errno));
        return -1;
    }

    cfsetospeed(&tty, (speed_t)speed);
    cfsetispeed(&tty, (speed_t)speed);

    tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;         /* 8-bit characters */
    tty.c_cflag &= ~PARENB;     /* no parity bit */
    tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
    tty.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */

    /* setup for non-canonical mode */
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tty.c_oflag &= ~OPOST;

    /* fetch bytes as they become available */
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 1;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        printf("Error from tcsetattr: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}


static void DPOUpdate(void * id, struct NoteFinder*nf)
{
#ifndef WIN32

	int i, j;
	struct DPODriver * d = (struct DPODriver*)id;

	if( strcmp( d->oldSerialPort, d->serialPort ) != 0 || d->socket == -1 || d->oldBaudRate != d->baudRate )
	{
		if( d->socket >= 0 )
		{
			d->oldBaudRate = d->baudRate;
			memcpy( d->oldSerialPort, d->serialPort, PARAM_BUFF );
		}
		else
		{
			d->socket = open(d->serialPort, O_RDWR | O_NOCTTY | O_SYNC);

			if (d->socket < 0) {
				printf("DisplayUSB Error opening %s: %s\n", d->serialPort, strerror(errno));
			}
			int error = set_interface_attribs(d->socket, d->baudRate);
			if(error < 0){
				printf("DisplayUSB Error setting interface attributes %s: %s  Baud: %d\n", d->serialPort, strerror(errno), d->baudRate);
			}
		}
	}

	if( d->socket > 0 )
	{
		uint8_t buffer[MAX_BUFFER];
		uint8_t lbuff[MAX_BUFFER];

		d->firstval = 0;
		i = 0;
		while( i < d->skipfirst )
		{
			lbuff[i] = d->firstval;
			buffer[i++] = d->firstval;
		}

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
		else
		{
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
		}
		// 3 bytes per LED.
		int bytesWritten = write(d->socket, buffer, d->leds * 3);
		//tcdrain(d->socket);
		if( bytesWritten < 0 )
		{
			fprintf( stderr, "Send fault.\n" );
			close(d->socket);
			d->socket = -1;
		}
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

static struct DriverInstance * DisplayUSB(const char * parameters)
{
	struct DriverInstance * ret = malloc( sizeof( struct DriverInstance ) );
	struct DPODriver * d = ret->driverConfig = malloc( sizeof( struct DPODriver ) );
	memset( d, 0, sizeof( struct DPODriver ) );
	ret->Func = DPOUpdate;
	ret->Params = DPOParams;
	DPOParams( d );
	return ret;
}

REGISTER_OUT_DRIVER(DisplayUSB);


