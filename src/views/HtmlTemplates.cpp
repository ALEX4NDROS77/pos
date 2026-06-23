#include <sstream>
#include <iomanip>
#include <views/HtmlTemplates.h>
#include <services/VendorService.h>
#include <services/InventoryService.h>
#include <services/CartService.h>

namespace HtmlTemplates {
	std::string get_css_style() {
		return R"(
<style>
/* Base Reset & Variables */
* { box-sizing: border-box; margin: 0; padding: 0; }

:root {
	--primary-red: #d41414;
	--accent-orange: #ff4500;
	--bg-dark: #000000;
	--text-main: #e0e0e0;
	/* Larger Base Fonts */
	--font-size-base: 18px;
	--font-size-h1: 2.2rem;
	--font-size-h2: 1.8rem;
	--font-size-btn: 1.1rem;
}

    body {
	    font-family: 'Inter', 'Segoe UI', sans-serif;
	    background: radial-gradient(circle at center, #1a1a1a 0%, var(--bg-dark) 100%);
	    color: var(--text-main);
	    font-size: var(--font-size-base);
	    line-height: 1.6;
	    min-height: 100vh;
	    padding: 10px; /* Reduced for mobile */
    }

    .container {
	    max-width: 900px;
	    margin: 0 auto;
	    background: rgba(255, 255, 255, 0.05);
	    backdrop-filter: blur(10px);
	    padding: 20px;
	    border-radius: 15px;
	    border: 1px solid rgba(212, 20, 20, 0.3);
    }

    /* Vertical/Mobile First Header */
    .header {
	    display: flex;
	    flex-direction: column;
	    align-items: center;
	    text-align: center;
	    gap: 15px;
	    margin-bottom: 25px;
	    padding-bottom: 15px;
	    border-bottom: 2px solid #333;
    }

    h1 { font-size: var(--font-size-h1); color: #ff3c3c; text-transform: uppercase; text-shadow: 0 0 10px rgba(212,20,20,0.3); }
    h2 { font-size: var(--font-size-h2); color: #ff3c3c; margin: 20px 0; }

    /* Touch-Friendly Buttons */
    .btn {
	    display: block; /* Stack buttons on mobile */
	    width: 100%;
	    padding: 18px;
	    margin: 10px 0;
	    border-radius: 12px;
	    font-size: var(--font-size-btn);
	    font-weight: 700;
	    text-align: center;
	    text-decoration: none;
	    text-transform: uppercase;
	    transition: 0.3s;
    }

    .btn-primary { background: var(--primary-red); color: white; }
    .btn-success { background: var(--accent-orange); color: white; }
    .btn-danger { background: #222; color: #ff3c3c; border: 2px solid #ff3c3c; }

    /* Inputs optimized for thumbs */
    input, select {
	    width: 100% !important;
	    padding: 16px !important;
	    font-size: 1.1rem !important;
	    background: rgba(0,0,0,0.5);
	    border: 2px solid #444;
	    border-radius: 10px;
	    color: white;
	    margin-bottom: 15px;
    }

    /* Responsive Tables (Scrollable on small screens) */
    .table-container {
	    width: 100%;
	    overflow-x: auto;
    }
    table { width: 100%; border-collapse: collapse; font-size: 1rem; }
    th, td { padding: 12px 8px; border-bottom: 1px solid #333; }
    th { background: var(--primary-red); }

    /* Menu Grid for Mobile */
    .menu-grid {
	    display: grid;
	    grid-template-columns: 1fr; /* Single column for phone */
	    gap: 15px;
    }

    .menu-item {
	    padding: 30px 15px;
	    background: linear-gradient(135deg, #d41414 0%, #660000 100%);
	    border-radius: 15px;
	    font-size: 1.4rem;
	    text-align: center;
	    text-decoration: none;
	    color: white;
	    font-weight: bold;
	    border: 1px solid #ff3c3c;
    }

    /* Desktop Adjustments */
    @media (min-width: 768px) {
	    body { padding: 40px; }
	    .header { flex-direction: row; justify-content: space-between; text-align: left; }
	    .btn { display: inline-block; width: auto; margin: 5px; }
	    .menu-grid { grid-template-columns: repeat(auto-fit, minmax(250px, 1fr)); }
	    .inline-form { display: flex; gap: 10px; }
	    .inline-form input { width: auto !important; }
    }
</style>
	   )";
	}

	std::string wrap_html(const std::string& title, const std::string& content) {
		return "<!DOCTYPE html><html><head>"
			"<meta charset='UTF-8'>"
			"<meta name='viewport' content='width=device-width, initial-scale=1.0'>" // CRITICAL FOR PHONE
			"<title>" + title + " - POS System</title>" + get_css_style() +
			"</head><body>" + content + "</body></html>";
	}

	std::string format_price(double price) {
		std::ostringstream oss;
		oss << price;
		return oss.str();
	}

	std::string login_page(const std::string& error) {
		std::string html = R"(
			<div class="container login-box" style="text-align: center;">
				<img src="/assets/diablo_luna.png" style="width: 179px; margin-bottom: 20px;" alt="Mute Moon">
				<img src="/assets/mvte.png" style="width: 119px; display: block; margin: 0 auto 10px;" alt="MUTE">
				<p style="color: #887; margin-bottom: 30px; text-transform: uppercase; letter-spacing: 2px;">Punto de Venta</p>
		)";

		if(!error.empty()) {
			html += "<div class='alert alert-danger'>" + error + "</div>";
		}

		html += R"(
			<h3>Seleccione su rol:</h3>
			<div class="role-select">
				<a href="/login/vendor" class="btn btn-success role-btn">Vendedor</a>
				<a href="/login/admin" class="btn btn-primary role-btn">Admin</a>
				<a href="/login/bar" class="btn btn-success role-btn">Barra</a>
			</div>
		</div>
		)";

		return wrap_html("Login",html);
	}

	std::string vendor_login_page(const std::string& error) {
		std::string html = R"(
			<style>
			.pin-display {
				background: rgba(0,0,0,0.8);
				border: 2px solid var(--primary-red);
				border-radius: 10px;
				padding: 15px;
				font-size: 2.5rem;
				letter-spacing: 15px;
				text-align: center;
				color: var(--primary-red);
				margin-bottom: 20px;
				min-height: 60px;
			}
			.pin-keypad {
				display: grid;
				grid-template-columns: repeat(3, 1fr);
				gap: 10px;
				max-width: 280px;
				margin: 0 auto 20px;
			}
			.pin-btn {
				padding: 20px;
				font-size: 1.6rem;
				font-weight: bold;
				background: linear-gradient(135deg, #333, #111 100%);
				border: 2px solid #444;
				border-radius: 12px;
				color: white;
				cursor: pointer;
				transition: all 0.2s;
			}
			.pin-btn:hover, .pin-btn:active {
				background: linear-gradient(135deg, var(--primary-red) 0%, #660000 100%);
				border-color: var(--primary-red);
				transform: scale(1.05);
			}
			.pin-btn.clear { background: #660000; }
			.vendor-select { margin-bottom: 20px; }
			</style>
			<div class="container login-box" style="text-align: center;">
				<h1>Acceso Vendedor</h1>
				<form method="POST" action="/login/vendor" id="vendor_form">
					<div class="form-group vendor-select">
						<label>Seleccione Vendedor:</label>
						<select name="username" id="vendor_select" required>
							<option value="">-- Seleccionar --</option>
		)";

		auto vendors = VendorService::get_instance().get_all_vendors();
		for(const auto& v : vendors) {
			if(v.activo) {
				html += "<option value='" + v.nombre + "'>" + v.nombre + "</option>";
			}
		}

		html += R"(
				</select>
			</div>
			<input type="hidden" name="password" id="pin_input" value="">
			<div class="pin-display" id="pin_display">____</div>
		)";

		if(!error.empty()) {
			html += "<div class='alert alert-danger'>" + error + "</div>";
		}

		html += R"(
				<div class="pin-keypad">
					<button type="button" class="pin-btn" onclick='add_digit(&#39;1&#39;)'>1</button>
					<button type="button" class="pin-btn" onclick='add_digit(&#39;2&#39;)'>2</button>
					<button type="button" class="pin-btn" onclick='add_digit(&#39;3&#39;)'>3</button>
					<button type="button" class="pin-btn" onclick='add_digit(&#39;4&#39;)'>4</button>
					<button type="button" class="pin-btn" onclick='add_digit(&#39;5&#39;)'>5</button>
					<button type="button" class="pin-btn" onclick='add_digit(&#39;6&#39;)'>6</button>
					<button type="button" class="pin-btn" onclick='add_digit(&#39;7&#39;)'>7</button>
					<button type="button" class="pin-btn" onclick='add_digit(&#39;8&#39;)'>8</button>
					<button type="button" class="pin-btn" onclick='add_digit(&#39;9&#39;)'>9</button>
					<button type="button" class="pin-btn clear" onclick='clear_pin()'>C</button>
					<button type="button" class="pin-btn" onclick='add_digit(&#39;0&#39;)'>0</button>
					<button type="button" class="pin-btn" onclick='delete_digit()'>&larr;</button>
				</div>
				<button type="submit" class="btn btn-success" style="width: 100%;">Entrar</button>
			</form>
			<br>
			<a href="/" class="btn btn-danger">Volver</a>
		</div>
		<script>
			let pin = '';
			const pin_input = document.getElementById('pin_input');
			const pin_display = document.getElementById('pin_display');

			function update_display() {
				let display = '';
				for(let i = 0; i < 4; i++) {
					display += (i < pin.length) ? '*' : '_';
				}
				pin_display.textContent = display;
				pin_input.value = pin;
			}

			function add_digit(d) {
				if(pin.length < 4) {
					pin += d;
					update_display();
				}
			}

			function delete_digit() {
				pin = pin.slice(0, -1);
				update_display();
			}

			function clear_pin() {
				pin = '';
				update_display();
			}
		</script>
		)";
		return wrap_html("Vendedor",html);
	}

	std::string admin_login_page(const std::string& error) {
		std::string html = R"(
			<div class="container login-box">
				<h0>Acceso Administrador</h1>
				<form method="POST" action="/login/admin">
					<div class="form-group">
						<label>Usuario:</label>
						<input type="text" name="username" placeholder="Ingrese usuario" required>
					</div>
					<div class="form-group">
						<label>Contraseña:</label>
						<input type="password" name="password" placeholder="Ingrese contraseña" required>
					</div>
		)";

		if(!error.empty()) {
			html += "<div class='alert alert-danger'>" + error + "</div>";
		}

		html += R"(
				<button type="submit" class="btn btn-primary" style="width: 100%;">Ingresar</button>
			</form>
			<br>
			<a href="/" class="btn btn-danger">Volver</a>
		</div>
		)";
		return wrap_html("Admin Login",html);
	}

	std::string homepage(Session* session) {
	    std::string html = "<div class='container'>";
	    html += "<div class='header'>"; // CSS handles column on phone, row on desktop

	    // Character and Title Block
	    html += "<div class='brand-block' style='display: flex; align-items: center; gap: 15px;'>";
	    html += "<img src='/assets/diablito_sentado.png' style='height: 100px; border-radius: 12px;'>";
	    html += "<div><img src='/assets/mvte.png' style='height: 45px;'><h1>POS System</h1></div>";
	    html += "</div>";

	    // User Info Block
	    html += "<div class='user-info' style='font-size: 1.1rem;'>";
	    html += "Operador: <strong>" + session->username + "</strong><br>";
	    html += "<a href='/logout' class='btn btn-danger' style='margin-top:10px;'>Salir</a>";
	    html += "</div>";

	    html += "</div>"; // End Header

	    html += "<h2>Menu</h2>";
	    html += "<div class='menu-grid'>";
	    html += "<a href='/cart' class='menu-item'>Carrito</a>";
	    html += "<a href='/cart/confirm' class='menu-item'>Confirmar Venta</a>";
	    html += "<a href='/inventory/view' class='menu-item'>Ver Inventario</a>";

	    if(session->role == "admin") {
		    html += "<a href='/inventory/manage' class='menu-item'>Gestionar Inventario</a>";
		    html += "<a href='/vendors/manage' class='menu-item'>Gestionar Vendedores</a>";
		    html += "<a href='/reports/sales' class='menu-item'>Reporte de Ventas</a>";
	    }

	    html += "</div></div>";
	    return wrap_html("Inicio", html);
	}

	std::string cart_page(Session* session, const std::string& message) {
		auto products = InventoryService::get_instance().get_all_products();

		std::string html = R"(
			<style>
				.product-grid {
					display: grid;
					grid-template-columns: repeat(3, 1fr);
					gap: 15px;
					margin-bottom: 30px;
				}
				.product-card {
					background: linear-gradient(135deg, #222 0%, #111 100%);
					border: 2px solid #333;
					border-radius: 8px;
					padding: 8px;
					text-align: center;
					cursor: pointer;
					transition: all 0.2s;
				}
				.product-card:hover, .product-card.selected {
					border-color: var(--primary-red);
					box-shadow: 0 0 10px rgba(212,20,20,0.4);
					transform: scale(1.02);
				}
				.product-card.out-of-stock {
					opacity: 0.4;
					pointer-events: none;
				}
				.product-img {
					width: 65px;
					height: 65px;
					background: linear-gradient(135deg, #444 0%, #222 100%);
					border-radius: 6px;
					margin: 0 auto 3px;
					display: flex;
					align-items: center;
					justify-content: center;
					font-size: 1.5rem;
					color: #666;
					object-fit: cover;
				}
				.product-name {
					font-weight: bold;
					color: white;
					font-size: 0.7rem;
					margin-bottom: 1px;
					overflow: hidden;
					text-overflow: ellipsis;
					white-space: nowrap;
					line-height: 1.2;
				}
				.product-price {
					color: var(--accent-orange);
					font-weight: bold;
					font-size: 0.75rem;
					line-height: 1.2;
				}
				.product-stock {
					color: #888;
					font-size: 0.6rem;
					line-height: 1.2;
				}
				.qty-controls {
					display: flex;
					align-items: center;
					justify-content: center;
					gap: 8px;
					margin: 8px 0;
				}
				.qty-btn {
					width: 40px;
					height: 40px;
					font-size: 1.3rem;
					background: var(--primary-red);
					border: none;
					border-radius: 6px;
					color: white;
					cursor: pointer;
				}
				.qty-display {
					font-size: 1.5rem;
					font-weight: bold;
					color: white;
					min-width: 40px;
					text-align: center;
				}
				.selected-product {
					background: rgba(212,20,20,0.2);
					border: 2px solid var(--primary-red);
					border-radius: 8px;
					padding: 10px;
					margin-bottom: 10px;
					text-align: center;
				}
				.selected-product h3 {
					font-size: 1rem;
					margin 0 0 5px 0;
				}
				.cart-table-wrapper {
					overflow-x: auto;
					-webkit-overflow-scrolling:touch;
				}
				.cart-table {
					width: 100%;
					font-size: 0.8rem;
					min-width: 400px;
				}
				.cart-table th, .cart-table td {
					padding: 6px 4px;
					white-space: nowrap;
				}
				.cart-table .btn { padding: 4px 8px; font-size: 0.7rem; }
				@media (min-width: 600px) {
					.product-grid { grid-template-columns: repeat(auto-fill, minmax(120px,1fr)); gap: 12px; }
					.product-card { padding: 10px; }
					.product-img { width: 70px; hehight: 70px; }
					.product-name { font-size: 0.85rem; }
				}
			</style>
		)";

		html += "<div class='container'>";
		html += "<div class='header'><h1>Carrito de Compras</h1>";
		html += "<a href='/home' class='btn btn-primary'>Volver al Menu</a></div>";

		if(!message.empty()) {
			html += "<div class='alert alert-info'>" + message + "</div>";
		}

		html += "<h3>Seleccionar Producto</h3>";
		html += "<div class='product-grid'>";

		std::string stock_json = "{";
		std::string price_json = "{";
		std::string name_json = "{";
		bool first = true;

		for(const auto& p : products) {
			std::string out_class = (p.quantity <= 0) ? " out-of-stock" : "";
			html += "<div class='product-card" + out_class + "' data-id='" + p.id + "' onclick=\"select_product('" + p.id + "')\" id='card-" + p.id + "'>";
			if(!p.imagen.empty()) {
				html += "<img src='" + p.imagen + "' class='product-img' alt='" + p.name + "' loading='lazy'>";
			} else {
				html += "<div class='product-img'>\xF0\x9F\x93\xA6</div>";
			}
			html += "<div class='product-name'>" + p.name + "</div>";
			html += "<div class='product-price'>$" + format_price(p.sale_price) + "</div>";
			html += "<div class='product-stock'>Stock: " + std::to_string(p.quantity) + "</div>";
			html += "</div>";

			if(!first) { stock_json += ","; price_json += ","; name_json += ","; }
			stock_json += "\"" + p.id + "\":" + std::to_string(p.quantity);
			price_json += "\"" + p.id + "\":" + format_price(p.sale_price);
			name_json += "\"" + p.id + "\":\"" + p.name + "\"";
			first = false;
		}

		stock_json += "}";
		price_json += "}";
		name_json += "}";

		html += "</div>";

		html += R"(
			<div class="selected-product" id="selected_area" style="display:none;">
				<h3 id="selected_name">-</h3>
				<div class="qty-controls">
					<button type="button" class="qty-btn" onclick='change_qty(-1)'>-</button>
					<span class="qty-display" id="qty_display">1</span>
					<button type="button" class="qty-btn" onclick='change_qty(1)'>+</button>
				</div>
				<form method="POST" action="/cart/add" id="add_form">
					<input type="hidden" name="product_id" id="selected_product_id">
					<input type="hidden" name="quantity" id="selected_qty" value="1">
					<button type="submit" class="btn btn-success" style="padding: 6px 16px; font-size: 0.85rem;">Agregar</button>
				</form>
			</div>
		)";

		html += "<script>";
		html += "const stocks = " + stock_json + ";";
		html += "const prices = " + price_json + ";";
		html += "const names = " + name_json + ";";
		html += R"(
				let selected_id = null;
				let qty = 1;

				function select_product(id) {
					if(stocks[id] <= 0) return;
					document.querySelectorAll('.product-card').forEach(c => c.classList.remove('selected'));
					selected_id = id;
					qty = 1;
					document.getElementById('card-' + id).classList.add('selected');
					document.getElementById('selected_area').style.display = 'block';
					document.getElementById('selected_name').textContent = names[id] + ' - $' + prices[id];
					document.getElementById('selected_product_id').value = id;
					update_qty();
				}

				function change_qty(delta) {
					if(!selected_id) return;
					qty = Math.max(1, Math.min(stocks[selected_id], qty + delta));
					update_qty();
				}

				function update_qty() {
					document.getElementById('qty_display').textContent = qty;
					document.getElementById('selected_qty').value = qty;
				}
			</script>
		)";

		html += "<h3 style='margin-top: 15px;'>Carrito Actual</h3>";

		auto& cart = session->cart;
		if(cart.empty()) {
			html += "<div class='alert alert-info'>El carrito esta vacio.</div>";
		} else {
			html += "<div class='cart-table-wrapper'>";
			html += "<table class='cart-table'><tr><th>Producto</th><th>Precio</th><th>Cant.</th><th>Subtotal</th><th></th></tr>";

			double total = 0;
			for(const auto& item : cart) {
				double subtotal = item.unit_price * item.quantity;
				total += subtotal;

				html += "<tr><td>" + item.name + "</td>";
				html += "<td>$" + format_price(item.unit_price) + "</td>";
				html += "<td>" + std::to_string(item.quantity) + "</td>";
				html += "<td>$" + format_price(subtotal) + "</td>";
				html += "<td><a href='/cart/remove/" + item.product_id + "' class='btn btn-danger'>X</a></td></tr>";
			}

			html += "</table></div>";
			html += "<div class='cart-summary'><span class='total'>Total: $" + format_price(total) + "</span>";
			html += " <a href='/cart/clear' class='btn btn-danger'>Vaciar</a>";
			html += " <a href='/cart/confirm' class='btn btn-success'>Confirmar</a></div>";
		}

		html += "</div>";
		return wrap_html("Carrito",html);
	}

	std::string cart_confirm_page(Session* session,const std::string& message) {
		std::string html = "<div class='container'>";
		html += "<div class='header'><h1>Confirmar Orden</h1>";
		html += "<a href='/home' class='btn btn-primary'>Volver al Menu</a></div>";
		
		if(!message.empty() && message.find("ORD") != std::string::npos) {
			html += "<div class='alert alert-success'>" + message + "</div>";
			html += "<a href='/cart' class='btn btn-primary'>Volver al Carrito</a></div>";
			return wrap_html("Orden Confirmada",html);
		} else if(!message.empty()) {
			html += "<div class='alert alert-danger'>" + message + "</div>";
			html += "<a href='/cart' class='btn btn-primary'>Volver al Carrito</a></div>";
			return wrap_html("Orden Rechazada",html);
		}

		auto& cart = session->cart;
		if(cart.empty()) {
			html += "<div class='alert alert-info'>El carrito esta vacio.</div>";
			html += "<a href='/cart' class='btn btn-primary'>Volver al Carrito</a></div>";
			return wrap_html("Confirmar Orden",html);
		}

		html += "<h3>Detalle de la Orden</h3>";
		html += "<table><tr><th>Producto</th><th>Precio</th><th>Cantidad</th><th>Subtotal</th></tr>";

		double total = 0;
		for(const auto& item : cart) {
			double subtotal = item.unit_price * item.quantity;
			total += subtotal;
			html += "<tr><td>" + item.name + "</td>";
			html += "<td>$" + format_price(item.unit_price) + "</td>";
			html += "<td>" + std::to_string(item.quantity) + "</td>";
			html += "<td>$" + format_price(subtotal) + "</td></tr>";
		}

		html += "</table>";
		html += "<div class='cart-summary'>";
		html += "<span class='total'>Total: $" + format_price(total) + "</span>";
		html += "</div>";

		html += "<h3>Metodo de Pago:</h3>";
		html += "<div class='role-select'>";
		html += "<a href='/cart/checkout/E' class='btn btn-success role-btn'>Efectivo</a>";
		html += "<a href='/cart/checkout/T' class='btn btn-success role-btn'>Tarjeta</a>";
		html += "<a href='/cart/checkout/C' class='btn btn-success role-btn'>Cortesia</a>";
		html += "</div>";
		html += "<a href='/cart' class='btn btn-danger' style='margin-top: 20px;'>Volver al Carrito</a>";

		html += "</div>";
		return wrap_html("Confirmar Orden",html);
	}

	std::string inventory_view_page(Session* session) {
		auto products= InventoryService::get_instance().get_all_products();

		std::string html = "<div class='container'>";
		html += "<div class='header'><h1>Inventario</h1>";
		if(session->role == "admin") {
			html += "<a href='/inventory/manage' class='btn btn-primary'>Gestionar Inventario</a>";
		}
		html += "<a href='/home' class='btn btn-primary'>Volver al Menu</a>";

		html += "</div>";
		
		html += "<table><tr><th>ID</th><th>Producto</th><th>Precio</th><th>Stock</th></tr>";

		for(const auto& p : products) {
			html += "<tr><td>" + p.id + "</td>";
			html += "<td>" + p.name + "</td>";
			html += "<td>$" + format_price(p.sale_price) + "</td>";
			html += "<td>" + std::to_string(p.quantity) + "</td></tr>";
		}

		html += "</table></div>";
		return wrap_html("Inventario",html);
	}

	std::string inventory_manage_page(Session* session,const std::string& message) {
		auto products = InventoryService::get_instance().get_all_products();

		std::string html = R"(
			<style>
				.inv-table { width: 100%; border-collapse: collapse; }
				.inv-table th, .inv-table td {
					padding: 10px;
					text-align: left;
					vertical-align: middle;
					border-bottom: 1px solid #333;
				}
				.inv-table th { background: #222; color: var(--primary-red); }
				.inv-table input[type="text"], .inv-table input[type="number"] {
					width: 100%;
					box-sizing: border-box;
				}
				.inv-table .col-img { width: 60px; }
				.inv-table .col-id { width: 100px; }
				.inv-table .col-name { width: 150px; }
				.inv-table .col-price { width: 80px; }
				.inv-table .col-stock { width: 70px; }
				.inv-table .col-url { width: 180px; }
				.inv-table .col-actions { width: 150px; white-space: nowrap; }
				.product-thumb {
					width: 50px;
					height: 50px;
					object-fit: cover;
					border-radius: 8px;
					background: #333;
				}
				.product-thumb-placeholder {
					width: 50px;
					height: 50px;
					background: linear-gradient(135deg, #333 0%, #222 100%);
					border-radius: 8px;
					display: flex;
					align-items: center;
					justify-content: center;
					font-size: 1.5rem;
				}
				.inv-form { display: contents; }
			</style>
)";

		html += "<div class='container'>";
		html += "<div class='header'><h1>Gestionar Inventario</h1>";
		html += "<a href='/home' class='btn btn-primary'>Volver al Menu</a></div>";

		if(!message.empty()) {
			html += "<div class='alert alert-success'>" + message + "</div>";
		}

		html += R"(
			<h3>Agregar Producto</h3>
			<form method="POST" action="/inventory/add" enctype="multipart/form-data" class="inline-form" style="flex-wrap: wrap;" id="add-form">
				<input type="text" name="name" placeholder="Nombre" required style="flex: 2;">
				<input type="number" name="price" placeholder="Precio" step="1.00" min="0" required style="flex: 1;">
				<input type="number" name="quantity" placeholder="Cant." min="0" required style="flex: 1;">
				<input type="file" name="imagen_original" accept="image/*" style="flex: 2;" onchange='compress_image(this, &#39;add-imagen&#39;)'>
				<input type="hidden" name="imagen" id="add-imagen">
				<button type="submit" class="btn btn-success">Agregar</button>
			</form>
			<script>
			function compress_image(input,target_id) {
				if(!input.files || !input.files[0]) return;
				const file = input.files[0];
				const reader = new FileReader();
				reader.onload = function(e) {
					const img = new Image();
					img.onload = function() {
						const canvas = document.createElement('canvas');
						const MAX_SIZE = 200;
						let w = img.width, h = img.height;
						if(w > h) { if(w > MAX_SIZE) { h *= MAX_SIZE/w; w = MAX_SIZE; } }
						else { if(h > MAX_SIZE) { w *= MAX_SIZE/h; h = MAX_SIZE; } }
						canvas.width = w; canvas.height = h;
						canvas.getContext('2d').drawImage(img,0,0,w,h);
						document.getElementById(target_id).value = canvas.toDataURL('image/jpeg',0.7);
					};
					img.src = e.target.result;
				};
				reader.readAsDataURL(file);
			}
			</script>
		)";

		html += "<h3 style='margin-top: 30px;'>Productos</h3>";
		html += "<table class='inv-table'>";
		html += "<tr><th class='col-img'>Img</th><th class='col-id'>ID</th><th class='col-name'>Producto</th><th class='col-price'>Precio</th><th class='col-stock'>Stock</th><th class='col-img'>Imagen</th><th class='col-actions'>Acciones</th></tr>";

		for(const auto& p : products) {
			html += "<tr>";
			html += "<td class='col-img'>";
			if(!p.imagen.empty()) {
				html += "<img src='" + p.imagen + "' class='product-thumb' alt='" + p.name + "' loading='lazy'>";
			} else {
				html += "<div class='product-thumb-placeholder'>\xF0\x9F\x93\xA6</div>";
			}
			html += "</td>";
			html += "<td class='col-id'>" + p.id + "</td>";
			html += "<form method='POST' action='/inventory/update' enctype='multipart/form-data' class='inv-form'>";
			html += "<input type='hidden' name='id' value='" + p.id + "'>";
			html += "<input type='hidden' name='imagen' id='img-" + p.id + "'>";
			html += "<td class='col-name'><input type='text' name='name' value='" + p.name + "'></td>";
			html += "<td class='col-price'><input type='number' name='price' value='" + format_price(p.sale_price) + "' step='1.00' min='0'></td>";
			html += "<td class='col-stock'><input type='number' name='quantity' value='" + std::to_string(p.quantity) + "' min='0'></td>";
			html += "<td class='col-img'><input type='file' accept='image/*' style='width: 100px;' onchange=\"compress_image(this, 'img-" + p.id + "')\"></td>";
			html += "<td class='col-actions'>";
			html += "<button type='submit' class='btn btn-primary'>Guardar</button>";
			html += "</form>";
			html += "<a href='/inventory/delete/" + p.id + "' class='btn btn-danger' onclick='return confirm(\"Eliminar producto?\")'>Eliminar</a></td></tr>";
		}

		html += "</table></div>";
		return wrap_html("Gestionar Inventario",html);
	}

	std::string sales_report_page(Session* session, const SalesReport& report) {
		std::string html = "<div class='container'>";
		html += "<div class='header'><h1>Reporte de Ventas</h1>";
		html += "<a href='/home' class='btn btn-primary'>Volver al Menu</a></div>";

		if(report.orders.empty()) {
			html += "<div class='alert alert-info'>No hay ventas registradas.</div></div>";
			return wrap_html("Reporte de Ventas",html);
		}

		html += "<table><tr><th>ID Orden</th><th>Hora Venta</th><th>Metodo Pago</th><th>Total</th><th>Vendedor</th></tr>";

		for(const auto& order : report.orders) {
			std::string method;
			switch(order.payment_method) {
				case 'E': method = "Efectivo"; break;
				case 'T': method = "Tarjeta"; break;
				case 'C': method = "Cortesia"; break;
				default: method = "Otro"; break;
			}

			html += "<tr><td>" + order.sale_id + "</td>";
			html += "<td>" + order.sale_time + "</td>";
			html += "<td>" + method + "</td>";
			html += "<td>$" + format_price(order.total_sale) + "</td>";
			html += "<td>" + order.vendor + "</td></tr>";
		}

		html += "</table>";

		auto get_count = [&](char key) -> int {
			auto it = report.transactions.find(key);
			return it != report.transactions.end() ? it->second.transaction_count : 0;
		};
		auto get_revenue = [&](char key) -> double {
			auto it = report.transactions.find(key);
			return it != report.transactions.end() ? it->second.revenue : 0.0;
		};

		html += "<div class='cart-summary'>";
		html += "<h3>Resumen</h3>";
		html += "<p>Total Transacciones: <strong>" + std::to_string(report.total_transactions) + "</strong><br>";
		html += "<span style='color #666666;'>&nbsp;Efectivo: <strong>" + std::to_string(get_count('E')) + "</strong></span><br>";
		html += "<span style='color #666666;'>&nbsp;Tarjeta : <strong>" + std::to_string(get_count('T')) + "</strong></span><br>";
		html += "<span style='color #666666;'>&nbsp;Cortesia: <strong>" + std::to_string(get_count('C')) + "</strong></span><br>";
		html += "</p>";

		html += "<p class='total'>Ingresos Totales: <strong>$" + format_price(report.total_revenue) + "</strong><br>";
		html += "<span style='color #666666;'>&nbsp;Efectivo: <strong>$" + format_price(get_revenue('E')) + "</strong></span><br>";
		html += "<span style='color #666666;'>&nbsp;Tarjeta : <strong>$" + format_price(get_revenue('T')) + "</strong></span><br>";
		html += "<span style='color #666666;'>&nbsp;Cortesia: <strong>$" + format_price(get_revenue('C')) + "</strong></span><br>";
		html += "</p>";
		html += "</div></div>";

		return wrap_html("Reporte de Ventas",html);
	}

	std::string bar_login_page(const std::string& error) {
		std::string html = R"(
			<div class="container login-box">
				<h1>Acceso Bar</h1>
				<form method="POST" action="/login/bar">
					<div class="form-group">
						<label>Contraseña:</label>
						<input type="password" name="password" placeholder="Ingrese contraseña" required>
					</div>
		)";

		if(!error.empty()) {
			html += "<div class='alert alert-danger'>" + error+ "</div>";
		}

		html += R"(
					<button type="submit" class="btn btn-success" style="width: 100%;">Entrar</button>
				</form>
				<br>
				<a href="/" class="btn btn-primary">Volver</a>
			</div>
		)";
		return wrap_html("Bar Login",html);
	}

	std::string vendor_manage_page(Session* session,const std::string& message) {
		auto vendors = VendorService::get_instance().get_all_vendors();

		std::string html = "<div class='container'>";
		html += "<div class='header'><h1>Gestionar Vendedores</h1>";
		html += "<a href='/home' class='btn btn-primary'>Volver al Menu</a></div>";

		if(!message.empty()) {
			html += "<div class='alert alert-success'>" + message + "</div>";
		}

		html += R"(
			<h3>Agregar Vendedor</h3>
			<form method="POST" action="/vendors/add" class="inline-form">
				<input type="text" name="nombre" placeholder="Usuario" required style="flex: 2;">
				<input type="password" name="password" placeholder="Contraseña" required style="flex: 2;">
				<button type="submit" class="btn btn-success">Agregar</button>
			</form>
		)";

		html += "<h3 style='margin-top: 30px;'>Vendedores</h3>";
		html += "<table><tr><th>ID</th><th>Usuario</th><th>Estado</th><th>Acciones</th></tr>";

		for(const auto& v :vendors) {
			html += "<tr><td>" + v.id + "</td>";
			html += "<td>" + v.nombre + "</td>";
			html += "<td>" + std::string(v.activo ? "<span style='color:green'>Activo</span>" : "<span style='color:red'>Inactivo</span>")  + "</td>";
			html += "<td>";
			html += "<a href='/vendors/toggle/" + v.id + "' class='btn btn-success'>" + std::string(v.activo ? "Desactivar" : "Activar") + "</a> ";
			html += "<a href='/vendors/delete/" + v.id + "' class='btn btn-danger' onclick='return confirm(\"¿Eliminar vendedor?\")'>Eliminar</a>";
			html += "</td></tr>";
		}

		html += "</table></div>";
		return wrap_html("Gestionar Vendedores",html);
	}

	std::string bar_screen(Session* session,const std::vector<Ticket>& tickets) {
		std::string html = R"(
			<style>
				.bar-container { max-width: 1200px; margin: 0 auto; }
				.ticket-grid { display: grid; grid-template-columns: repeat(auto-fill, minmax(300px,1fr)); gap: 20px; }
				.ticket-card {
					background: linear-gradient(135deg,#1a1a1a 0% $0a0a0a 100%);
					border: 2px solid #333;
					border-radius: 12px;
					padding: 20px;
					box-shadow: 0 4px 15px rgba(0,0,0,0.3);
				}
				.ticket-card.pendiente { border-left: 5px solid var(--accent-orange); }
				.ticket-card.en_proceso { border-left: 5px solid var(--primary-red); }
				.ticket-header { display: flex; justify-content: space-between; margin-bottom: 15px; align-items: center; }
				.ticket-id { font-weight: bold; font-size: 18px; color: var(--primary-red); }
				.ticket-vendor { color #aaa; }
				.ticket-items { margin: 15px 0; }
				.ticket-item { padding: 10px 0; border-bottom: 1px solid #333; display: flex; justify-content: space-between; color: #e0e0e0; }
				.ticket-item span:last-child {color: var(--accent-orange9; font-size: 1.2rem; }
				.ticket-actions { margin-top: 15px; display: flex; gap: 10px; flex-wrap: wrap; }
				.status-badge { padding: 5px 10px; border-radius: 20px; font-size: 12px; font-weight: bold; }
				.status-pendiente { background: rgba(255,69,0,0.2); color: var(--accent-orange); border: 1px solid var(--accent-orange); }
				.status-en_proceso { background: rgba(212,20,20,0.2); color var(--primary-red); border: 1px solid var(--primary-red); }
				.refresh-btn { position: fixed; bottom: 20px; right: 20px; padding: 15px 25px; font-size: 16px; }
				.bar-header {
					background: linear-gradient(135, #1a1a1a 0%, #0a0a0a 100%);
					border: 2px solid var(--primary-red);
					padding: 20px;
					border-radius: 12px;
					margin-bottom: 20px;
					display: flex;
					justify-content: space-between;
					align-items: center;
					flex-wrap: wrap;
					gap: 15px;
				}
				.bar-header h1 {margin: 0, color: var(--primary-red); }
				.ticket-count { font-size: 1.2rem; color: var(--accent-orange); }
				.empty-state {
					text-align: center;
					padding: 60px 20px;
					background: linear-gradient(135deg, #1a1a1a 0%, #0a0a0a 100%);
					border: 2px solid #333;
					border-radius: 12px;
				}
				.empty-state h2 { color: #28a745; margin-bottom: 10px; }
				.empty-state p { color: #888; }
			</style>
		)";

		html += "<div class='bar-container'>";
		html += "<div class='bar-header'>";
		html += "<h1>Pantalla Barra - Tickets</h1>";
		html += "<div style='display: flex; gap: 15px; align-items: center;'>";
		html += "<span class='ticket-count'>Pendientes: <strong>" + std::to_string(tickets.size()) + "</strong></span>";
		html += "<a href='/logout' class='btn btn-danger'>Salir</a>";
		html += "</div></div>";

		if(tickets.empty()) {
			html += "<div class='empty-state'>";
			html += "<h2>No hay tickets pendientes</h2>";
			html += "<p>Los nuevos pedidos apareceran aqui automaticamente</p>";
			html += "</div>";
		} else {
			html += "<div class='ticket-grid'>";
			for(const auto& ticket: tickets) {
				std::string status_class = (ticket.estado == "PENDIENTE") ? "pendiente" : "en_proceso";
				std::string status_badge_class = (ticket.estado == "PENDIENTE") ? "status-pendiente" : "status-en_proceso";

				html += "<div class='ticket-card " + status_class + "'>";
				std::string metodo_str;
				switch(ticket.metodo_pago) {
					case 'E': metodo_str = "Efectivo"; break;
					case 'T': metodo_str = "Tarjeta"; break;
					case 'C': metodo_str = "Cortesia"; break;
					default: metodo_str = "Desconocido"; break;
				}

				html += "<div class='ticket-header'>";
				html += "<span class='ticket-id'>" + metodo_str + "</span>";
				html += "<span class='status-badge " + status_badge_class + "'>" + ticket.estado + "</span>";
				html += "</div>";
				html += "<div class='ticket-vendor'><strong>" + ticket.vendor_name + "</strong></div>";
				html += "<div style='color: #666; font-size: 12px;'>" + ticket.fecha_creacion + "</div>";

				html += "<div class='ticket-items'>";
				for(const auto& item : ticket.items) {
					html += "<div class='ticket-item'>";
					html += "<span>" + item.name + "</span>";
					html += "<span><strong>x" + std::to_string(item.quantity) + "</strong></span>";
					html += "</div>";
				}
				html += "</div>";

				html += "<div class='ticket-actions'>";
				html += "<form method='POST' action='/bar/ticket/status' style='display:inline;'>";
				html += "<input type='hidden' name='ticket_id' value='" + ticket.id + "'>";
				html += "<input type='hidden' name='estado' value='COMPLETADO'>";
				html += "<button type='submit' class='btn btn-success'>Completado</button>";
				html += "</form>";
				html += "</div>";

				html += "</div>";
			}
			html += "</div>";
		}

		html += "<a href='/bar' class='btn btn-primary refresh-btn'>Actualizar</a>";
		html += "</div>";

		html += R"(
			<script>
				setTimeout(function() { location.reload(); }, 5000);
			</script>
		)";

		return wrap_html("Barra - Tickets",html);
	}
}
