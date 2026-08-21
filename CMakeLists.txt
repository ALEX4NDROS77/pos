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
)

add_executable(pos_server ${SOURCES})

target_include_directories(pos_server PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(pos_server PRIVATE httplib::httplib sqlite3_lib)
