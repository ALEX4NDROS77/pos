#ifndef SERVICES_TRANSFERSERVICE_H
#define	SERVICES_TRANSFERSERVICE_H	1

#include <models/Transfer.h>
#include <optional>
#include <string>
#include <vector>

class TransferService {
	public:
		static TransferService& get_instance();

		// nullopt = hard error; a "TRF-..." string = success id; any other string = user-facing rejection reason.
		std::optional<std::string> transfer(const std::string& product_id,const std::string& origin_id,const std::string& destination_id,int quantity,const std::string& username);
		std::vector<Transfer> get_transfer_history();

		static const std::string ALMACEN_ID;

	private:
		TransferService() = default;
		TransferService(const TransferService&) = delete;
		TransferService& operator=(const TransferService&) = delete;
};

#endif

