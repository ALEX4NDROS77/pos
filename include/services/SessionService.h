#ifndef SERVICES_SESSIONSERVICE_H
#define	SERVICES_SESSIONSERVICE_H	1

#include <map>
#include <mutex>
#include <httplib.h>
#include <models/Session.h>

class SessionService {
	public:
		static SessionService& get_instance();

		std::string create_session(const std::string& username,const std::string& role);
		void destroy_session(const std::string& session_id);
		Session* get_session(const std::string& session_id);
		Session* get_session_from_request(const httplib::Request& req);

		bool validate_admin_password(const std::string& password);
		bool validate_bar_password(const std::string& password);

	private:
		SessionService() = default;
		SessionService(const SessionService&) = delete;
		SessionService operator=(const SessionService&) = delete;

		std::map<std::string,Session> m_sessions;
		std::mutex m_mutex;

		const std::string ADMIN_PASSWORD = "admin123";
		const std::string BAR_PASSWORD = "bar123";
};

#endif

