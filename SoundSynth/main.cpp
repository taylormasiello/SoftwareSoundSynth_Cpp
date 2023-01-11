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

// ADSR = Attack time, Decay time, Sustain amplitude, Release time
struct sEnvelopeADSR
{
	double dAttackTime;
	double dDecayTime;
	double dReleaseTime;

	double dSustainAmplitude;
	double dStartAmplitude;

	double dTriggerOnTime;
	double dTriggerOffTime;

	bool bNoteOn;

	sEnvelopeADSR()
	{
		dAttackTime = 0.100; // 100 milliseconds
		dDecayTime = 0.01;
		dStartAmplitude = 1.0;
		dSustainAmplitude = 0.8;
		dReleaseTime = 0.200;
		dTriggerOnTime = 0.0;
		dTriggerOffTime = 0.0;
		bNoteOn = false;
	}

	// envelope can be indexed at any point in time by our MakeNoise function
	// need to return the amplitude of this envelope for a given time
	double GetAmplitude(double dTime) // dTime = real ("wall") time of music 
	{
		double dAmplitude = 0.0;
		double dLifeTime = dTime - dTriggerOnTime; // turns dTime into an index into the lifeTime of this envelope

		// 2 phases, key down and key released
		if (bNoteOn)
		{
			// ADS - Attack, Decay, Sustain (key down)

			// Attack
			if (dLifeTime <= dAttackTime) //in attack phase
				dAmplitude = (dLifeTime / dAttackTime) * dStartAmplitude; //normalizes attack time; creating a value between 0 and 1
				//if lifeTime < attackTime (ex 0/5), after 4 secs, will be 4/5, eventually will be 5/5 or 1 (thus going from 0 amplitude to 100%)

			// Decay 
			if (dLifeTime > dAttackTime && dLifeTime <= (dAttackTime + dDecayTime)) // start to end of decay phase
				dAmplitude = ((dLifeTime - dAttackTime) / dDecayTime) * (dSustainAmplitude - dStartAmplitude) + dStartAmplitude; 
				//index 0-1 of how long into decay time, with gradiant defined by susntainAmp - startAmp (sustainAmp usually lower than startAmp)

			// Sustain
			if (dLifeTime > (dAttackTime + dDecayTime)) //anytime after attack and decay phases, and key is down, we are in our sustain phase
			{
				dAmplitude = dSustainAmplitude; //just output a solidary volume
			}

		}		
		else
		{
			// R - Release (key released)
			dAmplitude = ((dTime - dTriggerOffTime) / dReleaseTime) * (0.0 - dSustainAmplitude) + dSustainAmplitude; // 0-1 value for release phase
		}

		// epsilon value check; stops signals coming out of envelope that are so low we're not concerned with them, can't hear them
		// can also cause problems not having these values set to 0, if they're below a certain value
		if (dAmplitude <= 0.0001)
		{
			dAmplitude = 0;
		}


		return dAmplitude;
	}

	// to make easier to use, NoteOn and NoteOff methods to capture time of each
	void NoteOn(double dTimeOn)
	{
		dTriggerOnTime = dTimeOn;
		bNoteOn = true;
	}

	void NoteOff(double dTimeOff)
	{
		dTriggerOffTime = dTimeOff;
		bNoteOn = false;
	}

};


// Global synthesizer variables
atomic<double> dFrequencyOutput = 0.0;			// dominant output frequency of instrument, i.e. the note
double dOctaveBaseFrequency = 110.0; // A2		// frequency of octave represented by keyboard
double d12thRootOf2 = pow(2.0, 1.0 / 12.0);		// assuming western 12 notes per ocatve
sEnvelopeADSR envelope;

// Function used by olcNoiseMaker to generate sound waves
// Returns amplitude (-1.0 to +1.0) as a function of time
double MakeNoise(double dTime)
{
	double dOutput = envelope.GetAmplitude(dTime) * //now modulating the addition of 2 different frequencies (more textured instrumental sound)
	(
		+ osc(dFrequencyOutput * 0.5, dTime, 3) //envelope gives amplitude at a point in time; sawtooth generator 50% frequency
		+ osc(dFrequencyOutput * 1.0, dTime, 1) //standard sine wave generator 100% frequency 
	);
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
		"|   |   |   |   |   | |   |   |   |   | |   | |   |   |   |  " << endl <<
		"|   | S |   |   | F | | G |   |   | J | | K | | L |   |   |  " << endl <<
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
					envelope.NoteOn(sound.GetTime()); //links envelope with when note is pressed, using current time of system
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
					envelope.NoteOff(sound.GetTime()); //links envelope with when note is released, using current time of system
					nCurrentKey = -1;
				}
			}
		}

	}



	return 0;
}