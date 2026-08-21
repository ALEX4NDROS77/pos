#include <utils/AppConfig.h>
#include <utils/Logger.h>
#include <fstream>

namespace {
	std::string trim(const std::string& s) {
		size_t start = s.find_first_not_of(" \t\r\n");
		if(start == std::string::npos) return "";
		size_t end = s.find_last_not_of(" \t\r\n");
		return s.substr(start,end - start + 1);
	}
}

AppConfig& AppConfig::get_instance() {
	static AppConfig instance;
	return instance;
}

void AppConfig::load(const std::string& path) {
	std::ifstream file(path);
	if(!file.is_open()) {
		LOG_INFO("AppConfig::load - no branding config at '" + path + "', using defaults");
		return;
	}

	std::string line;
	while(std::getline(file,line)) {
		std::string trimmed = trim(line);
		if(trimmed.empty() || trimmed[0] == '#') continue;

		size_t eq = trimmed.find('=');
		if(eq == std::string::npos) continue;

		std::string key = trim(trimmed.substr(0,eq));
		std::string value = trim(trimmed.substr(eq + 1));
		if(value.empty()) continue;

		if(key == "business_name") m_business_name = value;
		else if(key == "tagline") m_tagline = value;
		else if(key == "currency_symbol") m_currency_symbol = value;
		else if(key == "primary_color") m_primary_color = value;
		else if(key == "accent_color") m_accent_color = value;
		else if(key == "bg_color") m_bg_color = value;
		else if(key == "logo_path") m_logo_path = value;
		else if(key == "mascot_path") m_mascot_path = value;
		else if(key == "favicon_path") m_favicon_path = value;
		else LOG_WARNING("AppConfig::load - unknown key '" + key + "' in '" + path + "'");
	}

	LOG_INFO("AppConfig::load - branding config loaded from '" + path + "'");
}
