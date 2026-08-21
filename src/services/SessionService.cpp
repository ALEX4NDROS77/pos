#include <services/SessionService.h>
#include <utils/Utils.h>

SessionService& SessionService::get_instance() {
	static SessionService instance;
	return instance;
}

std::string SessionService::create_session(const std::string& username,const std::string& role) {
	std::lock_guard<std::mutex> lock(m_mutex);

	std::string session_id = Utils::generate_session_id();
	m_sessions[session_id] = {username,role,{}};

	return session_id;
}

void SessionService::destroy_session(const std::string& session_id) {
	std::lock_guard<std::mutex> lock(m_mutex);
	m_sessions.erase(session_id);
}

Session* SessionService::get_session(const std::string& session_id) {
	std::lock_guard<std::mutex> lock(m_mutex);

	auto it = m_sessions.find(session_id);
	if(it != m_sessions.end()) {
		return &it->second;
	}
	return nullptr;
}

Session* SessionService::get_session_from_request(const httplib::Request& req) {
	std::string session_id = Utils::get_session_id_from_cookie(req);
	if(!session_id.empty()) {
		return get_session(session_id);
	}
	return nullptr;
}

Session* SessionService::require_session(const httplib::Request& req,httplib::Response& res) {
	auto* session = get_session_from_request(req);
	if(!session) {
		res.set_redirect("/");
		return nullptr;
	}
	return session;
}

Session* SessionService::require_role(const httplib::Request& req,httplib::Response& res,const std::string& role) {
	auto* session = get_session_from_request(req);
	if(!session || session->role != role) {
		res.set_redirect("/");
		return nullptr;
	}
	return session;
}

bool SessionService::validate_admin_password(const std::string& password) {
	return password == ADMIN_PASSWORD;
}
