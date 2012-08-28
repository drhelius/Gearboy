
// Use Basic_Gb_Apu to play random tones. Writes output to sound file "out.wav".

#include "Basic_Gb_Apu.h"
#include "Wave_Writer.h"

#include <stdlib.h>

static Basic_Gb_Apu apu;

// "emulate" 1/60 second of sound
static void emulate_frame()
{
	static int delay;
	if ( --delay <= 0 )
	{
		delay = 12;
		
		// Start a new random tone
		int chan = rand() & 0x11;
		apu.write_register( 0xff26, 0x80 );
		apu.write_register( 0xff25, chan ? chan : 0x11 );
		apu.write_register( 0xff11, 0x80 );
		int freq = (rand() & 0x3ff) + 0x300;
		apu.write_register( 0xff13, freq & 0xff );
		apu.write_register( 0xff12, 0xf1 );
		apu.write_register( 0xff14, (freq >> 8) | 0x80 );
	}
	
	// Generate 1/60 second of sound into APU's sample buffer
	apu.end_frame();
}

int main( int argc, char** argv )
{
	long const sample_rate = 44100;
	
	// Set sample rate and check for out of memory error
	if ( apu.set_sample_rate( sample_rate ) )
		return EXIT_FAILURE;
	
	// Generate a few seconds of sound into wave file
	Wave_Writer wave( sample_rate );
	wave.stereo( true );
	for ( int n = 60 * 4; n--; )
	{
		// Simulate emulation of 1/60 second frame
		emulate_frame();
		
		// Samples from the frame can now be read out of the apu, or
		// allowed to accumulate and read out later. Use samples_avail()
		// to find out how many samples are currently in the buffer.
		
		int const buf_size = 2048;
		static blip_sample_t buf [buf_size];
		
		// Play whatever samples are available
		long count = apu.read_samples( buf, buf_size );
		wave.write( buf, count );
	}
	
	return 0;
}

