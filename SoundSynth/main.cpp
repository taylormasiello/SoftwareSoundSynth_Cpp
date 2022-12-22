#include <iostream>
#include "olcNoiseMaker.h"

using namespace std;


atomic<double> dFrequencyOutput = 0.0;


double MakeNoise(double dTime) // dTime = time passed since start of program
{
	// amplitude (0.5) * sin(frequency in hrtz * dTime)
	// sin takes angular velocity, not hrtz, so conversion (value * 2 * pi)
	// dOutput captures sin wave shape at given point in time (dTime)
	double dOutput = 1.0 * sin(dFrequencyOutput * (2 * 3.14159) * dTime);

	// takes dOutput sin wave and makes it into square wave
	// we set amplitude implicitly as can only have a + or - in a square wave
	if (dOutput > 0.0)
		return 0.2;
	else
		return -0.2;

}


int main()
{
	wcout << "Tay's Practice Sound Synth" << endl;
	
	// Get all sound hardware
	vector<wstring> devices = olcNoiseMaker<short>::Enumerate();

	// Display findings
	for (auto d : devices) wcout << "Found Output Device: " << d << endl;

	// Create sound machine
	olcNoiseMaker<short> sound(devices[0], 44100, 1, 8, 512);

	// Link noise function with sound machine
	// using a function pointer, as noise maker class constantly running in bckgrnd, thus dTime continuously increasing
	// will return a value between 0 and 1 to represent where the speaker cone should be (relative to time)
	sound.SetUserFunction(MakeNoise);

	// every doubling frequency moves up 1 octave; 12 notes in an octave
	// can't simply divide frequency by 12 as it doubles each time
	// instead using power of 2 to the 12th root
	double dOctaveBaseFrequency = 110.0; // A2
	double d12thRootOf2 = pow(2.0, 1.0 / 12.0);


	while (1)
	{
		// Add a keyboard like a piano

		bool bKeyPressed = false;
		for (int k = 0; k < 15; k++)
		{
			// testing if highest bit of a list key is present; if yes, it's pressed
			// note mapping: Z is a; S is aSharp; X is b; C is c; F is cSharp
			// V is d; G is dSharp; B is e; N is f; J is fSharp; M is g; K is gSharp
			// unsure what notes , (xbc) or L or . (xbe) are
			if (GetAsyncKeyState((unsigned char)("ZSXCFVGBNJMK\xbcL\xbe"[k])) & 0x8000)
			{
				// 440 hrtz should be the note A
				dFrequencyOutput = dOctaveBaseFrequency * pow(d12thRootOf2, k);
				bKeyPressed = true;
			}
		}

		if (!bKeyPressed)
		{
			dFrequencyOutput = 0.0;
		}

	}



	return 0;
}