#include <sqlite3.h>
#include <services/InventoryService.h>
#include <services/DatabaseService.h>
#include <utils/Logger.h>
#include <utils/Utils.h>

InventoryService& InventoryService::get_instance() {
	static InventoryService instance;
	return instance;
}

std::vector<Product> InventoryService::get_all_products() {
	std::vector<Product> products;
	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	const char* sql = "SELECT ID_PRODUCTO, NOMBRE, PRECIO_VENTA, CANTIDAD, IFNULL(IMAGEN,'') FROM PRODUCTOS ORDER BY NOMBRE;";
	sqlite3_stmt* stmt;

	if(sqlite3_prepare_v2(db.get_connection(),sql,-1,&stmt,nullptr) == SQLITE_OK) {
		while (sqlite3_step(stmt) == SQLITE_ROW) {
			Product p;
			p.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt,0));
			p.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt,1));
			p.sale_price = sqlite3_column_double(stmt,2);
			p.quantity = sqlite3_column_int(stmt,3);
			const char* img = reinterpret_cast<const char*>(sqlite3_column_text(stmt,4));
			p.imagen = img ? img : "";
			products.push_back(p);
		}
		sqlite3_finalize(stmt);
	}

	return products;
}

std::optional<Product> InventoryService::get_product_by_id(const std::string& id) {
	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	const char* sql = "SELECT ID_PRODUCTO, NOMBRE, PRECIO_VENTA, CANTIDAD, IFNULL(IMAGEN,'') FROM PRODUCTOS WHERE ID_PRODUCTO = ?;";
	sqlite3_stmt* stmt;

	if(sqlite3_prepare_v2(db.get_connection(),sql,-1,&stmt,nullptr) == SQLITE_OK) {
		sqlite3_bind_text(stmt,1,id.c_str(), -1, SQLITE_STATIC);

		if(sqlite3_step(stmt) == SQLITE_ROW) {
			Product p;
			p.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt,0));
			p.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt,1));
			p.sale_price = sqlite3_column_double(stmt,2);
			p.quantity = sqlite3_column_int(stmt,3);
			const char* img = reinterpret_cast<const char*>(sqlite3_column_text(stmt,4));
			p.imagen = img ? img : "";
			sqlite3_finalize(stmt);
			return p;
		}
		sqlite3_finalize(stmt);
	}

	return std::nullopt;
}

bool InventoryService::add_product(const std::string& name,double price,int quantity,const std::string& imagen) {
	LOG_DEBUG("InventoryService::add_product - name=" + name + ", imagen_size=" + std::to_string(imagen.size()));

	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	std::string id = "PROD-" + Utils::generate_uuid().substr(0,8);

	const char* sql = "INSERT INTO PRODUCTOS (ID_PRODUCTO, NOMBRE, PRECIO_VENTA, CANTIDAD, IMAGEN) VALUES (?,?,?,?,?);";
	sqlite3_stmt* stmt;

	if(sqlite3_prepare_v2(db.get_connection(),sql,-1,&stmt,nullptr) == SQLITE_OK) {
		sqlite3_bind_text(stmt,1,id.c_str(),-1,SQLITE_STATIC);
		sqlite3_bind_text(stmt,2,name.c_str(),-1,SQLITE_STATIC);
		sqlite3_bind_double(stmt,3,price);
		sqlite3_bind_int(stmt,4,quantity);
		sqlite3_bind_text(stmt,5,imagen.c_str(),-1,SQLITE_STATIC);

		int result = sqlite3_step(stmt);
		bool success = result == SQLITE_DONE;
		if(!success) {
			LOG_ERROR("InventoryService::add_product - SQLite error: " + std::string(sqlite3_errmsg(db.get_connection())));
		}
		sqlite3_finalize(stmt);
		return success;
	}

	LOG_ERROR("InventoryService::add_product - Failed to prepare statement: " + std::string(sqlite3_errmsg(db.get_connection())));
	return false;
}

bool InventoryService::update_product(const std::string& id,const std::string& name, double price,int quantity,const std::string& imagen) {
	LOG_DEBUG("InventoryService::update_product - id=" + id + ", name=" + name + ", imagen_size=" + std::to_string(imagen.size()));

	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	std::string sql;
	if(imagen.empty()) {
		sql = "UPDATE PRODUCTOS SET NOMBRE = ?, PRECIO_VENTA = ?, CANTIDAD = ? WHERE ID_PRODUCTO = ?;";
		LOG_DEBUG("InventoryService::update_product - Updating without image");
	} else {
		sql = "UPDATE PRODUCTOS SET NOMBRE = ?, PRECIO_VENTA = ?, CANTIDAD = ?, IMAGEN = ? WHERE ID_PRODUCTO = ?;";
		LOG_DEBUG("InventoryService::update_product - Updating with new image");
	}
	sqlite3_stmt* stmt;

	if(sqlite3_prepare_v2(db.get_connection(),sql.c_str(),-1,&stmt,nullptr) == SQLITE_OK) {
		sqlite3_bind_text(stmt,1,name.c_str(),-1,SQLITE_STATIC);
		sqlite3_bind_double(stmt,2,price);
		sqlite3_bind_int(stmt,3,quantity);
		if(imagen.empty()) {
			sqlite3_bind_text(stmt,4,id.c_str(),-1,SQLITE_STATIC);
		} else {
			sqlite3_bind_text(stmt,4,imagen.c_str(),-1,SQLITE_STATIC);
			sqlite3_bind_text(stmt,5,id.c_str(),-1,SQLITE_STATIC);
		}

		int result = sqlite3_step(stmt);
		bool success = result == SQLITE_DONE;
		if(!success) {
			LOG_ERROR("InventoryService::update_product - SQLite error: " + std::string(sqlite3_errmsg(db.get_connection())));
		}
		sqlite3_finalize(stmt);
		return success;
	}

	LOG_ERROR("InventoryService::update_product - Failed to prepare statement: " + std::string(sqlite3_errmsg(db.get_connection())));
	return false;
}

bool InventoryService::delete_product(const std::string& id) {
	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	const char* sql = "DELETE FROM PRODUCTOS WHERE ID_PRODUCTO = ?;";
	sqlite3_stmt* stmt;

	if(sqlite3_prepare_v2(db.get_connection(),sql,-1,&stmt,nullptr) == SQLITE_OK) {
		sqlite3_bind_text(stmt,1,id.c_str(),-1,SQLITE_STATIC);

		bool success = sqlite3_step(stmt) == SQLITE_DONE;
		sqlite3_finalize(stmt);
		return success;
	}

	return false;
}

bool InventoryService::update_quantity(const std::string& id,int quantity) {
	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	const char* sql = "UPDATE PRODUCTOS SET CANTIDAD = ? WHERE ID_PRODUCTO = ?;";
	sqlite3_stmt* stmt;

	if(sqlite3_prepare_v2(db.get_connection(),sql,-1,&stmt,nullptr) == SQLITE_OK) {
		sqlite3_bind_int(stmt,1,quantity);
		sqlite3_bind_text(stmt,2,id.c_str(),-1,SQLITE_STATIC);

		bool success = sqlite3_step(stmt) == SQLITE_DONE;
		sqlite3_finalize(stmt);
		return success;
	}

	return false;
}

