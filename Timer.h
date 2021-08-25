#pragma once
#include <iostream>
#include <chrono>

// Timer class, credit to The Cherno on YT
struct Timer {
	std::chrono::time_point<std::chrono::steady_clock> start, end;
	std::chrono::duration<float> duration;
	std::string name;
	float* output;

	Timer(std::string s, float* o) : name(s) {
		start = std::chrono::high_resolution_clock::now();
		output = o;
	}
	~Timer() {
		end = std::chrono::high_resolution_clock::now();
		duration = end - start;

		float ms = duration.count() * 1000.0f;
		if (output)
			*output = ms;
		std::cout << '[' << name << "]: " << ms << " ms\n";
	}
};

