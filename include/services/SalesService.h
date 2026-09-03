#ifndef SERVICES_SALESSERVICE_H
#define	SERVICES_SALESSERVICE_H	1

#include <optional>
#include <unordered_map>
#include <vector>
#include <models/Order.h>
#include <models/Product.h>
#include <models/Sale.h>
#include <models/Session.h>

struct Transaction {
	double revenue;
	int transaction_count;
};

struct SalesReport {
	std::vector<Order> orders;
	std::unordered_map<char,Transaction> transactions;
	double total_revenue;
	int total_transactions;
};

class SalesService {
	public:
		static SalesService& get_instance();

		std::optional<std::string> checkout(Session* session,char payment_method);
		SalesReport get_sales_report(const std::string& vendor = "",const std::string& product_id = "");
		SalesReport get_isle_sales_summary(const std::string& isla_id);
		std::vector<Sale> get_sales_by_order_id(const std::string& order_id);
		std::vector<std::string> get_all_vendors_with_sales();
		std::vector<Product> get_all_products_with_sales();

	private:
		SalesService() = default;
		SalesService(const SalesService&) = delete;
		SalesService& operator=(const SalesService&) = delete;
};

#endif

