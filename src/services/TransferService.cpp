#include <mutex>
#include <sqlite3.h>
#include <services/DatabaseService.h>
#include <services/TransferService.h>
#include <utils/Logger.h>
#include <utils/SqliteStatement.h>
#include <utils/Utils.h>

const std::string TransferService::ALMACEN_ID = "ALMACEN";

TransferService& TransferService::get_instance() {
	static TransferService instance;
	return instance;
}

std::optional<std::string> TransferService::transfer(const std::string& product_id,const std::string& origin_id,const std::string& destination_id,int quantity,const std::string& username) {
	if(quantity <= 0 || origin_id.empty() || destination_id.empty() || origin_id == destination_id) {
		LOG_WARNING("TransferService::transfer - invalid request: origin='" + origin_id + "', destination='" + destination_id + "', quantity=" + std::to_string(quantity));
		return "Error: cantidad invalida u origen/destino iguales.";
	}

	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	sqlite3_exec(db.get_connection(),"BEGIN IMMEDIATE;",nullptr,nullptr,nullptr);

	int available = 0;
	{
		std::string sql = (origin_id == ALMACEN_ID)
			? "SELECT CANTIDAD FROM PRODUCTOS WHERE ID_PRODUCTO = ?;"
			: "SELECT CANTIDAD FROM INVENTARIO_ISLA WHERE ISLA_ID = ? AND PRODUCTO_ID = ?;";
		SqliteStatement stmt(db.get_connection(),sql);
		if(!stmt.ok()) {
			sqlite3_exec(db.get_connection(),"ROLLBACK;",nullptr,nullptr,nullptr);
			return std::nullopt;
		}
		if(origin_id == ALMACEN_ID) {
			stmt.bind(1,product_id);
		} else {
			stmt.bind(1,origin_id);
			stmt.bind(2,product_id);
		}
		if(stmt.step() == SQLITE_ROW) {
			available = stmt.column_int(0);
		}
	}

	if(available < quantity) {
		sqlite3_exec(db.get_connection(),"ROLLBACK;",nullptr,nullptr,nullptr);
		LOG_WARNING("TransferService::transfer - insufficient stock at origin='" + origin_id + "' for product='" + product_id + "'");
		return "Error: stock insuficiente en el origen.";
	}

	{
		std::string sql = (origin_id == ALMACEN_ID)
			? "UPDATE PRODUCTOS SET CANTIDAD = CANTIDAD - ? WHERE ID_PRODUCTO = ?;"
			: "UPDATE INVENTARIO_ISLA SET CANTIDAD = CANTIDAD - ? WHERE ISLA_ID = ? AND PRODUCTO_ID = ?;";
		SqliteStatement stmt(db.get_connection(),sql);
		if(!stmt.ok()) {
			sqlite3_exec(db.get_connection(),"ROLLBACK;",nullptr,nullptr,nullptr);
			return std::nullopt;
		}
		if(origin_id == ALMACEN_ID) {
			stmt.bind(1,quantity);
			stmt.bind(2,product_id);
		} else {
			stmt.bind(1,quantity);
			stmt.bind(2,origin_id);
			stmt.bind(3,product_id);
		}
		if(!stmt.exec()) {
			sqlite3_exec(db.get_connection(),"ROLLBACK;",nullptr,nullptr,nullptr);
			return std::nullopt;
		}
	}

	{
		std::string sql = (destination_id == ALMACEN_ID)
			? "UPDATE PRODUCTOS SET CANTIDAD = CANTIDAD + ? WHERE ID_PRODUCTO = ?;"
			: "INSERT INTO INVENTARIO_ISLA (ISLA_ID,PRODUCTO_ID,CANTIDAD) VALUES (?,?,?) ON CONFLICT(ISLA_ID,PRODUCTO_ID) DO UPDATE SET CANTIDAD = CANTIDAD + excluded.CANTIDAD;";
		SqliteStatement stmt(db.get_connection(),sql);
		if(!stmt.ok()) {
			sqlite3_exec(db.get_connection(),"ROLLBACK;",nullptr,nullptr,nullptr);
			return std::nullopt;
		}
		if(destination_id == ALMACEN_ID) {
			stmt.bind(1,quantity);
			stmt.bind(2,product_id);
		} else {
			stmt.bind(1,destination_id);
			stmt.bind(2,product_id);
			stmt.bind(3,quantity);
		}
		if(!stmt.exec()) {
			sqlite3_exec(db.get_connection(),"ROLLBACK;",nullptr,nullptr,nullptr);
			return std::nullopt;
		}
	}

	std::string transfer_id = "TRF-" + Utils::generate_uuid().substr(0,8);
	{
		SqliteStatement stmt(db.get_connection(),"INSERT INTO TRANSFERENCIAS (ID_TRANSFERENCIA,PRODUCTO_ID,ORIGEN,DESTINO,CANTIDAD,USUARIO,FECHA) VALUES (?,?,?,?,?,?,?);");
		if(!stmt.ok()) {
			sqlite3_exec(db.get_connection(),"ROLLBACK;",nullptr,nullptr,nullptr);
			return std::nullopt;
		}
		stmt.bind(1,transfer_id);
		stmt.bind(2,product_id);
		stmt.bind(3,origin_id);
		stmt.bind(4,destination_id);
		stmt.bind(5,quantity);
		stmt.bind(6,username);
		stmt.bind(7,Utils::get_timestamp());
		if(!stmt.exec()) {
			sqlite3_exec(db.get_connection(),"ROLLBACK;",nullptr,nullptr,nullptr);
			return std::nullopt;
		}
	}

	sqlite3_exec(db.get_connection(),"COMMIT;",nullptr,nullptr,nullptr);
	LOG_INFO("TransferService::transfer - completed transfer=" + transfer_id + ", product=" + product_id + ", " + origin_id + " -> " + destination_id + ", qty=" + std::to_string(quantity));
	return transfer_id;
}

std::vector<Transfer> TransferService::get_transfer_history() {
	std::vector<Transfer> transfers;
	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	SqliteStatement stmt(db.get_connection(),
		"SELECT T.ID_TRANSFERENCIA, T.PRODUCTO_ID, P.NOMBRE, T.ORIGEN, T.DESTINO, T.CANTIDAD, T.USUARIO, T.FECHA "
		"FROM TRANSFERENCIAS T JOIN PRODUCTOS P ON P.ID_PRODUCTO = T.PRODUCTO_ID "
		"ORDER BY T.FECHA DESC;");
	if(!stmt.ok()) return transfers;

	while(stmt.step() == SQLITE_ROW) {
		Transfer t;
		t.id = stmt.column_text(0);
		t.product_id = stmt.column_text(1);
		t.product_name = stmt.column_text(2);
		t.origin_id = stmt.column_text(3);
		t.destination_id = stmt.column_text(4);
		t.quantity = stmt.column_int(5);
		t.username = stmt.column_text(6);
		t.fecha = stmt.column_text(7);
		transfers.push_back(t);
	}

	return transfers;
}
