#include <iostream>
#include "olcNoiseMaker.h"

using namespace std;


double MakeNoise(double dTime) // dTime = time passed since start of program
{
	// amplitude (0.5) * sin(frequency in hrtz * dTime)
	// 440 hrtz should be the note A
	// sin takes angular velocity, not hrtz, so conversion (value * 2 * pi)
	// "at given point in time (dTime), what should our sin wave look like"
	return 0.5 * sin(440.0 * (2 * 3.14159) * dTime);
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

	}



	return 0;
}