#ifndef	CONTROLLER_INVENTORYCONTROLLER_H
#define	CONTROLLER_INVENTORYCONTROLLER_H	1

#include <httplib.h>

class InventoryController {
	public:
		static void register_routes(httplib::Server& server);

	private:
		static void view_inventory(const httplib::Request& req,httplib::Response& res);
		static void manage_inventory(const httplib::Request& req,httplib::Response& res);
		static void add_product(const httplib::Request& req,httplib::Response& res);
		static void update_product(const httplib::Request& req,httplib::Response& res);
		static void delete_product(const httplib::Request& req,httplib::Response& res);
};

#endif

