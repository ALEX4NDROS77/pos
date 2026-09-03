#include <services/IsleService.h>
#include <services/DatabaseService.h>
#include <utils/SqliteStatement.h>
#include <utils/Utils.h>
#include <sqlite3.h>

IsleService& IsleService::get_instance() {
	static IsleService instance;
	return instance;
}

std::vector<Isla> IsleService::get_all_isles() {
	std::vector<Isla> isles;
	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	SqliteStatement stmt(db.get_connection(),"SELECT ID_ISLA,NOMBRE FROM ISLAS ORDER BY NOMBRE;");
	if(!stmt.ok()) return isles;

	while(stmt.step() == SQLITE_ROW) {
		Isla isla;
		isla.id = stmt.column_text(0);
		isla.nombre = stmt.column_text(1);
		isles.push_back(isla);
	}

	return isles;
}

std::optional<Isla> IsleService::get_isle_by_id(const std::string& id) {
	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	SqliteStatement stmt(db.get_connection(),"SELECT ID_ISLA,NOMBRE FROM ISLAS WHERE ID_ISLA = ?;");
	if(!stmt.ok()) return std::nullopt;

	stmt.bind(1,id);
	if(stmt.step() != SQLITE_ROW) return std::nullopt;

	Isla isla;
	isla.id = stmt.column_text(0);
	isla.nombre = stmt.column_text(1);
	return isla;
}

bool IsleService::add_isle(const std::string& nombre) {
	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	std::string id = "ISLA-" + Utils::generate_uuid().substr(0,8);

	SqliteStatement stmt(db.get_connection(),"INSERT INTO ISLAS (ID_ISLA,NOMBRE) VALUES (?,?);");
	if(!stmt.ok()) return false;

	stmt.bind(1,id);
	stmt.bind(2,nombre);

	return stmt.exec();
}

std::optional<std::string> IsleService::delete_isle(const std::string& id) {
	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	{
		SqliteStatement stmt(db.get_connection(),"SELECT COUNT(*) FROM VENDEDORES WHERE ISLA_ID = ?;");
		if(!stmt.ok()) return "Error interno al verificar vendedores.";
		stmt.bind(1,id);
		if(stmt.step() == SQLITE_ROW && stmt.column_int(0) > 0) {
			return "No se puede eliminar: hay vendedores asignados a esta isla.";
		}
	}

	{
		SqliteStatement stmt(db.get_connection(),"SELECT COALESCE(SUM(CANTIDAD),0) FROM INVENTARIO_ISLA WHERE ISLA_ID = ?;");
		if(!stmt.ok()) return "Error interno al verificar inventario.";
		stmt.bind(1,id);
		if(stmt.step() == SQLITE_ROW && stmt.column_int(0) > 0) {
			return "No se puede eliminar: la isla tiene inventario. Transfiere el stock primero.";
		}
	}

	{
		SqliteStatement stmt(db.get_connection(),"DELETE FROM INVENTARIO_ISLA WHERE ISLA_ID = ?;");
		if(stmt.ok()) {
			stmt.bind(1,id);
			stmt.exec();
		}
	}

	SqliteStatement stmt(db.get_connection(),"DELETE FROM ISLAS WHERE ID_ISLA = ?;");
	if(!stmt.ok()) return "Error interno al eliminar la isla.";
	stmt.bind(1,id);
	if(!stmt.exec()) return "Error interno al eliminar la isla.";

	return std::nullopt;
}
