#ifndef SERVICES_CARTSERVICE_H
#define	SERVICES_CARTSERVICE_H	1

#include <vector>
#include <models/Session.h>
#include <models/CartItem.h>

class CartService {
	public:
		static CartService& get_instance();

		bool add_to_cart(Session* session,const std::string& product_id,int quantity);
		bool update_cart_item(Session* session,const std::string& product_id,int quantity);
		bool remove_from_cart(Session* session,const std::string& product_id);
		void clear_cart(Session* session);

		double get_cart_total(Session* session);
		std::vector<CartItem>& get_cart_items(Session* session);

	private:
		CartService() = default;
		CartService(const CartService&) = delete;
		CartService& operator=(const CartService&) = delete;
};

#endif

