#include <services/VendorService.h>
#include <services/DatabaseService.h>
#include <utils/SqliteStatement.h>
#include <utils/Utils.h>
#include <sqlite3.h>

VendorService& VendorService::get_instance() {
	static VendorService instance;
	return instance;
}

std::optional<Vendor> VendorService::authenticate(const std::string& nombre,const std::string& password) {
	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	SqliteStatement stmt(db.get_connection(),"SELECT ID_VENDEDOR,NOMBRE,PASSWORD,ACTIVO,ISLA_ID FROM VENDEDORES WHERE NOMBRE = ? AND PASSWORD = ? AND ACTIVO = 1;");
	if(!stmt.ok()) return std::nullopt;

	stmt.bind(1,nombre);
	stmt.bind(2,password);

	if(stmt.step() != SQLITE_ROW) return std::nullopt;

	Vendor v;
	v.id = stmt.column_text(0);
	v.nombre = stmt.column_text(1);
	v.password = stmt.column_text(2);
	v.activo = stmt.column_int(3) == 1;
	v.isla_id = stmt.column_text(4);
	return v;
}

std::vector<Vendor> VendorService::get_all_vendors() {
	std::vector<Vendor> vendors;
	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	SqliteStatement stmt(db.get_connection(),"SELECT ID_VENDEDOR,NOMBRE,PASSWORD,ACTIVO,ISLA_ID FROM VENDEDORES ORDER BY NOMBRE;");
	if(!stmt.ok()) return vendors;

	while(stmt.step() == SQLITE_ROW) {
		Vendor v;
		v.id = stmt.column_text(0);
		v.nombre = stmt.column_text(1);
		v.password = stmt.column_text(2);
		v.activo = stmt.column_int(3) == 1;
		v.isla_id = stmt.column_text(4);
		vendors.push_back(v);
	}

	return vendors;
}

std::optional<Vendor> VendorService::get_vendor_by_id(const std::string& id) {
	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	SqliteStatement stmt(db.get_connection(),"SELECT ID_VENDEDOR,NOMBRE,PASSWORD,ACTIVO,ISLA_ID FROM VENDEDORES WHERE ID_VENDEDOR = ?;");
	if(!stmt.ok()) return std::nullopt;

	stmt.bind(1,id);

	if(stmt.step() != SQLITE_ROW) return std::nullopt;

	Vendor v;
	v.id = stmt.column_text(0);
	v.nombre = stmt.column_text(1);
	v.password = stmt.column_text(2);
	v.activo = stmt.column_int(3) == 1;
	v.isla_id = stmt.column_text(4);
	return v;
}

bool VendorService::add_vendor(const std::string& nombre,const std::string& password) {
	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	std::string id = "VEND-" + Utils::generate_uuid().substr(0,8);

	SqliteStatement stmt(db.get_connection(),"INSERT INTO VENDEDORES (ID_VENDEDOR,NOMBRE,PASSWORD,ACTIVO) VALUES (?,?,?,1);");
	if(!stmt.ok()) return false;

	stmt.bind(1,id);
	stmt.bind(2,nombre);
	stmt.bind(3,password);

	return stmt.exec();
}

bool VendorService::update_vendor(const std::string& id,const std::string& nombre,const std::string& password,bool activo) {
	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	SqliteStatement stmt(db.get_connection(),"UPDATE VENDEDORES SET NOMBRE = ?, PASSWORD = ?, ACTIVO = ? WHERE ID_VENDEDOR = ?;");
	if(!stmt.ok()) return false;

	stmt.bind(1,nombre);
	stmt.bind(2,password);
	stmt.bind(3,activo ? 1 : 0);
	stmt.bind(4,id);

	return stmt.exec();
}

bool VendorService::delete_vendor(const std::string& id) {
	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	SqliteStatement stmt(db.get_connection(),"DELETE FROM VENDEDORES WHERE ID_VENDEDOR = ?;");
	if(!stmt.ok()) return false;

	stmt.bind(1,id);
	return stmt.exec();
}

bool VendorService::set_vendor_active(const std::string& id,bool activo) {
	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	SqliteStatement stmt(db.get_connection(),"UPDATE VENDEDORES SET ACTIVO = ? WHERE ID_VENDEDOR = ?;");
	if(!stmt.ok()) return false;

	stmt.bind(1,activo ? 1 : 0);
	stmt.bind(2,id);

	return stmt.exec();
}

bool VendorService::set_vendor_isle(const std::string& id,const std::string& isla_id) {
	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	SqliteStatement stmt(db.get_connection(),"UPDATE VENDEDORES SET ISLA_ID = ? WHERE ID_VENDEDOR = ?;");
	if(!stmt.ok()) return false;

	stmt.bind(1,isla_id);
	stmt.bind(2,id);

	return stmt.exec();
}
