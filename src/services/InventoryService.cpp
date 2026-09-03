#include <sqlite3.h>
#include <services/InventoryService.h>
#include <services/DatabaseService.h>
#include <utils/Logger.h>
#include <utils/SqliteStatement.h>
#include <utils/Utils.h>

InventoryService& InventoryService::get_instance() {
	static InventoryService instance;
	return instance;
}

std::vector<Product> InventoryService::get_all_products() {
	std::vector<Product> products;
	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	SqliteStatement stmt(db.get_connection(),"SELECT ID_PRODUCTO, NOMBRE, PRECIO_VENTA, CANTIDAD, IFNULL(IMAGEN,'') FROM PRODUCTOS ORDER BY NOMBRE;");
	if(!stmt.ok()) return products;

	while(stmt.step() == SQLITE_ROW) {
		Product p;
		p.id = stmt.column_text(0);
		p.name = stmt.column_text(1);
		p.sale_price = stmt.column_double(2);
		p.quantity = stmt.column_int(3);
		p.imagen = stmt.column_text(4);
		products.push_back(p);
	}

	return products;
}

std::optional<Product> InventoryService::get_product_by_id(const std::string& id) {
	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	SqliteStatement stmt(db.get_connection(),"SELECT ID_PRODUCTO, NOMBRE, PRECIO_VENTA, CANTIDAD, IFNULL(IMAGEN,'') FROM PRODUCTOS WHERE ID_PRODUCTO = ?;");
	if(!stmt.ok()) return std::nullopt;

	stmt.bind(1,id);

	if(stmt.step() == SQLITE_ROW) {
		Product p;
		p.id = stmt.column_text(0);
		p.name = stmt.column_text(1);
		p.sale_price = stmt.column_double(2);
		p.quantity = stmt.column_int(3);
		p.imagen = stmt.column_text(4);
		return p;
	}

	return std::nullopt;
}

bool InventoryService::add_product(const std::string& name,double price,int quantity,const std::string& imagen) {
	LOG_DEBUG("InventoryService::add_product - name=" + name + ", imagen_size=" + std::to_string(imagen.size()));

	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	std::string id = "PROD-" + Utils::generate_uuid().substr(0,8);

	SqliteStatement stmt(db.get_connection(),"INSERT INTO PRODUCTOS (ID_PRODUCTO, NOMBRE, PRECIO_VENTA, CANTIDAD, IMAGEN) VALUES (?,?,?,?,?);");
	if(!stmt.ok()) {
		LOG_ERROR("InventoryService::add_product - Failed to prepare statement: " + std::string(sqlite3_errmsg(db.get_connection())));
		return false;
	}

	stmt.bind(1,id);
	stmt.bind(2,name);
	stmt.bind(3,price);
	stmt.bind(4,quantity);
	stmt.bind(5,imagen);

	bool success = stmt.exec();
	if(!success) {
		LOG_ERROR("InventoryService::add_product - SQLite error: " + std::string(sqlite3_errmsg(db.get_connection())));
	}
	return success;
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

	SqliteStatement stmt(db.get_connection(),sql);
	if(!stmt.ok()) {
		LOG_ERROR("InventoryService::update_product - Failed to prepare statement: " + std::string(sqlite3_errmsg(db.get_connection())));
		return false;
	}

	stmt.bind(1,name);
	stmt.bind(2,price);
	stmt.bind(3,quantity);
	if(imagen.empty()) {
		stmt.bind(4,id);
	} else {
		stmt.bind(4,imagen);
		stmt.bind(5,id);
	}

	bool success = stmt.exec();
	if(!success) {
		LOG_ERROR("InventoryService::update_product - SQLite error: " + std::string(sqlite3_errmsg(db.get_connection())));
	}
	return success;
}

bool InventoryService::delete_product(const std::string& id) {
	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	SqliteStatement stmt(db.get_connection(),"DELETE FROM PRODUCTOS WHERE ID_PRODUCTO = ?;");
	if(!stmt.ok()) return false;

	stmt.bind(1,id);
	return stmt.exec();
}

bool InventoryService::update_quantity(const std::string& id,int quantity) {
	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	SqliteStatement stmt(db.get_connection(),"UPDATE PRODUCTOS SET CANTIDAD = ? WHERE ID_PRODUCTO = ?;");
	if(!stmt.ok()) return false;

	stmt.bind(1,quantity);
	stmt.bind(2,id);
	return stmt.exec();
}

int InventoryService::get_isla_stock(const std::string& isla_id,const std::string& product_id) {
	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	SqliteStatement stmt(db.get_connection(),"SELECT CANTIDAD FROM INVENTARIO_ISLA WHERE ISLA_ID = ? AND PRODUCTO_ID = ?;");
	if(!stmt.ok()) return 0;

	stmt.bind(1,isla_id);
	stmt.bind(2,product_id);

	if(stmt.step() == SQLITE_ROW) {
		return stmt.column_int(0);
	}
	return 0;
}

std::vector<Product> InventoryService::get_products_with_isla_stock(const std::string& isla_id) {
	std::vector<Product> products;
	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	SqliteStatement stmt(db.get_connection(),
		"SELECT P.ID_PRODUCTO, P.NOMBRE, P.PRECIO_VENTA, IFNULL(I.CANTIDAD,0), IFNULL(P.IMAGEN,'') "
		"FROM PRODUCTOS P LEFT JOIN INVENTARIO_ISLA I ON I.PRODUCTO_ID = P.ID_PRODUCTO AND I.ISLA_ID = ? "
		"ORDER BY P.NOMBRE;");
	if(!stmt.ok()) return products;

	stmt.bind(1,isla_id);

	while(stmt.step() == SQLITE_ROW) {
		Product p;
		p.id = stmt.column_text(0);
		p.name = stmt.column_text(1);
		p.sale_price = stmt.column_double(2);
		p.quantity = stmt.column_int(3);
		p.imagen = stmt.column_text(4);
		products.push_back(p);
	}

	return products;
}
