#include <httplib.h>
#include <iostream>

#include <controllers/AuthController.h>
#include <controllers/CartController.h>
#include <controllers/InventoryController.h>
#include <controllers/IsleController.h>
#include <controllers/MonitoringController.h>
#include <controllers/ReportController.h>
#include <controllers/TicketController.h>
#include <controllers/TransferController.h>
#include <controllers/VendorController.h>
#include <services/DatabaseService.h>
#include <utils/AppConfig.h>
#include <utils/Logger.h>
#include <utils/StaticAssets.h>

int main() {
	Logger::get_instance().set_level(LogLevel::DEBUG);
	Logger::get_instance().set_file("pos.log");

	LOG_INFO("==================================");
	LOG_INFO("  POS System - Initializing...");
	LOG_INFO("==================================");

	AppConfig::get_instance().load("branding.conf");

	if(!DatabaseService::get_instance().initialize("pos.db")) {
		LOG_ERROR("Failed to initialize database!");
		return 1;
	}
	LOG_INFO("Database initialized successfully");

	httplib::Server server;

	StaticAssets::register_routes(server);
	AuthController::register_routes(server);
	CartController::register_routes(server);
	InventoryController::register_routes(server);
	ReportController::register_routes(server);
	VendorController::register_routes(server);
	TicketController::register_routes(server);
	IsleController::register_routes(server);
	TransferController::register_routes(server);
	MonitoringController::register_routes(server);

	LOG_INFO("==================================");
	LOG_INFO("  Server running on:");
	LOG_INFO("  http://localhost:8080");
	LOG_INFO("==================================");
	LOG_INFO("	Credentials:");
	LOG_INFO("	- Admin: admin123");
	LOG_INFO("	- Vendors: vendedor1/1234, vendedor2/5678");
	LOG_INFO("==================================");

	server.listen("0.0.0.0",8080);

	LOG_INFO("Server shutting down...");
	DatabaseService::get_instance().close();
	LOG_INFO("Database closed. Goodbye!");

	return 0;
}
