#ifndef	SERVICES_DATABASESERVICE_H
#define	SERVICES_DATABASESERVICE_H	1

#include <string>
#include <sqlite3.h>
#include <mutex>

class DatabaseService {
	public:
		static DatabaseService& get_instance();

		bool initialize(const std::string& db_path = "pos.db");
		void close();

		sqlite3* get_connection();
		std::mutex& get_mutex();

	private:
		DatabaseService() = default;
		~DatabaseService();
		DatabaseService(const DatabaseService&) = delete;
		DatabaseService& operator=(const DatabaseService&) = delete;

		bool create_tables();
		bool insert_sample_data();

		sqlite3* m_db = nullptr;
		std::mutex m_mutex;
};

#endif

