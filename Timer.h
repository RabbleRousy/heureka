// INSTRUMENTATION FRAMEWORK FOR PROFILING BY THECHERNO
// https://www.youtube.com/watch?v=xlAH4dbMVnU

#pragma once
#include <iostream>
#include <chrono>

struct ProfileResult {
	std::string name;
	long long start, end;
};

struct InstrumentationSession {
	std::string name;
};

class Instrumentor {
private:
	InstrumentationSession* currentSession;
	std::ofstream outputStream;
	int profileCount;
public:
	Instrumentor() : currentSession(NULL), profileCount(0) {}

	void BeginSession(const std::string& name, const std::string& path = "profileResults.json") {
		outputStream.open(path);
		WriteHeader();
		currentSession = new InstrumentationSession{ name };
	}

	void EndSession() {
		WriteFooter();
		outputStream.close();
		delete currentSession;
		currentSession = NULL;
		profileCount = 0;
	}

	// Writes the results of one profiling session into the output file
	void WriteProfile(const ProfileResult& result) {
		if (profileCount++ > 0) outputStream << ',';

		std::string name = result.name;
		std::replace(name.begin(), name.end(), '"', '\'');

		outputStream << "{";
		outputStream << "\"cat\":\"function\",";
		outputStream << "\"dur\":" << (result.end - result.start) << ',';
		outputStream << "\"name\":\"" << name << "\",";
		outputStream << "\"ph\":\"X\",";
		outputStream << "\"pid\":0,";
		outputStream << "\"tid\":0,";
		outputStream << "\"ts\":" << result.start;
		outputStream << "}";
		
		outputStream.flush();
	}

	void WriteHeader() {
		outputStream << "{\"otherData\": {},\"traceEvents\":[";
		outputStream.flush();
	}
	
	void WriteFooter() {
		outputStream << "]}";
		outputStream.flush();
	}

	static Instrumentor& Get() {
		static Instrumentor instance;
		return instance;
	}
};

struct InstrumentationTimer {
	std::chrono::time_point<std::chrono::high_resolution_clock> startTimePoint;
	std::string name;
	bool stopped;

	InstrumentationTimer(std::string s) : name(s), stopped(false) {
		startTimePoint = std::chrono::high_resolution_clock::now();
	}


	~InstrumentationTimer() {
		if (!stopped) Stop();
	}

	void Stop() {
		auto endTimePoint = std::chrono::high_resolution_clock::now();

		long long start = std::chrono::time_point_cast<std::chrono::microseconds>(startTimePoint).time_since_epoch().count();
		long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimePoint).time_since_epoch().count();

		Instrumentor::Get().WriteProfile({ name, start, end });

		stopped = true;
	}
};


// Enables using directive PROFILE_FUNCTION() in the whole project
// Causes that function to be profiled, if PROFILING is 1
#define PROFILING 1
#if PROFILING
#define PROFILE_SCOPE(name) InstrumentationTimer timer##__LINE__(name)
#define PROFILE_FUNCTION() PROFILE_SCOPE(__FUNCTION__)
#else
#define PROFILE_SCOPE(name)
#endif
