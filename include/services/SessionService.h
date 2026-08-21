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

		// Look up the session for req; on failure, redirect res to "/" and return nullptr.
		Session* require_session(const httplib::Request& req,httplib::Response& res);
		// Same as require_session, but also redirects (and returns nullptr) if the session's role doesn't match.
		Session* require_role(const httplib::Request& req,httplib::Response& res,const std::string& role);

		bool validate_admin_password(const std::string& password);

	private:
		SessionService() = default;
		SessionService(const SessionService&) = delete;
		SessionService operator=(const SessionService&) = delete;

		std::map<std::string,Session> m_sessions;
		std::mutex m_mutex;

		const std::string ADMIN_PASSWORD = "admin123";
};

#endif
