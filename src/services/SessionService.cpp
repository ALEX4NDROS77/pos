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

bool SessionService::validate_admin_password(const std::string& password) {
	return password == ADMIN_PASSWORD;
}

