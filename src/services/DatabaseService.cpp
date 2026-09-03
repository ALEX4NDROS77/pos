#include <iostream>
#include <services/DatabaseService.h>

DatabaseService& DatabaseService::get_instance() {
	static DatabaseService instance;
	return instance;
}

DatabaseService::~DatabaseService() {
	close();
}

bool DatabaseService::initialize(const std::string& db_path) {
	std::lock_guard<std::mutex> lock(m_mutex);

	if(m_db) {
		return true;
	}

	int rc = sqlite3_open(db_path.c_str(),&m_db);
	if(rc != SQLITE_OK) {
		std::cerr << "Cannot open database: " << sqlite3_errmsg(m_db) << std::endl;
		return false;
	}

	if(!create_tables()) {
		return false;
	}

	migrate_schema();

	if(!insert_sample_data()) {
		return false;
	}

	std::cout << "Database initialized successfully." <<std::endl;
	return true;
};

void DatabaseService::close() {
	if(m_db) {
		sqlite3_close(m_db);
		m_db = nullptr;
	}
}

sqlite3* DatabaseService::get_connection() {
	return m_db;
}

std::mutex& DatabaseService::get_mutex() {
	return m_mutex;
}

bool DatabaseService::create_tables() {
	const char* sql = R"(
		CREATE TABLE IF NOT EXISTS PRODUCTOS (
			ID_PRODUCTO VARCHAR(36) PRIMARY KEY,
			NOMBRE VARCHAR(100) NOT NULL,
			PRECIO_VENTA REAL NOT NULL,
			CANTIDAD INTEGER NOT NULL DEFAULT 0,
			IMAGEN VARCHAR(255) DEFAULT ''
		);

		CREATE TABLE IF NOT EXISTS ORDENES (
			ID_VENTA VARCHAR(36) PRIMARY KEY,
			FECHA_VENTA DATE NOT NULL,
			VENTA_TOTAL REAL NOT NULL,
			METODO_PAGO CHAR(1) NOT NULL,
			VENDEDOR VARCHAR(20) NOT NULL
		);

		CREATE TABLE IF NOT EXISTS VENTAS (
			ID_VENTA VARCHAR(36) PRIMARY KEY,
			ORDEN_ID VARCHAR(36) NOT NULL,
			PRODUCTO_ID VARCHAR(36) NOT NULL,
			CANTIDAD_VENTA INTEGER NOT NULL,
			PRECIO_UNITARIO REAL NOT NULL,
			FOREIGN KEY (ORDEN_ID) REFERENCES ORDENES(ID_VENTA),
			FOREIGN KEY(PRODUCTO_ID) REFERENCES PRODUCTOS(ID_PRODUCTO)
		);

		CREATE TABLE IF NOT EXISTS VENDEDORES (
			ID_VENDEDOR VARCHAR(36) PRIMARY KEY,
			NOMBRE VARCHAR(100) NOT NULL,
			PASSWORD VARCHAR(100) NOT NULL,
			ACTIVO INTEGER NOT NULL DEFAULT 1
		);

		CREATE TABLE IF NOT EXISTS TICKETS (
			ID_TICKET VARCHAR(36) PRIMARY KEY,
			ORDEN_ID VARCHAR(36) NOT NULL,
			VENDEDOR_NOMBRE VARCHAR(100) NOT NULL,
			FECHA_CREACION DATETIME NOT NULL,
			ESTADO VARCHAR(20) NOT NULL DEFAULT 'PENDIENTE',
			FOREIGN KEY (ORDEN_ID) REFERENCES ORDENES(ID_VENTA)
		);

		CREATE TABLE IF NOT EXISTS TICKET_ITEMS (
			ID INTEGER PRIMARY KEY  AUTOINCREMENT,
			TICKET_ID VARCHAR(36) NOT NULL,
			PRODUCTO_ID VARCHAR(36) NOT NULL,
			NOMBRE_PRODUCTO VARCHAR(100) NOT NULL,
			CANTIDAD INTEGER NOT NULL,
			FOREIGN KEY (TICKET_ID) REFERENCES TICKETS(ID_TICKET)
		);

		CREATE TABLE IF NOT EXISTS ISLAS (
			ID_ISLA VARCHAR(36) PRIMARY KEY,
			NOMBRE VARCHAR(100) NOT NULL
		);

		CREATE TABLE IF NOT EXISTS INVENTARIO_ISLA (
			ISLA_ID VARCHAR(36) NOT NULL,
			PRODUCTO_ID VARCHAR(36) NOT NULL,
			CANTIDAD INTEGER NOT NULL DEFAULT 0,
			PRIMARY KEY (ISLA_ID, PRODUCTO_ID),
			FOREIGN KEY (ISLA_ID) REFERENCES ISLAS(ID_ISLA),
			FOREIGN KEY (PRODUCTO_ID) REFERENCES PRODUCTOS(ID_PRODUCTO)
		);

		CREATE TABLE IF NOT EXISTS TRANSFERENCIAS (
			ID_TRANSFERENCIA VARCHAR(36) PRIMARY KEY,
			PRODUCTO_ID VARCHAR(36) NOT NULL,
			ORIGEN VARCHAR(36) NOT NULL,
			DESTINO VARCHAR(36) NOT NULL,
			CANTIDAD INTEGER NOT NULL,
			USUARIO VARCHAR(100) NOT NULL,
			FECHA DATETIME NOT NULL,
			FOREIGN KEY (PRODUCTO_ID) REFERENCES PRODUCTOS(ID_PRODUCTO)
		);
	)";

	char* err_msg = nullptr;
	int rc = sqlite3_exec(m_db,sql,nullptr,nullptr,&err_msg);

	if(rc != SQLITE_OK) {
		std::cerr << "SQL error: " <<err_msg << std::endl;
		sqlite3_free(err_msg);
		return false;
	}

	return true;
}

void DatabaseService::add_column_if_missing(const std::string& table,const std::string& column,const std::string& decl) {
	std::string sql = "ALTER TABLE " + table + " ADD COLUMN " + column + " " + decl + ";";
	char* err_msg = nullptr;
	int rc = sqlite3_exec(m_db,sql.c_str(),nullptr,nullptr,&err_msg);

	if(rc != SQLITE_OK) {
		std::string err = err_msg ? err_msg : "";
		if(err.find("duplicate column name") == std::string::npos) {
			std::cerr << "SQL error adding column " << table << "." << column << ": " << err << std::endl;
		}
		sqlite3_free(err_msg);
	}
}

void DatabaseService::migrate_schema() {
	add_column_if_missing("VENDEDORES","ISLA_ID","VARCHAR(36) NOT NULL DEFAULT ''");
	add_column_if_missing("ORDENES","ISLA_ID","VARCHAR(36) NOT NULL DEFAULT ''");
}

bool DatabaseService::insert_sample_data() {
	const char* check_sql = "SELECT COUNT(*) FROM PRODUCTOS;";
	sqlite3_stmt* stmt;
	int count = 0;

	if(sqlite3_prepare_v2(m_db,check_sql,-1,&stmt,nullptr) == SQLITE_OK) {
		if(sqlite3_step(stmt) == SQLITE_ROW) {
			count = sqlite3_column_int(stmt,0);
		}
		sqlite3_finalize(stmt);
	}

	if(count > 0) {
		return true;
	}

	const char* sql = R"(
		INSERT INTO PRODUCTOS (ID_PRODUCTO,NOMBRE,PRECIO_VENTA,IMAGEN) VALUES
		('PROD-001','Manzana',5,''),
		('PROD-002','Banana',3,''),
		('PROD-003','Naranja',6,''),
		('PROD-004','Leche 1L',2,''),
		('PROD-005','Pan',5,''),
		('PROD-006','Huevos (12)',5,''),
		('PROD-007','Queso',3,''),
		('PROD-008','Mantequilla',2,'');

		INSERT INTO ISLAS (ID_ISLA,NOMBRE) VALUES
		('ISLA-001','Isla 1'),
		('ISLA-002','Isla 2');

		INSERT INTO VENDEDORES (ID_VENDEDOR,NOMBRE,PASSWORD,ACTIVO,ISLA_ID) VALUES
		('VEND-001','vendedor1','1234',1,'ISLA-001'),
		('VEND-002','vendedor2','5678',1,'ISLA-002');
	)";

	char* err_msg = nullptr;
	int rc = sqlite3_exec(m_db,sql,nullptr,nullptr,&err_msg);

	if(rc != SQLITE_OK) {
		std::cerr << "Error inserting sample data: " << err_msg << std::endl;
		sqlite3_free(err_msg);
		return false;
	}

	std::cout <<"Sample products inserted." << std::endl;
	return true;
}
