#ifndef VIEWS_HTMLTEMPLATES_H
#define	VIEWS_HTMLTEMPLATES_H	1

#include <string>
#include <vector>
#include <models/Session.h>
#include <models/Ticket.h>
#include <services/SalesService.h>

namespace HtmlTemplates {
	std::string wrap_html(const std::string& title,const std::string& content);

	std::string login_page(const std::string& error = "");
	std::string vendor_login_page(const std::string& error = "");
	std::string admin_login_page(const std::string& error = "");
	std::string homepage(Session* session,const std::vector<Ticket>& tickets);

	std::string cart_page(Session* session,const std::string& message = "");
	std::string cart_confirm_page(Session* session,const std::string& message = "");

	std::string inventory_view_page(Session* session);
	std::string inventory_manage_page(Session* session,const std::string& message = "");

	std::string sales_report_page(Session* session,const SalesReport& report,const std::string& vendor = "",const std::string& product_id = "");

	std::string vendor_manage_page(Session* session,const std::string& message = "");

	std::string isle_manage_page(Session* session,const std::string& message = "");

	std::string transfer_page(Session* session,const std::string& message = "");

	std::string monitoring_page(Session* session);
	std::string monitoring_body(Session* session);
}

#endif

