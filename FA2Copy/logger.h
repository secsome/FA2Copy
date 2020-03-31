#ifndef __FA2COPY_LOGGER__
#define __FA2COPY_LOGGER__

#include <fstream>
#include <string>

class logger
{
public:
	logger();
	logger(std::string Name);
	void Warn(std::string Reason);
	void Info(std::string Reason);
	void Error(std::string Reason);
	void Custom(std::string Prefix, std::string Reason, bool ShowTime);

private:
	std::string CurrentTime();
	std::ofstream fout;
	std::string DebugName;
};

#endif