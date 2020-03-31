#include "logger.h"
#include <Windows.h>

logger::logger() {
	fout.close();
}

logger::logger(std::string Name) {
	fout.close();
	DebugName = Name;
	fout.open(DebugName, std::ios::out);
}

void logger::Warn(std::string Reason) {
	std::string out = "[Warn] " + CurrentTime() + ": " + Reason;
	fout << out << std::endl;
}

void logger::Info(std::string Reason) {
	std::string out = "[Info] " + CurrentTime() + ": " + Reason;
	fout << out << std::endl;
}

void logger::Error(std::string Reason) {
	std::string out = "[Error] " + CurrentTime() + ": " + Reason;
	fout << out << std::endl;
}

void logger::Custom(std::string Prefix,std::string Reason,bool ShowTime) {
	std::string out=Prefix;
	if (ShowTime)	out += CurrentTime() + ' ';
	out += Reason;
	fout << out << std::endl;
}
std::string logger::CurrentTime() {
	SYSTEMTIME Sys;
	GetLocalTime(&Sys);
	std::string ret;
	ret = (std::to_string(Sys.wYear) + '/' + std::to_string(Sys.wMonth) + '/'
		+ std::to_string(Sys.wDay) + ' ' + std::to_string(Sys.wHour) + ':'
		+ std::to_string(Sys.wMinute) + ':' + std::to_string(Sys.wSecond)) + '.'
		+ std::to_string(Sys.wMilliseconds);
	return ret;
}