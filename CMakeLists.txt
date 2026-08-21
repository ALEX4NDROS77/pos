cmake_minimum_required(VERSION 3.14)
project(pos_webservice)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

# Fetch cpp-httplib
FetchContent_Declare(
	httplib
	GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
	GIT_TAG v0.14.1
)
FetchContent_MakeAvailable(httplib)

# Fetch SQLite3
FetchContent_Declare(
	sqlite3
	URL https://www.sqlite.org/2024/sqlite-amalgamation-3450000.zip
	DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(sqlite3)

# Create SQLite library
add_library(sqlite3_lib STATIC ${sqlite3_SOURCE_DIR}/sqlite3.c)
target_include_directories(sqlite3_lib PUBLIC ${sqlite3_SOURCE_DIR})

# Embed template/CSS assets as byte-array headers at build time
set(GENERATED_DIR ${CMAKE_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_DIR})

set(EMBEDDED_HEADERS "")
function(embed_resource name input_relpath)
	set(input ${CMAKE_SOURCE_DIR}/${input_relpath})
	set(output ${GENERATED_DIR}/${name}.h)
	add_custom_command(
		OUTPUT ${output}
		COMMAND ${CMAKE_COMMAND} -DINPUT=${input} -DOUTPUT=${output} -DVARNAME=${name} -P ${CMAKE_SOURCE_DIR}/cmake/EmbedResource.cmake
		DEPENDS ${input} ${CMAKE_SOURCE_DIR}/cmake/EmbedResource.cmake
		COMMENT "Embedding ${input_relpath} -> generated/${name}.h"
	)
	set(EMBEDDED_HEADERS ${EMBEDDED_HEADERS} ${output} PARENT_SCOPE)
endfunction()

embed_resource(layout_html templates/layout.html)
embed_resource(style_css templates/style.css)

embed_resource(login_html templates/login.html)
embed_resource(admin_login_html templates/admin_login.html)
embed_resource(vendor_login_html templates/vendor_login.html)
embed_resource(homepage_html templates/homepage.html)
embed_resource(cart_html templates/cart.html)
embed_resource(cart_confirm_html templates/cart_confirm.html)
embed_resource(cart_confirm_message_html templates/cart_confirm_message.html)
embed_resource(inventory_view_html templates/inventory_view.html)
embed_resource(inventory_manage_html templates/inventory_manage.html)
embed_resource(sales_report_html templates/sales_report.html)
embed_resource(vendor_manage_html templates/vendor_manage.html)

embed_resource(alert_danger_html templates/partials/alert_danger.html)
embed_resource(alert_success_html templates/partials/alert_success.html)
embed_resource(alert_info_html templates/partials/alert_info.html)
embed_resource(vendor_option_html templates/partials/vendor_option.html)
embed_resource(admin_menu_items_html templates/partials/admin_menu_items.html)
embed_resource(ticket_empty_html templates/partials/ticket_empty.html)
embed_resource(ticket_mini_html templates/partials/ticket_mini.html)
embed_resource(ticket_item_html templates/partials/ticket_item.html)
embed_resource(product_image_html templates/partials/product_image.html)
embed_resource(product_image_placeholder_html templates/partials/product_image_placeholder.html)
embed_resource(product_card_html templates/partials/product_card.html)
embed_resource(cart_row_html templates/partials/cart_row.html)
embed_resource(cart_table_html templates/partials/cart_table.html)
embed_resource(order_row_html templates/partials/order_row.html)
embed_resource(manage_inventory_link_html templates/partials/manage_inventory_link.html)
embed_resource(inventory_view_row_html templates/partials/inventory_view_row.html)
embed_resource(product_thumb_html templates/partials/product_thumb.html)
embed_resource(product_thumb_placeholder_html templates/partials/product_thumb_placeholder.html)
embed_resource(inventory_manage_row_html templates/partials/inventory_manage_row.html)
embed_resource(vendor_filter_html templates/partials/vendor_filter.html)
embed_resource(vendor_filter_link_html templates/partials/vendor_filter_link.html)
embed_resource(sales_order_row_html templates/partials/sales_order_row.html)
embed_resource(sales_report_body_html templates/partials/sales_report_body.html)
embed_resource(vendor_active_html templates/partials/vendor_active.html)
embed_resource(vendor_inactive_html templates/partials/vendor_inactive.html)
embed_resource(vendor_row_html templates/partials/vendor_row.html)

add_custom_target(generate_templates DEPENDS ${EMBEDDED_HEADERS})

# Source files
set(SOURCES
	src/main.cpp
	src/services/DatabaseService.cpp
	src/services/InventoryService.cpp
	src/services/CartService.cpp
	src/services/SalesService.cpp
	src/services/SessionService.cpp
	src/services/VendorService.cpp
	src/services/TicketService.cpp
	src/controllers/AuthController.cpp
	src/controllers/CartController.cpp
	src/controllers/InventoryController.cpp
	src/controllers/ReportController.cpp
	src/controllers/VendorController.cpp
	src/controllers/TicketController.cpp
	src/views/HtmlTemplates.cpp
	src/utils/Utils.cpp
	src/utils/Logger.cpp
	src/utils/AppConfig.cpp
	src/utils/TemplateEngine.cpp
)

add_executable(pos_server ${SOURCES})
add_dependencies(pos_server generate_templates)

target_include_directories(pos_server PRIVATE ${CMAKE_SOURCE_DIR}/include ${GENERATED_DIR})
target_link_libraries(pos_server PRIVATE httplib::httplib sqlite3_lib)
