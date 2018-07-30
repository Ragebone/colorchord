//Copyright 2015 <>< Charles Lohr under the ColorChord License.

//XXX TODO Figure out why it STILL fails when going around a loop

#include <stdlib.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "outdrivers.h"
#include "notefinder.h"
#include "parameters.h"
#include "color.h"


struct RawDriver
{
	int did_init;
	int total_leds;
	int is_loop;
	float satamp;
};

// driver update method.
static void LEDUpdate(void * id, struct NoteFinder*nf)
{
	struct RawDriver * led = (struct RawDriver*)id;


	//Step 1: Calculate the quantity of all the LEDs we'll want.
	int totbins = nf->note_peaks;	//nf->dists;
	int i, j;
	float binvals[totbins];
	float binvalsQ[totbins];
	float binpos[totbins];
	float totalbinval = 0;

//	if( totbins > led_bins ) totbins = led_bins;

	for( i = 0; i < totbins; i++ )
	{

		printf("nP: %f nA2: %f nA: %f\n", nf->note_positions[i], nf->note_amplitudes2[i], nf->note_amplitudes[i]);
		binpos[i] = nf->note_positions[i] / nf->freqbins;
		binvals[i] = pow( nf->note_amplitudes2[i], 1);
//		binvals[i] = (binvals[i]<led->led_floor)?0:binvals[i];
//		if( nf->note_positions[i] < 0 ) { binvals[i] = 0; binvalsQ[i] = 0; }

		binvalsQ[i] =pow( nf->note_amplitudes[i], 1);
		// nf->note_amplitudes[i];//
		totalbinval += binvals[i];
	}



	float newtotal = 0;

	printf("current_note_id %d \n",nf->current_note_id);
}

static void LEDParams(void * id )
{
	struct RawDriver * led = (struct RawDriver*)id;

	led->satamp = 2;		RegisterValue( "satamp", PAFLOAT, &led->satamp, sizeof( led->satamp ) );
	led->total_leds = 300;	RegisterValue( "leds", PAINT, &led->total_leds, sizeof( led->total_leds ) );

	printf("output via Raw Driver");

}


static struct DriverInstances * OutputRaw()
{
	struct DriverInstances * ret = malloc( sizeof( struct DriverInstances ) );
	memset( ret, 0, sizeof( struct DriverInstances ) );
	struct RawDriver * led = ret->id = malloc( sizeof( struct RawDriver ) );
	memset( led, 0, sizeof( struct RawDriver ) );

	ret->Func = LEDUpdate;
	ret->Params = LEDParams;
	LEDParams( led );
	return ret;

}

REGISTER_OUT_DRIVER(OutputRaw);


