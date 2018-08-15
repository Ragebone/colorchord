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
	int total_leds; // don't need that here
	float satamp; // global Amplification setting.
};


struct rawOutput{
	float satamp;

};

// driver update method.
static void LEDUpdate(void * id, struct NoteFinder*nf)
{
	struct RawDriver * led = (struct RawDriver*)id;

	// Declare Variables.

	int totbins = nf->note_peaks;	//nf->dists;

	// Bin as in english BIN -> bucket of things.
	// declare floatArrays, length of total bins
	float binvals[totbins];
	float binvalsQ[totbins];
	float binpos[totbins];
	float totalbinval = 0;

	int i;

	// Begin of Output-Frame
	printf("frame, totBins: %d \n", totbins);

	// output: setamp
	// output: totBins

	for( i = 0; i < totbins; i++ )
	{
		// calculate values
		binpos[i] = nf->note_positions[i] / nf->freqbins;
		binvals[i] = pow( nf->note_amplitudes2[i], 1);
		binvalsQ[i] =pow( nf->note_amplitudes[i], 1);
		totalbinval += binvals[i];

		// output binPos[i]
		// output binVals[i]
		// output binValsQ[i]
		printf("%f, %f, %f \n",binpos[i], binvals[i], binvalsQ[i]);
	}

	// output: totalBinVal
	printf("totBinVal: %f -Frame END \n\n", totalbinval);
	// End Frame

}

static void LEDParams(void * id )
{
	struct RawDriver * led = (struct RawDriver*)id;
	led->satamp = 2;		RegisterValue( "satamp", PAFLOAT, &led->satamp, sizeof( led->satamp ) );
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


