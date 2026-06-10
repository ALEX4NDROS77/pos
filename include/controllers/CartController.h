#ifndef CONTROLLER_CARTCONTROLLER_H
#define	CONTROLLER_CARTCONTROLLER_H	1

#include <httplib.h>

class CartController {
	public:
		static void register_routes(httplib::Server& server);

	private:
		static void cart_page(const httplib::Request& req,httplib::Response& res);
		static void add_to_cart(const httplib::Request& req,httplib::Response& res);
		static void update_cart_item(const httplib::Request& req,httplib::Response& res);
		static void remove_from_cart(const httplib::Request& req,httplib::Response& res);
		static void clear_cart(const httplib::Request& req,httplib::Response& res);
		static void confirm_page(const httplib::Request& req,httplib::Response& res);
		static void checkout(const httplib::Request& req,httplib::Response& res);
};

#endif

