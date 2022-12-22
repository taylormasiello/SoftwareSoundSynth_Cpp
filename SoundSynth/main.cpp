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


	while (1)
	{
		// Add a keyboard

		// testing if highest bit of A key is present; if yes, it's pressed
		if (GetAsyncKeyState('A') & 0x8000)
		{
			// 440 hrtz should be the note A
			dFrequencyOutput = 440.0;
		}
		else
		{
			dFrequencyOutput = 0.0;
		}
	}



	return 0;
}