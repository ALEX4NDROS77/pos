#ifndef	UTILS_TEMPLATEENGINE_H
#define	UTILS_TEMPLATEENGINE_H	1

#include <string>
#include <map>

namespace TemplateEngine {
	std::string load(const unsigned char* data,unsigned int len);
	std::string render(const std::string& tmpl,const std::map<std::string,std::string>& values);
}

#endif

