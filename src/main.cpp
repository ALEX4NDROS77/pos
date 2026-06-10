#include <iostream>
#include <httplib.h>

#include <services/DatabaseService.h>
#include <controllers/AuthController.h>
#include <controllers/CartController.h>
#include <controllers/InventoryController.h>
#include <controllers/ReportController.h>
#include <utils/StaticAssets.h>

int main() {
	std::cout << "==================================" << std::endl;
	std::cout << "  POS System - Initializing..." << std::endl;
	std::cout << "==================================" << std::endl;

	if(!DatabaseService::get_instance().initialize("pos.db")) {
		std::cerr << "Failed to initialize database!" << std::endl;
		return 1;
	}

	httplib::Server server;

	StaticAssets::register_routes(server);
	AuthController::register_routes(server);
	CartController::register_routes(server);
	InventoryController::register_routes(server);
	ReportController::register_routes(server);

	std::cout << "==================================" << std::endl;
	std::cout << "  Server running on:" << std::endl;
	std::cout << "  http://localhost:8080" << std::endl;
	std::cout << "  Admin password: admin123" << std::endl;
	std::cout << "==================================" << std::endl;

	server.listen("0.0.0.0",8080);

	DatabaseService::get_instance().close();

	return 0;
}

