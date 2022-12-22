#include <iostream>
#include "olcNoiseMaker.h"

using namespace std;


double MakeNoise(double dTime) // dTime = time passed since start of program
{

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

	return 0;
}