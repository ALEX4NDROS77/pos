#ifndef SERVICES_VENDORSERVICE_H
#define	SERVICES_VENDORSERVICE_H	1

#include <models/Vendor.h>
#include <vector>
#include <optional>

class VendorService {
	public:
		static VendorService& get_instance();

		std::optional<Vendor> authenticate(const std::string& nombre,const std::string& password);
		std::vector<Vendor> get_all_vendors();
		std::optional<Vendor> get_vendor_by_id(const std::string& id);
		bool add_vendor(const std::string& nombre,const std::string& password);
		bool update_vendor(const std::string& id,const std::string& nombre,const std::string& password,bool activo);
		bool delete_vendor(const std::string& id);
		bool set_vendor_active(const std::string& id,bool activo);
		bool set_vendor_isle(const std::string& id,const std::string& isla_id);

	private:
		VendorService() = default;
		VendorService(const VendorService&) = delete;
		VendorService& operator=(const VendorService&) = delete;
};

#endif

