#include <sstream>
#include <iomanip>
#include <views/HtmlTemplates.h>
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
			</div>
		</div>
		)";

		return wrap_html("Login",html);
	}

	std::string vendor_login_page(const std::string& error) {
		std::string html = R"(
			<div class="container login-box">
				<h1>Accesso Vendedor</h1>
				<form method="POST" action="/login/vendor">
					<div class="form-group">
						<label>Nombre:</label>
						<input type="text" name="username" placeholder="Ingrese su nombre" required>
					</div>
		)";

		if(!error.empty()) {
			html += "<div class='alert alert-danger'>" + error + "</div>";
		}

		html += R"(
				<button type="submit" class="btn btn-success" style="width: 100%;">Entrar</button>
			</form>
			<br>
			<a href="/" class="btn btn-warning">Volver</a>
		</div>
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
			<a href="/" class="btn btn-warning">Volver</a>
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
		    html += "<a href='/reports/sales' class='menu-item'>Reporte de Ventas</a>";
	    }

	    html += "</div></div>";
	    return wrap_html("Inicio", html);
	}

	std::string cart_page(Session* session, const std::string& message) {
		auto products = InventoryService::get_instance().get_all_products();

		std::string html = "<div class='container'>";
		html += "<div class='header'><h1>Carrito de Compras</h1>";
		html += "<a href='/home' class='btn btn-primary'>Volver al Menu</a></div>";

		if(!message.empty()) {
			html += "<div class='alert alert-info'>" + message + "</div>";
		}

		html += "<h3>Agregar Producto</h3>";
		html += "<form method='POST' action='/cart/add' class='inline-form'>";

		html += "<select name='product_id' id='product-select' style='flex: 2;'>";

		std::string stock_json = "{";
		bool first = true;

		for(const auto& p : products) {
			if(p.quantity > 0) {
				html+= "<option value='" + p.id + "'>" + p.name + " - $" + format_price(p.sale_price) + " (Stock: " + std::to_string(p.quantity) + ")</option>";

				if(!first) stock_json += ",";
				stock_json += "\"" + p.id + "\":" + std::to_string(p.quantity);
				first = false;
			}
		}
		stock_json += "}";

		html += "</select>";
		html += "<input type='number' id='quantity-input' name='quantity' placeholder='Cant.' min='1' value='1' required style='flex: 1; max-width: 100px;'>";
		html += "<button type='submit' class='btn btn-success'>Agregar</button>";
		html += "</form>";

		html += "<script>";
		html += "const stocks = " + stock_json + ";";
		html += R"(
				const select = document.getElementById('product-select');
				const input = document.getElementById('quantity-input');

				function updateMax() {
					const selectedId = select.value;
					const maxStock = stocks[selectedId] || 1;
					input.max = maxStock;
					// If current value is higher than new max, reset it
					if (parseInt(input.value) > maxStock) {
						input.value = maxStock;
					}
				}

				select.addEventListener('change', updateMax);
				window.onload = updateMax; // Initialize on page load
			</script>
		)";

		html += "<h3 style='margin-top: 30px;'>Carrito Actual</h3>";

		auto& cart = session->cart;
		if(cart.empty()) {
			html += "<div class='alert alert-info'>El carrito esta vacio.</div>";
		} else {
			html += "<table><tr><th>Producto</th><th>Precio</th><th>Cantidad</th><th>Subtotal</th><th>Acciones</th></tr>";

			double total = 0;
			for(const auto& item : cart) {
				double subtotal = item.unit_price * item.quantity;
				total += subtotal;

				// Find the stock for the current product
				int max_stock = 1;
				for (const auto& p : products) {
					if(p.id == item.product_id) {
						max_stock = p.quantity;
						break;
					}
				}

				html += "<tr><td>" + item.name + "</td>";
				html += "<td>$" + format_price(item.unit_price) + "</td>";
				html += "<td>";
				html += "<form method='POST' action='/cart/update' class='inline-form'>";
				html += "<input type='hidden' name='product_id' value='" + item.product_id + "'>";
				html += "<input type='number' name='quantity' value='" + std::to_string(item.quantity) + "' min='1' max='" + std::to_string(max_stock) + " style='width: 60px;'>";
				html += "<button type='submit' class='btn btn-info'>OK</button>";
				html += "</form></td>";
				html += "<td>$" + format_price(subtotal) + "</td>";
				html += "<td><a href='/cart/remove/" +  item.product_id + "' class='btn btn-danger'>Quitar</a></td></tr>";
			}

			html += "</table>";
			html += "<div class='cart-summary'><span class='total'>Total: $" + format_price(total) + "</span>";
			html += " <a href='/cart/clear' class='btn btn-danger'>Vaciar</a>";
			html += " <a href='/cart/confirm' class='btn btn-success'>Confirmar</a></div>";
		}

		html += "</div>";
		return wrap_html("Carrito",html);
	}

	std::string cart_confirm_page(Session* session,const std::string& message) {
		std::string html = "<div class='container'>";
		html += "<div class='header'><h1>Confirmar Ordern</h1>";
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

		html += "<form method='POST' action='/cart/checkout'>";
		html += "<div class='form-group'><label>Metodo de Pago:</label>";
		html += "<select name='payment_method'>";
		html += "<option value='E'>Efectivo</option>";
		html += "<option value='T'>Tarjeta</option>";
		html += "<option vlaue='C'>Cortesia</option>";
		html += "</select></div>";
		html += "<button type='submit' class='btn btn-success' style='font-size: 18px; padding: 15px 40px;'>Completar Venta</button>";
		html += "<a href='/cart' class='btn btn-primary'>Volver al Carrito</a>";
		html += "</form>";

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

		std::string html = "<div class='container'>";
		html += "<div class='header'><h1>Gestionar Inventario</h1>";
		html += "<a href='/home' class='btn btn-primary'>Volver al Menu</a></div>";

		if(!message.empty()) {
			html += "<div class='alert alert-success'>" + message + "</div>";
		}

		html += R"(
			<h3>Agregar Producto</h3>
			<form method="POST" action="/inventory/add" class="inline-form">
				<input type="text" name="name" placeholder="Nombre del producto" required style="flex: 2;">
				<input type="number" name="price" placeholder="Precio" step="1.00" min="0" required style="flex: 1;">
				<input type="number" name="quantity" placeholder="Cantidad" min="0" required style="flex: 1;">
				<button type="submit" class="btn btn-success">Agregar</button>
			</form>
		)";

		html += "<h3 style='margin-top: 30px;'>Productos</h3>";
		html += "<table><tr><th>ID</th><th>Producto</th><th>Precio</th><th>Stock</th><th>Acciones</th></tr>";

		for(const auto& p : products) {
			html += "<tr><td>" + p.id + "</td>";
			html += "<td>";
			html += "<form method='POST' action='/inventory/update' class='inline-form'>";
			html += "<input type='hidden' name='id' value='" + p.id + "'>";
			html += "<input type='text' name='name' value='" + p.name + "' style='width: 150px;'>";
			html += "</td><td>";
			html += "<input type='number' name='price' value='" + format_price(p.sale_price) + "' step='1.00' min='0' style='width: 80px;'>";
			html += "</td><td>";
			html += "<input type='number' name='quantity' value='" + std::to_string(p.quantity) + "'min='0' style='width: 80px;'>";
			html += "</td><td>";
			html += "<button type='submit' class='btn btn-info'>Guardar</button>";
			html += "</form></td>";
			html += "<td><a href='/inventory/delete/" + p.id + "' class='btn btn-danger' onclick='return confirm(\"Eliminar producto?\")'>Eliminar</a></td></tr>";
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

		html += "<div class='cart-summary'>";
		html += "<h3>Resumen</h3>";
		html += "<p>Total Transacciones: <strong>" + std::to_string(report.total_transactions) + "</strong><br>";
		html += "<span style='color #666666;'>&nbsp;Efectivo: <strong>" + std::to_string(report.transactions.at('E').transaction_count) + "</strong></span><br>";
		html += "<span style='color #666666;'>&nbsp;Tarjeta : <strong>" + std::to_string(report.transactions.at('T').transaction_count) + "</strong></span><br>";
		html += "<span style='color #666666;'>&nbsp;Cortesia: <strong>" + std::to_string(report.transactions.at('C').transaction_count) + "</strong></span><br>";
		html += "</p>";

		html += "<p class='total'>Ingresos Totales: <strong>$" + format_price(report.total_revenue) + "</strong><br>";
		html += "<span style='color #666666;'>&nbsp;Efectivo: <strong>$" + format_price(report.transactions.at('E').revenue) + "</strong></span><br>";
		html += "<span style='color #666666;'>&nbsp;Tarjeta : <strong>$" + format_price(report.transactions.at('T').revenue) + "</strong></span><br>";
		html += "<span style='color #666666;'>&nbsp;Cortesia: <strong>$" + format_price(report.transactions.at('C').revenue) + "</strong></span><br>";
		html += "</p>";
		html += "</div></div>";

		return wrap_html("Reprte de Ventas",html);
	}
}
