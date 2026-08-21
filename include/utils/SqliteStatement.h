#ifndef	UTILS_SQLITESTATEMENT_H
#define	UTILS_SQLITESTATEMENT_H	1

#include <sqlite3.h>
#include <string>

// RAII wrapper around sqlite3_stmt*: prepares in the constructor, finalizes in the
// destructor so early returns can't leak a statement. Text binds copy the value
// (SQLITE_TRANSIENT) since callers often bind temporaries.
class SqliteStatement {
	public:
		SqliteStatement(sqlite3* db,const std::string& sql) {
			m_ok = sqlite3_prepare_v2(db,sql.c_str(),-1,&m_stmt,nullptr) == SQLITE_OK;
		}

		~SqliteStatement() {
			if(m_stmt) sqlite3_finalize(m_stmt);
		}

		SqliteStatement(const SqliteStatement&) = delete;
		SqliteStatement& operator=(const SqliteStatement&) = delete;

		bool ok() const { return m_ok; }

		void bind(int index,const std::string& value) { sqlite3_bind_text(m_stmt,index,value.c_str(),-1,SQLITE_TRANSIENT); }
		void bind(int index,const char* value) { sqlite3_bind_text(m_stmt,index,value,-1,SQLITE_TRANSIENT); }
		void bind(int index,int value) { sqlite3_bind_int(m_stmt,index,value); }
		void bind(int index,double value) { sqlite3_bind_double(m_stmt,index,value); }
		void bind(int index,char value) { char buf[2] = {value,'\0'}; sqlite3_bind_text(m_stmt,index,buf,-1,SQLITE_TRANSIENT); }

		int step() { return sqlite3_step(m_stmt); }
		bool exec() { return step() == SQLITE_DONE; }

		void reset() {
			sqlite3_reset(m_stmt);
			sqlite3_clear_bindings(m_stmt);
		}

		std::string column_text(int index) const {
			const unsigned char* text = sqlite3_column_text(m_stmt,index);
			return text ? reinterpret_cast<const char*>(text) : "";
		}

		int column_int(int index) const { return sqlite3_column_int(m_stmt,index); }
		double column_double(int index) const { return sqlite3_column_double(m_stmt,index); }

	private:
		sqlite3_stmt* m_stmt = nullptr;
		bool m_ok = false;
};

#endif
