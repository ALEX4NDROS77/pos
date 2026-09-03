#include <controllers/TransferController.h>
#include <services/SessionService.h>
#include <services/TransferService.h>
#include <utils/Logger.h>
#include <views/HtmlTemplates.h>

void TransferController::register_routes(httplib::Server& server) {
	server.Get("/transfers",transfer_page);
	server.Post("/transfers/create",create_transfer);
}

void TransferController::transfer_page(const httplib::Request& req,httplib::Response& res) {
	auto* session = SessionService::get_instance().require_role(req,res,"admin");
	if(!session) return;

	res.set_content(HtmlTemplates::transfer_page(session),"text/html");
}

void TransferController::create_transfer(const httplib::Request& req,httplib::Response& res) {
	auto* session = SessionService::get_instance().require_role(req,res,"admin");
	if(!session) return;

	std::string product_id = req.get_param_value("product_id");
	std::string origin_id = req.get_param_value("origin_id");
	std::string destination_id = req.get_param_value("destination_id");
	std::string quantity_str = req.get_param_value("quantity");

	int quantity = 0;
	try {
		quantity = std::stoi(quantity_str);
	} catch(const std::exception& e) {
		LOG_ERROR("TransferController::create_transfer - Failed to parse quantity - " + std::string(e.what()));
		res.set_content(HtmlTemplates::transfer_page(session,"Error: cantidad invalida."),"text/html");
		return;
	}

	LOG_INFO("TransferController::create_transfer - product='" + product_id + "', origin='" + origin_id + "', destination='" + destination_id + "', quantity=" + std::to_string(quantity));

	auto result = TransferService::get_instance().transfer(product_id,origin_id,destination_id,quantity,session->username);

	std::string message;
	if(!result) {
		message = "Error al procesar la transferencia.";
	} else if(result->find("TRF-") != std::string::npos) {
		message = "Transferencia completada!";
	} else {
		message = *result;
	}

	res.set_content(HtmlTemplates::transfer_page(session,message),"text/html");
}
