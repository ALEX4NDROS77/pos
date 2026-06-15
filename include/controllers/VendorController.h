#ifndef CONTROLLERS_VENDORCONTROLLER_H
#define	CONTROLLERS_VENDORCONTROLLER_H	1

#include <httplib.h>

class VendorController {
	public:
		static void register_routes(httplib::Server& server);

	private:
		static void manage_vendors(const httplib::Request& req,httplib::Response& res);
		static void add_vendor(const httplib::Request& req,httplib::Response& res);
		static void update_vendor(const httplib::Request& req,httplib::Response& res);
		static void delete_vendor(const httplib::Request& req,httplib::Response& res);
		static void toggle_vendor(const httplib::Request& req,httplib::Response& res);
};

#endif

