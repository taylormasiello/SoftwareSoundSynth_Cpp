#include <iostream>
#include "olcNoiseMaker.h"

using namespace std;

// Converts frequency (Hz) to angular velocity
double w(double dHertz)
{
	return dHertz * 2.0 * PI;
}

// oscillator selection function takes in frequency, current time to know where in osc cycle we are, and type of oscillation
double osc(double dHertz, double dTime, int nType)
{
	// depending on type of osc; can choose a different function for each, if invalid type return 0 i.e. no sound output
	switch (nType)
	{

	// standard sine wave
	case 0:
		return sin(w(dHertz) * dTime);

	// square sine wave
	case 1:
		return sin(w(dHertz) * dTime) > 0.0 ? 1.0 : -1.0;

	// triangle sine wave; arcsine of the sine of the standard sine wave
	case 2:
		return asin(sin(w(dHertz) * dTime)) * (2.0 / PI);

	// sawtooth sine wave, looks like edge of saw
	// quick incremental increases to a slope, then a sharp fall
	// math description: sum up all multiples of a particular frequency of a sine wave
	// comp: uses mod function (computer rounding, integer mathmatics) to simulate addition of sine waves (more efficient)
	// comp version saves CPU resources while providing a "perfect" piecewise linear approximation 
	case 3: // MATH sawtooth wave (analogue / warm, not a straight line, more interesting sound /slow)
	{
		double dOutput = 0.0;

		for (double n = 1.0; n < 100.0; n++)
			dOutput += (sin(n * w(dHertz) * dTime)) / n;
		
		return dOutput * (2.0 / PI);
	}

	// as time progresses along x axis, we mod it with our frequency
	case 4: // COMP sawtooth wave (optimized / harsh sound, approximated straight line, not incremented / fast)
		return (2.0 / PI) * (dHertz * PI * fmod(dTime, 1.0 / dHertz) - (PI / 2.0));

	// Pseudo Random Noise
	case 5:
		return 2.0 * ((double)rand() / (double)RAND_MAX) - 1.0; //random # between -1.0 and 1.0

	default:
		return 0.0;
	}
}

// Global synthesizer variables
atomic<double> dFrequencyOutput = 0.0;			// dominant output frequency of instrument, i.e. the note
double dOctaveBaseFrequency = 110.0; // A2		// frequency of octave represented by keyboard
double d12thRootOf2 = pow(2.0, 1.0 / 12.0);		// assuming western 12 notes per ocatve

// Function used by olcNoiseMaker to generate sound waves
// Returns amplitude (-1.0 to +1.0) as a function of time
double MakeNoise(double dTime)
{
	double dOutput = osc(dFrequencyOutput, dTime, 3);

	return dOutput * 0.4; // Master Volume

}


int main()
{
	wcout << "Tay's Practice Sound Synth" << endl;
	
	// Get all sound hardware
	vector<wstring> devices = olcNoiseMaker<short>::Enumerate();

	// Display findings
	for (auto d : devices) wcout << "Found Output Device: " << d << endl;
	wcout << "Using Device: " << devices[0] << endl;

	// Display a keyboard
	wcout << endl <<
		"|   |   |   |   |   | |   |   |   |   | |   | |   |   |   |" << endl <<
		"|   | S |   |   | F | | G |   |   | J | | K | | L |   |   |" << endl <<
		"|   |___|   |   |___| |___|   |   |___| |___| |___|   |   |__" << endl <<
		"|     |     |     |     |     |     |     |     |     |     |" << endl <<
		"|  Z  |  X  |  C  |  V  |  B  |  N  |  M  |  ,  |  .  |  /  |" << endl <<
		"|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|" << endl << endl;

	// Create sound machine
	olcNoiseMaker<short> sound(devices[0], 44100, 1, 8, 512);

	// Link noise function with sound machine
	sound.SetUserFunction(MakeNoise);

	// Sit in loop, capturing keyboard state changes and modify synthesizer output accordingly
	int nCurrentKey = -1;
	bool bKeyPressed = false;

	while (1)
	{
		bKeyPressed = false;
		for (int k = 0; k < 16; k++)
		{
			if (GetAsyncKeyState((unsigned char)("ZSXCFVGBNJMK\xbcL\xbe\xbf"[k])) & 0x8000)
			{
				if (nCurrentKey != k)
				{
					dFrequencyOutput = dOctaveBaseFrequency * pow(d12thRootOf2, k);
					wcout << "\nNote On: " << sound.GetTime() << "s" << dFrequencyOutput << "Hz";
					nCurrentKey = k;
				}

				bKeyPressed = true;
			}
		}

		if (!bKeyPressed)
		{
			{
				if (nCurrentKey != -1)
				{
					wcout << "\rNote Off: " << sound.GetTime() << "s                        ";
					nCurrentKey = -1;
				}
			}
			
			dFrequencyOutput = 0.0;
		}

	}



	return 0;
}