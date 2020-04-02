#pragma region Includes
#include "Ini.h"
#pragma endregion

#pragma region TerrainSort
//Class TerrainSort
TerrainSort::TerrainSort() {
	Data.clear();
}

TerrainSort::TerrainSort(std::vector<std::string> init) {
	Name = init[0];
	int count = atoi(init[1].c_str());
	Data.resize(count);
	for (register int i = 0; i < count; ++i) {
		Data[i].first = init[i * 2 + 2];
		Data[i].second = init[i * 2 + 3];
	}
}

int TerrainSort::Count() {
	return Data.size();
}

void TerrainSort::Resize(int size) {
	Data.resize(size);
	return;
}

std::pair<std::string, std::string>* TerrainSort::operator[] (int index) {
	return &Data[index];
}

std::string TerrainSort::GetName() {
	return Name;
}
#pragma endregion

#pragma region ScriptTemplate
//Class ScriptTemplate
ScriptTemplate::ScriptTemplate() {
	Data.clear();
}

ScriptTemplate::ScriptTemplate(std::vector<std::string> init) {
	int count = atoi(init[2].c_str());
	Data.resize(count + 1);
	Data[0].first = init[0];//UIName
	Data[0].second = init[1];//Name
	for (register int i = 1; i <= count; ++i) {
		Data[i].first = init[2 * i + 1];
		Data[i].second = init[2 * i + 2];
	}
}

int ScriptTemplate::Count() {
	return Data.size() - 1;
}

void ScriptTemplate::Resize(int size) {
	Data.resize(size);
	return;
}

std::pair<std::string, std::string>* ScriptTemplate::operator[] (int index) {
	return &Data[index];
}
#pragma endregion

#pragma region TeamTemplate
//Class TeamTemplate
TeamTemplate::TeamTemplate(std::vector<std::string> init) {
	
	for (register int i = 0; i < 27; ++i)
		Data[i] = init[i];
}

TeamTemplate::TeamTemplate() {
	for (register int i = 0; i < 27; ++i)
		Data[i] = "0";
}

std::string* TeamTemplate::operator[](int index) {
	return &Data[index];
}
#pragma endregion

#pragma region Ini
//Class Ini
Ini::Ini() {
	Path.clear();
}

Ini::Ini(std::string path) {
	Path = path;
}

std::string Ini::Read(std::string section, std::string key) {
	TCHAR tmp[1024];
	GetPrivateProfileString((LPCSTR)section.c_str(), (LPCSTR)key.c_str(), "", tmp, 1024, (LPCSTR)Path.c_str());
	std::string ret(tmp);
	return Split(ret, ';')[0];
}

BOOL Ini::Write(std::string section, std::string key, std::string value) {
	return WritePrivateProfileString((LPCSTR)section.c_str(), (LPCSTR)key.c_str(), (LPCSTR)value.c_str(), (LPCSTR)Path.c_str());
}

BOOL Ini::Exist() {
	std::ifstream file(Path);
	if (file.is_open())	return TRUE;
	file.close();
	return FALSE;
}

std::vector<std::string> Ini::Split(std::string str, char ch) {
	int length = str.size();
	std::vector<std::string> ret;
	std::string cur;
	for (register int i = 0; i < length; ++i) {
		if (str[i] != ch)	cur += str[i];
		else {
			ret.push_back(cur);
			cur.clear();
		}
	}
	ret.push_back(cur);
	return ret;
}

std::string& Ini::Trim(std::string& s)
{
	if (s.empty())
		return s;
	s.erase(0, s.find_first_not_of(" "));
	s.erase(s.find_last_not_of(" ") + 1);
	return s;
}
#pragma endregion