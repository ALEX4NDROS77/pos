#ifndef MODELS_CARTITEM_H
#define	MODELS_CARTITEM_H	1

#include <string>

struct CartItem {
	std::string product_id;
	std::string name;
	int quantity;
	double unit_price;
};

#endif

