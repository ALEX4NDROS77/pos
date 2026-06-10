#ifndef MODELS_ORDER_H
#define	MODELS_ORDER_H	1

#include <string>

struct Order {
	std::string sale_id;
	std::string sale_time;
	std::string vendor;
	double total_sale;
	char payment_method;
};

#endif

