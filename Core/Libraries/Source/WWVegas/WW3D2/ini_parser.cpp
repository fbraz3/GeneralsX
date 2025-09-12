#include "ini_parser.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <map>

IniParser::IniParser() {
}

IniParser::~IniParser() {
}

bool IniParser::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    std::string line;
    std::string currentSection;
    
    while (std::getline(file, line)) {
        line = trim(line);
        
        if (line.empty() || isComment(line)) {
            continue;
        }
        
        std::string sectionName;
        if (isSection(line, sectionName)) {
            currentSection = toLower(sectionName);
            continue;
        }
        
        std::string key, value;
        if (isKeyValue(line, key, value)) {
            if (!currentSection.empty()) {
                sections_[currentSection][toLower(key)] = parseValue(value);
            }
        }
    }
    
    file.close();
    return true;
}

bool IniParser::saveToFile(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    for (const auto& section : sections_) {
        file << "[" << section.first << "]\n";
        
        for (const auto& keyValue : section.second) {
            file << keyValue.first << "=" << escapeValue(keyValue.second) << "\n";
        }
        
        file << "\n"; // Empty line between sections
    }
    
    file.close();
    return true;
}

std::string IniParser::getValue(const std::string& section, const std::string& key, const std::string& defaultValue) {
    std::string lowerSection = toLower(section);
    std::string lowerKey = toLower(key);
    
    auto sectionIt = sections_.find(lowerSection);
    if (sectionIt != sections_.end()) {
        auto keyIt = sectionIt->second.find(lowerKey);
        if (keyIt != sectionIt->second.end()) {
            return keyIt->second;
        }
    }
    
    return defaultValue;
}

void IniParser::setValue(const std::string& section, const std::string& key, const std::string& value) {
    sections_[toLower(section)][toLower(key)] = value;
}

unsigned int IniParser::getUnsignedInt(const std::string& section, const std::string& key, unsigned int defaultValue) {
    std::string value = getValue(section, key, "");
    if (!value.empty()) {
        try {
            return static_cast<unsigned int>(std::stoul(value));
        } catch (...) {
            // Fall through to return default
        }
    }
    return defaultValue;
}

void IniParser::setUnsignedInt(const std::string& section, const std::string& key, unsigned int value) {
    setValue(section, key, std::to_string(value));
}

bool IniParser::hasSection(const std::string& section) {
    return sections_.find(toLower(section)) != sections_.end();
}

bool IniParser::hasKey(const std::string& section, const std::string& key) {
    std::string lowerSection = toLower(section);
    std::string lowerKey = toLower(key);
    
    auto sectionIt = sections_.find(lowerSection);
    if (sectionIt != sections_.end()) {
        return sectionIt->second.find(lowerKey) != sectionIt->second.end();
    }
    
    return false;
}

void IniParser::removeKey(const std::string& section, const std::string& key) {
    std::string lowerSection = toLower(section);
    std::string lowerKey = toLower(key);
    
    auto sectionIt = sections_.find(lowerSection);
    if (sectionIt != sections_.end()) {
        sectionIt->second.erase(lowerKey);
    }
}

void IniParser::removeSection(const std::string& section) {
    sections_.erase(toLower(section));
}

void IniParser::clear() {
    sections_.clear();
}

std::string IniParser::trim(const std::string& str) {
    const char* whitespace = " \t\r\n";
    size_t start = str.find_first_not_of(whitespace);
    if (start == std::string::npos) {
        return "";
    }
    
    size_t end = str.find_last_not_of(whitespace);
    return str.substr(start, end - start + 1);
}

std::string IniParser::toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

std::string IniParser::parseValue(const std::string& value) {
    std::string trimmedValue = trim(value);
    
    // Handle quoted values
    if (trimmedValue.length() >= 2) {
        if ((trimmedValue.front() == '"' && trimmedValue.back() == '"') ||
            (trimmedValue.front() == '\'' && trimmedValue.back() == '\'')) {
            return trimmedValue.substr(1, trimmedValue.length() - 2);
        }
    }
    
    return trimmedValue;
}

std::string IniParser::escapeValue(const std::string& value) {
    // If value contains spaces or special characters, quote it
    if (value.find(' ') != std::string::npos ||
        value.find('\t') != std::string::npos ||
        value.find('=') != std::string::npos ||
        value.find('[') != std::string::npos ||
        value.find(']') != std::string::npos) {
        return "\"" + value + "\"";
    }
    
    return value;
}

bool IniParser::isComment(const std::string& line) {
    return line.empty() || line[0] == '#' || line[0] == ';';
}

bool IniParser::isSection(const std::string& line, std::string& sectionName) {
    if (line.length() >= 3 && line[0] == '[' && line.back() == ']') {
        sectionName = line.substr(1, line.length() - 2);
        return true;
    }
    return false;
}

bool IniParser::isKeyValue(const std::string& line, std::string& key, std::string& value) {
    size_t equalsPos = line.find('=');
    if (equalsPos != std::string::npos && equalsPos > 0) {
        key = trim(line.substr(0, equalsPos));
        value = trim(line.substr(equalsPos + 1));
        return true;
    }
    return false;
}