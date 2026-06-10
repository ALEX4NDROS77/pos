#include <algorithm>
#include <services/CartService.h>
#include <services/InventoryService.h>

CartService& CartService::get_instance() {
	static CartService instance;
	return instance;
}

bool CartService::add_to_cart(Session* session,const std::string& product_id, int quantity) {
	if(!session || quantity <= 0) return false;

	auto product = InventoryService::get_instance().get_product_by_id(product_id);
	if(!product) return false;

	for(auto& item : session->cart) {
		if(item.product_id == product_id) {
			item.quantity += quantity;
			return true;
		}
	}

	CartItem item;
	item.product_id = product_id;
	item.name = product->name;
	item.quantity = quantity;
	item.unit_price = product->sale_price;
	session->cart.push_back(item);

	return true;
}

 bool CartService::update_cart_item(Session* session,const std::string& product_id,int quantity) {
	 if(!session) return false;

	 if(quantity <= 0) {
		 return remove_from_cart(session,product_id);
	 }

	 for(auto& item : session->cart) {
		 if(item.product_id == product_id) {
			 item.quantity = quantity;
			 return true;
		 }
	 }

	 return false;
 }

 bool CartService::remove_from_cart(Session* session,const std::string& product_id) {
	 if(!session) return false;

	 auto& cart = session->cart;
	 auto it = std::remove_if(cart.begin(),cart.end(),
			 [&product_id](const CartItem& item) { return item.product_id == product_id; });

	 if(it != cart.end()) {
		 cart.erase(it,cart.end());
		 return true;
	 }

	 return false;
 }

 void CartService::clear_cart(Session* session) {
	 if(session) {
		 session->cart.clear();
	 }
 }

 double CartService::get_cart_total(Session* session) {
	 if(!session) return 0.0;

	 double total = 0.0;
	 for(const auto& item : session->cart) {
		 total += item.unit_price * item.quantity;
	 }
	 return total;
 }

 std::vector<CartItem>& CartService::get_cart_items(Session* session) {
	 static std::vector<CartItem> empty;
	 if(!session) return empty;
	 return session->cart;
 }
