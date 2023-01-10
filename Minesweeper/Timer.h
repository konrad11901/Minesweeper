#pragma once

class Timer {
public:
	Timer();

	void StartCounter();
	double GetTime();
private:
	LARGE_INTEGER start;
	double frequency;
};
