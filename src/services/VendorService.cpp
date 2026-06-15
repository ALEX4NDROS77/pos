#include <services/VendorService.h>
#include <services/DatabaseService.h>
#include <utils/Utils.h>
#include <sqlite3.h>

VendorService& VendorService::get_instance() {
	static VendorService instance;
	return instance;
}

std::optional<Vendor> VendorService::authenticate(const std::string& nombre,const std::string& password) {
	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	const char* sql = "SELECT ID_VENDEDOR,NOMBRE,PASSWORD,ACTIVO FROM VENDEDORES WHERE NOMBRE = ? AND PASSWORD = ? AND ACTIVO = 1;";
	sqlite3_stmt* stmt;

	if(sqlite3_prepare_v2(db.get_connection(),sql,-1,&stmt,nullptr) != SQLITE_OK) {
		return std::nullopt;
	}

	sqlite3_bind_text(stmt,1,nombre.c_str(),-1,SQLITE_STATIC);
	sqlite3_bind_text(stmt,2,password.c_str(),-1,SQLITE_STATIC);

	if(sqlite3_step(stmt) != SQLITE_ROW) {
		sqlite3_finalize(stmt);
		return std::nullopt;
	}

	Vendor v;
	v.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt,0));
	v.nombre = reinterpret_cast<const char*>(sqlite3_column_text(stmt,1));
	v.password = reinterpret_cast<const char*>(sqlite3_column_text(stmt,2));
	v.activo = sqlite3_column_int(stmt,3) == 1;
	sqlite3_finalize(stmt);
	return v;
}

std::vector<Vendor> VendorService::get_all_vendors() {
	std::vector<Vendor> vendors;
	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	const char* sql = "SELECT ID_VENDEDOR,NOMBRE,PASSWORD,ACTIVO FROM VENDEDORES ORDER BY NOMBRE;";
	sqlite3_stmt* stmt;

	if(sqlite3_prepare_v2(db.get_connection(),sql,-1,&stmt,nullptr) != SQLITE_OK) {
		return vendors;
	}

	while(sqlite3_step(stmt) == SQLITE_ROW) {
		Vendor v;
		v.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt,0));
		v.nombre = reinterpret_cast<const char*>(sqlite3_column_text(stmt,1));
		v.password = reinterpret_cast<const char*>(sqlite3_column_text(stmt,2));
		v.activo = sqlite3_column_int(stmt,3) == 1;
		vendors.push_back(v);
	}
	sqlite3_finalize(stmt);

	return vendors;
}

std::optional<Vendor> VendorService::get_vendor_by_id(const std::string& id) {
	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	const char* sql = "SELECT ID_VENDEDOR,NOMBRE,PASSWORD,ACTIVO FROM VENDEDORES WHERE ID_VENDEDOR = ?;";
	sqlite3_stmt* stmt;

	if(sqlite3_prepare_v2(db.get_connection(),sql,-1,&stmt,nullptr) != SQLITE_OK) {
		return std::nullopt;
	}

	sqlite3_bind_text(stmt,1,id.c_str(),-1,SQLITE_STATIC);

	if(sqlite3_step(stmt) != SQLITE_ROW) {
		sqlite3_finalize(stmt);
		return std::nullopt;
	}

	Vendor v;
	v.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt,0));
	v.nombre = reinterpret_cast<const char*>(sqlite3_column_text(stmt,1));
	v.password = reinterpret_cast<const char*>(sqlite3_column_text(stmt,2));
	v.activo = sqlite3_column_int(stmt,3) == 1;
	sqlite3_finalize(stmt);

	return v;
}

bool VendorService::add_vendor(const std::string& nombre,const std::string& password) {
	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	std::string id = "VEND-" + Utils::generate_uuid().substr(0,8);

	const char* sql = "INSERT INTO VENDEDORES (ID_VENDEDOR,NOMBRE,PASSWORD,ACTIVO) VALUES (?,?,?,1);";
	sqlite3_stmt* stmt;

	if(sqlite3_prepare_v2(db.get_connection(),sql,-1,&stmt,nullptr) != SQLITE_OK) {
		return false;
	}

	sqlite3_bind_text(stmt,1,id.c_str(),-1,SQLITE_STATIC);
	sqlite3_bind_text(stmt,2,nombre.c_str(),-1,SQLITE_STATIC);
	sqlite3_bind_text(stmt,3,password.c_str(),-1,SQLITE_STATIC);

	bool success = sqlite3_step(stmt) == SQLITE_DONE;
	sqlite3_finalize(stmt);
	return success;
}

bool VendorService::update_vendor(const std::string& id,const std::string& nombre,const std::string& password,bool activo) {
	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	const char* sql = "UPDATE VENDEDORES SET NOMBRE = ?, PASSWORD = ?, ACTIVO = ? WHERE ID_VENDEDOR = ?;";
	sqlite3_stmt* stmt;

	if(sqlite3_prepare_v2(db.get_connection(),sql,-1,&stmt,nullptr) != SQLITE_OK) {
		return false;
	}

	sqlite3_bind_text(stmt,1,nombre.c_str(),-1,SQLITE_STATIC);
	sqlite3_bind_text(stmt,2,password.c_str(),-1,SQLITE_STATIC);
	sqlite3_bind_int(stmt,3,activo ? 1 : 0);
	sqlite3_bind_text(stmt,4,id.c_str(),-1,SQLITE_STATIC);

	bool success = sqlite3_step(stmt) == SQLITE_DONE;
	sqlite3_finalize(stmt);

	return success;
}

bool VendorService::delete_vendor(const std::string& id) {
	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	const char* sql = "DELETE FROM VENDEDORES WHERE ID_VENDEDOR = ?;";
	sqlite3_stmt* stmt;

	if(sqlite3_prepare_v2(db.get_connection(),sql,-1,&stmt,nullptr) != SQLITE_OK) {
		return false;
	}

	sqlite3_bind_text(stmt,1,id.c_str(),-1,SQLITE_STATIC);
	
	bool success = sqlite3_step(stmt) == SQLITE_DONE;
	sqlite3_finalize(stmt);

	return success;
}

bool VendorService::set_vendor_active(const std::string& id,bool activo) {
	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	const char* sql = "UPDATE VENDEDORES SET ACTIVO = ? WHERE ID_VENDEDOR = ?;";
	sqlite3_stmt* stmt;

	if(sqlite3_prepare_v2(db.get_connection(),sql,-1,&stmt,nullptr) != SQLITE_OK) {
		return false;
	}

	sqlite3_bind_int(stmt,1,activo ? 1 : 0);
	sqlite3_bind_text(stmt,2,id.c_str(),-1,SQLITE_STATIC);

	bool success = sqlite3_step(stmt) == SQLITE_DONE;
	sqlite3_finalize(stmt);

	return success;
}

