#include <sstream>
#include <views/HtmlTemplates.h>
#include <services/VendorService.h>
#include <services/InventoryService.h>
#include <services/CartService.h>
#include <services/SalesService.h>
#include <services/IsleService.h>
#include <services/TransferService.h>
#include <utils/AppConfig.h>
#include <utils/TemplateEngine.h>

#include <layout_html.h>
#include <login_html.h>
#include <admin_login_html.h>
#include <vendor_login_html.h>
#include <homepage_html.h>
#include <cart_html.h>
#include <cart_confirm_html.h>
#include <cart_confirm_message_html.h>
#include <inventory_view_html.h>
#include <inventory_manage_html.h>
#include <sales_report_html.h>
#include <vendor_manage_html.h>

#include <alert_danger_html.h>
#include <alert_success_html.h>
#include <alert_info_html.h>
#include <vendor_option_html.h>
#include <admin_menu_items_html.h>
#include <ticket_empty_html.h>
#include <ticket_mini_html.h>
#include <ticket_item_html.h>
#include <product_image_html.h>
#include <product_image_placeholder_html.h>
#include <product_card_html.h>
#include <cart_row_html.h>
#include <cart_table_html.h>
#include <order_row_html.h>
#include <manage_inventory_link_html.h>
#include <inventory_view_row_html.h>
#include <product_thumb_html.h>
#include <product_thumb_placeholder_html.h>
#include <inventory_manage_row_html.h>
#include <vendor_filter_html.h>
#include <vendor_filter_link_html.h>
#include <product_filter_html.h>
#include <product_filter_link_html.h>
#include <sales_order_row_html.h>
#include <sales_report_body_html.h>
#include <vendor_active_html.h>
#include <vendor_inactive_html.h>
#include <vendor_row_html.h>
#include <isle_manage_html.h>
#include <isle_row_html.h>
#include <isla_option_html.h>
#include <transfer_html.h>
#include <transfer_row_html.h>
#include <transfer_product_card_html.h>
#include <monitoring_html.h>
#include <monitoring_isle_card_html.h>
#include <monitoring_inventory_row_html.h>

namespace HtmlTemplates {
	namespace {
		std::string render(const unsigned char* data,unsigned int len,const std::map<std::string,std::string>& values) {
			return TemplateEngine::render(TemplateEngine::load(data,len),values);
		}

		std::string load(const unsigned char* data,unsigned int len) {
			return TemplateEngine::load(data,len);
		}

		std::string build_isla_options(const std::vector<Isla>& isles,const std::string& selected_id,bool include_none,bool include_almacen) {
			std::string options;
			if(include_none) {
				options += render(isla_option_html,isla_option_html_len,{
					{"ID",""},
					{"NOMBRE","Sin isla"},
					{"SELECTED",selected_id.empty() ? "selected" : ""},
				});
			}
			if(include_almacen) {
				options += render(isla_option_html,isla_option_html_len,{
					{"ID",TransferService::ALMACEN_ID},
					{"NOMBRE","Almacen Central"},
					{"SELECTED",selected_id == TransferService::ALMACEN_ID ? "selected" : ""},
				});
			}
			for(const auto& isla : isles) {
				options += render(isla_option_html,isla_option_html_len,{
					{"ID",isla.id},
					{"NOMBRE",isla.nombre},
					{"SELECTED",selected_id == isla.id ? "selected" : ""},
				});
			}
			return options;
		}

		std::string stock_class_for(int quantity) {
			if(quantity <= 0) return "stock-critical";
			if(quantity <= 5) return "stock-warning";
			return "stock-ok";
		}
	}

	std::string wrap_html(const std::string& title, const std::string& content) {
		return render(layout_html,layout_html_len,{
			{"TITLE",title + " - " + AppConfig::get_instance().business_name()},
			{"CONTENT",content},
		});
	}

	std::string format_price(double price) {
		std::ostringstream oss;
		oss << price;
		return oss.str();
	}

	std::string format_currency(double price) {
		return AppConfig::get_instance().currency_symbol() + format_price(price);
	}

	std::string login_page(const std::string& error) {
		std::string error_block = error.empty() ? "" : render(alert_danger_html,alert_danger_html_len,{{"MESSAGE",error}});

		return wrap_html("Login",render(login_html,login_html_len,{
			{"TAGLINE",AppConfig::get_instance().tagline()},
			{"ERROR_BLOCK",error_block},
		}));
	}

	std::string vendor_login_page(const std::string& error) {
		auto vendors = VendorService::get_instance().get_all_vendors();

		std::string options;
		for(const auto& v : vendors) {
			if(v.activo) {
				options += render(vendor_option_html,vendor_option_html_len,{{"NOMBRE",v.nombre}});
			}
		}

		std::string error_block = error.empty() ? "" : render(alert_danger_html,alert_danger_html_len,{{"MESSAGE",error}});

		return wrap_html("Vendedor",render(vendor_login_html,vendor_login_html_len,{
			{"VENDOR_OPTIONS",options},
			{"ERROR_BLOCK",error_block},
		}));
	}

	std::string admin_login_page(const std::string& error) {
		std::string error_block = error.empty() ? "" : render(alert_danger_html,alert_danger_html_len,{{"MESSAGE",error}});

		return wrap_html("Admin Login",render(admin_login_html,admin_login_html_len,{
			{"ERROR_BLOCK",error_block},
		}));
	}

	std::string homepage(Session* session,const std::vector<Ticket>& tickets) {
		std::string admin_items = session->role == "admin" ? load(admin_menu_items_html,admin_menu_items_html_len) : "";

		std::string ticket_list;
		if(tickets.empty()) {
			ticket_list = load(ticket_empty_html,ticket_empty_html_len);
		} else {
			for(const auto& ticket : tickets) {
				std::string status_class = (ticket.estado == "PENDIENTE") ? "pendiente" : "en_proceso";
				std::string badge_class = (ticket.estado == "PENDIENTE") ? "status-pendiente" : "status-en_proceso";

				std::string metodo_str;
				switch(ticket.metodo_pago) {
					case 'E': metodo_str = "Efectivo"; break;
					case 'T': metodo_str = "Tarjeta"; break;
					case 'C': metodo_str = "Cortesia"; break;
					default: metodo_str = "?"; break;
				}

				std::string items;
				for(const auto& item : ticket.items) {
					items += render(ticket_item_html,ticket_item_html_len,{
						{"NAME",item.name},
						{"QTY",std::to_string(item.quantity)},
					});
				}

				ticket_list += render(ticket_mini_html,ticket_mini_html_len,{
					{"STATUS_CLASS",status_class},
					{"BADGE_CLASS",badge_class},
					{"VENDOR_NAME",ticket.vendor_name},
					{"METODO",metodo_str},
					{"ESTADO",ticket.estado},
					{"ITEMS",items},
					{"TICKET_ID",ticket.id},
				});
			}
		}

		return wrap_html("Inicio",render(homepage_html,homepage_html_len,{
			{"BUSINESS_NAME",AppConfig::get_instance().business_name()},
			{"USERNAME",session->username},
			{"ADMIN_MENU_ITEMS",admin_items},
			{"TICKET_COUNT",std::to_string(tickets.size())},
			{"TICKET_LIST",ticket_list},
		}));
	}

	std::string cart_page(Session* session, const std::string& message) {
		auto products = (session->role == "vendor")
			? InventoryService::get_instance().get_products_with_isla_stock(session->isla_id)
			: InventoryService::get_instance().get_all_products();

		std::string product_cards;
		std::string stock_json = "{";
		std::string price_json = "{";
		std::string name_json = "{";
		bool first = true;

		for(const auto& p : products) {
			std::string image_cell = !p.imagen.empty()
				? render(product_image_html,product_image_html_len,{{"SRC",p.imagen},{"ALT",p.name}})
				: load(product_image_placeholder_html,product_image_placeholder_html_len);

			product_cards += render(product_card_html,product_card_html_len,{
				{"OUT_CLASS",p.quantity <= 0 ? " out-of-stock" : ""},
				{"ID",p.id},
				{"IMAGE",image_cell},
				{"NAME",p.name},
				{"PRICE",format_currency(p.sale_price)},
				{"STOCK",std::to_string(p.quantity)},
			});

			if(!first) { stock_json += ","; price_json += ","; name_json += ","; }
			stock_json += "\"" + p.id + "\":" + std::to_string(p.quantity);
			price_json += "\"" + p.id + "\":" + format_price(p.sale_price);
			name_json += "\"" + p.id + "\":\"" + p.name + "\"";
			first = false;
		}

		stock_json += "}";
		price_json += "}";
		name_json += "}";

		std::string message_block = message.empty() ? "" : render(alert_info_html,alert_info_html_len,{{"MESSAGE",message}});

		std::string cart_section;
		if(session->cart.empty()) {
			cart_section = render(alert_info_html,alert_info_html_len,{{"MESSAGE","El carrito esta vacio."}});
		} else {
			std::string rows;
			double total = 0;
			for(const auto& item : session->cart) {
				double subtotal = item.unit_price * item.quantity;
				total += subtotal;
				rows += render(cart_row_html,cart_row_html_len,{
					{"NAME",item.name},
					{"PRICE",format_currency(item.unit_price)},
					{"QTY",std::to_string(item.quantity)},
					{"SUBTOTAL",format_currency(subtotal)},
					{"PRODUCT_ID",item.product_id},
				});
			}

			cart_section = render(cart_table_html,cart_table_html_len,{
				{"CART_ROWS",rows},
				{"TOTAL",format_currency(total)},
			});
		}

		return wrap_html("Carrito",render(cart_html,cart_html_len,{
			{"MESSAGE_BLOCK",message_block},
			{"PRODUCT_CARDS",product_cards},
			{"STOCKS_JSON",stock_json},
			{"PRICES_JSON",price_json},
			{"NAMES_JSON",name_json},
			{"CURRENCY_SYMBOL",AppConfig::get_instance().currency_symbol()},
			{"CART_SECTION",cart_section},
		}));
	}

	std::string cart_confirm_page(Session* session,const std::string& message) {
		auto render_message = [&](const std::string& alert_html,const std::string& title) {
			return wrap_html(title,render(cart_confirm_message_html,cart_confirm_message_html_len,{
				{"ALERT_BLOCK",alert_html},
			}));
		};

		if(!message.empty() && message.find("ORD") != std::string::npos) {
			return render_message(render(alert_success_html,alert_success_html_len,{{"MESSAGE",message}}),"Orden Confirmada");
		}
		if(!message.empty()) {
			return render_message(render(alert_danger_html,alert_danger_html_len,{{"MESSAGE",message}}),"Orden Rechazada");
		}

		auto& cart = session->cart;
		if(cart.empty()) {
			return render_message(render(alert_info_html,alert_info_html_len,{{"MESSAGE","El carrito esta vacio."}}),"Confirmar Orden");
		}

		std::string rows;
		double total = 0;
		for(const auto& item : cart) {
			double subtotal = item.unit_price * item.quantity;
			total += subtotal;
			rows += render(order_row_html,order_row_html_len,{
				{"NAME",item.name},
				{"PRICE",format_currency(item.unit_price)},
				{"QTY",std::to_string(item.quantity)},
				{"SUBTOTAL",format_currency(subtotal)},
			});
		}

		return wrap_html("Confirmar Orden",render(cart_confirm_html,cart_confirm_html_len,{
			{"ORDER_ROWS",rows},
			{"TOTAL",format_currency(total)},
		}));
	}

	std::string inventory_view_page(Session* session) {
		auto products = (session->role == "vendor")
			? InventoryService::get_instance().get_products_with_isla_stock(session->isla_id)
			: InventoryService::get_instance().get_all_products();

		std::string manage_link = session->role == "admin" ? load(manage_inventory_link_html,manage_inventory_link_html_len) : "";

		std::string rows;
		for(const auto& p : products) {
			rows += render(inventory_view_row_html,inventory_view_row_html_len,{
				{"ID",p.id},
				{"NAME",p.name},
				{"PRICE",format_currency(p.sale_price)},
				{"STOCK",std::to_string(p.quantity)},
			});
		}

		return wrap_html("Inventario",render(inventory_view_html,inventory_view_html_len,{
			{"MANAGE_LINK",manage_link},
			{"PRODUCT_ROWS",rows},
		}));
	}

	std::string inventory_manage_page(Session* session,const std::string& message) {
		auto products = InventoryService::get_instance().get_all_products();

		std::string message_block = message.empty() ? "" : render(alert_success_html,alert_success_html_len,{{"MESSAGE",message}});

		std::string rows;
		for(const auto& p : products) {
			std::string image_cell = !p.imagen.empty()
				? render(product_thumb_html,product_thumb_html_len,{{"SRC",p.imagen},{"ALT",p.name}})
				: load(product_thumb_placeholder_html,product_thumb_placeholder_html_len);

			rows += render(inventory_manage_row_html,inventory_manage_row_html_len,{
				{"ID",p.id},
				{"NAME",p.name},
				{"PRICE",format_price(p.sale_price)},
				{"STOCK",std::to_string(p.quantity)},
				{"IMAGE_CELL",image_cell},
			});
		}

		return wrap_html("Gestionar Inventario",render(inventory_manage_html,inventory_manage_html_len,{
			{"MESSAGE_BLOCK",message_block},
			{"PRODUCT_ROWS",rows},
		}));
	}

	std::string sales_report_page(Session* session, const SalesReport& report, const std::string& vendor, const std::string& product_id) {
		auto all_vendors = SalesService::get_instance().get_all_vendors_with_sales();
		auto all_products = SalesService::get_instance().get_all_products_with_sales();

		std::string vendor_filter;
		if(!all_vendors.empty()) {
			std::string links;
			for(const auto& v : all_vendors) {
				links += render(vendor_filter_link_html,vendor_filter_link_html_len,{
					{"VENDOR",v},
					{"PRODUCT",product_id},
					{"CLASS",vendor == v ? "btn-primary" : "btn-danger"},
				});
			}
			vendor_filter = render(vendor_filter_html,vendor_filter_html_len,{
				{"ALL_CLASS",vendor.empty() ? "btn-primary" : "btn-danger"},
				{"PRODUCT",product_id},
				{"VENDOR_LINKS",links},
			});
		}

		std::string product_filter;
		std::string product_name;
		if(!all_products.empty()) {
			std::string links;
			for(const auto& p : all_products) {
				if(p.id == product_id) product_name = p.name;
				links += render(product_filter_link_html,product_filter_link_html_len,{
					{"VENDOR",vendor},
					{"PRODUCT_ID",p.id},
					{"PRODUCT_NAME",p.name},
					{"CLASS",product_id == p.id ? "btn-primary" : "btn-danger"},
				});
			}
			product_filter = render(product_filter_html,product_filter_html_len,{
				{"ALL_CLASS",product_id.empty() ? "btn-primary" : "btn-danger"},
				{"VENDOR",vendor},
				{"PRODUCT_LINKS",links},
			});
		}

		std::string vendor_suffix = vendor.empty() ? "" : " - " + vendor;
		std::string product_suffix = product_name.empty() ? "" : " - " + product_name;

		std::string report_body;
		if(report.orders.empty()) {
			report_body = render(alert_info_html,alert_info_html_len,{{"MESSAGE","No hay ventas registradas."}});
		} else {
			std::string rows;
			for(const auto& order : report.orders) {
				std::string method;
				switch(order.payment_method) {
					case 'E': method = "Efectivo"; break;
					case 'T': method = "Tarjeta"; break;
					case 'C': method = "Cortesia"; break;
					default: method = "Otro"; break;
				}

				rows += render(sales_order_row_html,sales_order_row_html_len,{
					{"SALE_ID",order.sale_id},
					{"SALE_TIME",order.sale_time},
					{"METHOD",method},
					{"TOTAL",format_currency(order.total_sale)},
					{"VENDOR",order.vendor},
				});
			}

			auto get_count = [&](char key) -> int {
				auto it = report.transactions.find(key);
				return it != report.transactions.end() ? it->second.transaction_count : 0;
			};
			auto get_revenue = [&](char key) -> double {
				auto it = report.transactions.find(key);
				return it != report.transactions.end() ? it->second.revenue : 0.0;
			};

			report_body = render(sales_report_body_html,sales_report_body_html_len,{
				{"ORDER_ROWS",rows},
				{"TOTAL_TRANSACTIONS",std::to_string(report.total_transactions)},
				{"COUNT_E",std::to_string(get_count('E'))},
				{"COUNT_T",std::to_string(get_count('T'))},
				{"COUNT_C",std::to_string(get_count('C'))},
				{"REVENUE_TOTAL",format_currency(report.total_revenue)},
				{"REVENUE_E",format_currency(get_revenue('E'))},
				{"REVENUE_T",format_currency(get_revenue('T'))},
				{"REVENUE_C",format_currency(get_revenue('C'))},
			});
		}

		return wrap_html("Reporte de Ventas",render(sales_report_html,sales_report_html_len,{
			{"VENDOR_SUFFIX",vendor_suffix},
			{"PRODUCT_SUFFIX",product_suffix},
			{"VENDOR_FILTER",vendor_filter},
			{"PRODUCT_FILTER",product_filter},
			{"REPORT_BODY",report_body},
		}));
	}

	std::string vendor_manage_page(Session* session,const std::string& message) {
		auto vendors = VendorService::get_instance().get_all_vendors();
		auto isles = IsleService::get_instance().get_all_isles();

		std::string message_block = message.empty() ? "" : render(alert_success_html,alert_success_html_len,{{"MESSAGE",message}});

		std::string rows;
		for(const auto& v : vendors) {
			std::string estado = v.activo
				? load(vendor_active_html,vendor_active_html_len)
				: load(vendor_inactive_html,vendor_inactive_html_len);

			rows += render(vendor_row_html,vendor_row_html_len,{
				{"ID",v.id},
				{"NOMBRE",v.nombre},
				{"ESTADO",estado},
				{"TOGGLE_LABEL",v.activo ? "Desactivar" : "Activar"},
				{"ISLA_OPTIONS",build_isla_options(isles,v.isla_id,true,false)},
			});
		}

		return wrap_html("Gestionar Vendedores",render(vendor_manage_html,vendor_manage_html_len,{
			{"MESSAGE_BLOCK",message_block},
			{"VENDOR_ROWS",rows},
		}));
	}

	std::string isle_manage_page(Session* session,const std::string& message) {
		auto isles = IsleService::get_instance().get_all_isles();

		std::string message_block = message.empty() ? "" : render(alert_success_html,alert_success_html_len,{{"MESSAGE",message}});

		std::string rows;
		for(const auto& isla : isles) {
			rows += render(isle_row_html,isle_row_html_len,{
				{"ID",isla.id},
				{"NOMBRE",isla.nombre},
			});
		}

		return wrap_html("Gestionar Islas",render(isle_manage_html,isle_manage_html_len,{
			{"MESSAGE_BLOCK",message_block},
			{"ISLE_ROWS",rows},
		}));
	}

	std::string transfer_page(Session* session,const std::string& message) {
		auto products = InventoryService::get_instance().get_all_products();
		auto isles = IsleService::get_instance().get_all_isles();

		std::string message_block = message.empty() ? "" : render(alert_success_html,alert_success_html_len,{{"MESSAGE",message}});

		std::string product_cards;
		std::string names_json = "{";
		bool first_name = true;
		for(const auto& p : products) {
			std::string image_cell = !p.imagen.empty()
				? render(product_image_html,product_image_html_len,{{"SRC",p.imagen},{"ALT",p.name}})
				: load(product_image_placeholder_html,product_image_placeholder_html_len);

			product_cards += render(transfer_product_card_html,transfer_product_card_html_len,{
				{"ID",p.id},
				{"IMAGE",image_cell},
				{"NAME",p.name},
			});

			if(!first_name) names_json += ",";
			names_json += "\"" + p.id + "\":\"" + p.name + "\"";
			first_name = false;
		}
		names_json += "}";

		std::string origin_stocks_json = "{\"" + TransferService::ALMACEN_ID + "\":{";
		{
			bool first = true;
			for(const auto& p : products) {
				if(!first) origin_stocks_json += ",";
				origin_stocks_json += "\"" + p.id + "\":" + std::to_string(p.quantity);
				first = false;
			}
		}
		origin_stocks_json += "}";

		for(const auto& isla : isles) {
			auto isle_products = InventoryService::get_instance().get_products_with_isla_stock(isla.id);
			origin_stocks_json += ",\"" + isla.id + "\":{";
			bool first = true;
			for(const auto& p : isle_products) {
				if(!first) origin_stocks_json += ",";
				origin_stocks_json += "\"" + p.id + "\":" + std::to_string(p.quantity);
				first = false;
			}
			origin_stocks_json += "}";
		}
		origin_stocks_json += "}";

		std::string origin_options = build_isla_options(isles,"",false,true);
		std::string destination_options = build_isla_options(isles,"",false,true);

		std::map<std::string,std::string> location_names;
		location_names[TransferService::ALMACEN_ID] = "Almacen Central";
		for(const auto& isla : isles) {
			location_names[isla.id] = isla.nombre;
		}

		auto history = TransferService::get_instance().get_transfer_history();
		std::string rows;
		for(const auto& t : history) {
			auto origin_it = location_names.find(t.origin_id);
			auto destination_it = location_names.find(t.destination_id);

			rows += render(transfer_row_html,transfer_row_html_len,{
				{"PRODUCT_NAME",t.product_name},
				{"ORIGIN_NAME",origin_it != location_names.end() ? origin_it->second : t.origin_id},
				{"DESTINATION_NAME",destination_it != location_names.end() ? destination_it->second : t.destination_id},
				{"QUANTITY",std::to_string(t.quantity)},
				{"USERNAME",t.username},
				{"FECHA",t.fecha},
			});
		}

		return wrap_html("Transferencias",render(transfer_html,transfer_html_len,{
			{"MESSAGE_BLOCK",message_block},
			{"PRODUCT_CARDS",product_cards},
			{"ORIGIN_STOCKS_JSON",origin_stocks_json},
			{"NAMES_JSON",names_json},
			{"ORIGIN_OPTIONS",origin_options},
			{"DESTINATION_OPTIONS",destination_options},
			{"TRANSFER_ROWS",rows},
		}));
	}

	std::string monitoring_body(Session* session) {
		auto isles = IsleService::get_instance().get_all_isles();

		std::string cards;
		for(const auto& isla : isles) {
			auto summary = SalesService::get_instance().get_isle_sales_summary(isla.id);
			auto products = InventoryService::get_instance().get_products_with_isla_stock(isla.id);

			auto get_count = [&](char key) -> int {
				auto it = summary.transactions.find(key);
				return it != summary.transactions.end() ? it->second.transaction_count : 0;
			};
			auto get_revenue = [&](char key) -> double {
				auto it = summary.transactions.find(key);
				return it != summary.transactions.end() ? it->second.revenue : 0.0;
			};

			std::string inventory_rows;
			for(const auto& p : products) {
				inventory_rows += render(monitoring_inventory_row_html,monitoring_inventory_row_html_len,{
					{"NAME",p.name},
					{"STOCK",std::to_string(p.quantity)},
					{"STOCK_CLASS",stock_class_for(p.quantity)},
				});
			}

			cards += render(monitoring_isle_card_html,monitoring_isle_card_html_len,{
				{"ISLE_NAME",isla.nombre},
				{"COUNT_E",std::to_string(get_count('E'))},
				{"COUNT_T",std::to_string(get_count('T'))},
				{"COUNT_C",std::to_string(get_count('C'))},
				{"REVENUE_E",format_currency(get_revenue('E'))},
				{"REVENUE_T",format_currency(get_revenue('T'))},
				{"REVENUE_C",format_currency(get_revenue('C'))},
				{"TOTAL_TRANSACTIONS",std::to_string(summary.total_transactions)},
				{"REVENUE_TOTAL",format_currency(summary.total_revenue)},
				{"INVENTORY_ROWS",inventory_rows},
			});
		}

		if(cards.empty()) {
			cards = render(alert_info_html,alert_info_html_len,{{"MESSAGE","No hay islas configuradas."}});
		}

		return cards;
	}

	std::string monitoring_page(Session* session) {
		return wrap_html("Monitoreo",render(monitoring_html,monitoring_html_len,{
			{"MONITORING_BODY",monitoring_body(session)},
		}));
	}
}
