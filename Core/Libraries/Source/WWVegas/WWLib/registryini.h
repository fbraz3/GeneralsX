#pragma once

#include <string>
#include <vector>

namespace RegistryIni {
	const char *CurrentUserRoot();
	const char *LocalMachineRoot();

	bool ReadString(const char *root, const char *path, const char *key, std::string &value);
	bool ReadUnsignedInt(const char *root, const char *path, const char *key, unsigned int &value);
	bool WriteString(const char *root, const char *path, const char *key, const char *value);
	bool WriteUnsignedInt(const char *root, const char *path, const char *key, unsigned int value);
	bool SectionExists(const char *root, const char *path);
	bool DeleteValue(const char *root, const char *path, const char *key);
	bool ClearSection(const char *root, const char *path);
	bool ListValues(const char *root, const char *path, std::vector<std::string> &keys);
}