#ifndef	MODELS_SESSION_H
#define	MODELS_SESSION_H	1

#include <string>
#include <vector>
#include <models/CartItem.h>

struct Session {
	std::string username;
	std::string role;	// "admin" or "vendor"
	std::string isla_id;	// assigned isle for vendors; empty for admin/unassigned
	std::vector<CartItem> cart;
};

#endif

