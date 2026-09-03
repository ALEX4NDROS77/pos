#ifndef SERVICES_ISLESERVICE_H
#define	SERVICES_ISLESERVICE_H	1

#include <models/Isla.h>
#include <optional>
#include <string>
#include <vector>

class IsleService {
	public:
		static IsleService& get_instance();

		std::vector<Isla> get_all_isles();
		std::optional<Isla> get_isle_by_id(const std::string& id);
		bool add_isle(const std::string& nombre);

		// nullopt on success; otherwise a user-facing reason the delete was blocked/failed.
		std::optional<std::string> delete_isle(const std::string& id);

	private:
		IsleService() = default;
		IsleService(const IsleService&) = delete;
		IsleService& operator=(const IsleService&) = delete;
};

#endif

