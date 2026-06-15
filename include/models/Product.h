#ifndef	MODELS_PRODUCT_H
#define	MODELS_PRODUCT_H	1

#include <string>

struct Product {
	std::string id;
	std::string name;
	double sale_price;
	int quantity;
	std::string imagen;
};

#endif

