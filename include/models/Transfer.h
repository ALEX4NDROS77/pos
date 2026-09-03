#ifndef MODELS_TRANSFER_H
#define	MODELS_TRANSFER_H	1

#include <string>

struct Transfer {
	std::string id;
	std::string product_id;
	std::string product_name;
	std::string origin_id;
	std::string destination_id;
	int quantity;
	std::string username;
	std::string fecha;
};

#endif

