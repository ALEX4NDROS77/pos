#ifndef	SERVICES_INVENTORYSERVICE_H
#define	SERVICES_INVENTORYSERVICE_H	1

#include <vector>
#include <optional>
#include <models/Product.h>

class InventoryService {
	public:
		static InventoryService& get_instance();

		std::vector<Product> get_all_products();
		std::optional<Product> get_product_by_id(const std::string& id);
		bool add_product(const std::string& name,double price,int quantity,const std::string& image);
		bool update_product(const std::string& id,const std::string& name, double price,int quantity,const std::string& imagen);
		bool delete_product(const std::string& id);
		bool update_quantity(const std::string& id,int cantidad);

		int get_isla_stock(const std::string& isla_id,const std::string& product_id);
		std::vector<Product> get_products_with_isla_stock(const std::string& isla_id);

	private:
		InventoryService() = default;
		InventoryService(const InventoryService&) = delete;
		InventoryService& operator=(const InventoryService&) = delete;
};

#endif

