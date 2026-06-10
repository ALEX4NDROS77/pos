#ifndef	MODELS_SALE_H
#define	MODELS_SALE_H	1

#include <string>

struct Sale {
	std::string sale_id;
	std::string order_id;
	std::string product_id;
	int sale_quantity;
	double unit_price;
};

#endif

