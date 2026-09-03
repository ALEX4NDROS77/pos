#ifndef MODELS_VENDOR_H
#define MODELS_VENDOR_H	1

#include <string>

struct Vendor {
	std::string id;
	std::string nombre;
	std::string password;
	bool activo;
	std::string isla_id;
};

#endif

