#include <utils/TemplateEngine.h>

namespace TemplateEngine {
	std::string load(const unsigned char* data,unsigned int len) {
		return std::string(reinterpret_cast<const char*>(data),len);
	}

	std::string render(const std::string& tmpl,const std::map<std::string,std::string>& values) {
		std::string out = tmpl;

		for(const auto& [key,value] : values) {
			std::string placeholder = "{{" + key + "}}";
			size_t pos = 0;
			while((pos = out.find(placeholder,pos)) != std::string::npos) {
				out.replace(pos,placeholder.length(),value);
				pos += value.length();
			}
		}

		return out;
	}
}
