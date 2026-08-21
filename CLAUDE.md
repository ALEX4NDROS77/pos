# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

`pos_webservice` — a C++17 point-of-sale web server for a bar/event (products, cart, checkout, pending-order tickets, vendor management, sales reports). It's a single self-contained binary: `httplib` (bundled HTTP server) + SQLite3, both pulled in via CMake `FetchContent`. There is no separate frontend build in the npm/webpack sense, but HTML/CSS markup lives in its own files under `templates/` rather than as C++ string literals — see Views below.

The server is white-labelable: business name, tagline, currency symbol, theme colors, and logo/mascot/favicon images can be overridden per deployment via an optional `branding.conf` file read at startup (see `AppConfig` below), without recompiling.

## Build & run

```bash
cmake -B build -S .        # first run fetches httplib and the SQLite amalgamation from the network
cmake --build build -j     # or: cd build && make
./build/pos_server          # run from the directory where you want pos.db / pos.log created
```

The server listens on `http://0.0.0.0:8080`. `pos.db` (SQLite file) and `pos.log` are created relative to the current working directory on startup, and the schema/sample data are (re)seeded automatically if the `PRODUCTOS` table is empty — see `insert_sample_data()` in `src/services/DatabaseService.cpp` for the seeded logins. If a `branding.conf` file (`KEY=VALUE` lines) exists in the working directory, it's loaded at startup by `AppConfig` to override branding defaults — see Architecture below.

Editing a file under `templates/` requires a rebuild to take effect (they're compiled into the binary as byte arrays, not read from disk at runtime — see Views below), but does **not** require touching any `.cpp` file.

There is no unit test suite. `stress_test.sh <sessions> <requests>` is a manual load-test script that drives a **running** server via `curl` (logs in N vendor sessions concurrently, adds items to cart, checks out) — run it against `./build/pos_server` while it's up, not as part of the build.

`compile_commands.json` is emitted into `build/` (`CMAKE_EXPORT_COMPILE_COMMANDS ON`) for clangd/YCM-style tooling.

## Architecture

Strict layered flow: **Controller → Service → DatabaseService (raw SQLite3 C API)**, with **HtmlTemplates** producing the HTML response. There is no ORM and no query builder — every service method hand-writes `sqlite3_prepare_v2` / `sqlite3_bind_*` / `sqlite3_step` calls, wrapped in the RAII helper `SqliteStatement` (`include/utils/SqliteStatement.h`: prepares in the constructor, finalizes in the destructor, thin `bind`/`step`/`exec`/`column_*` helpers) rather than hand-rolled prepare/finalize pairs.

- **Controllers** (`include/controllers/`, `src/controllers/`) — one class per feature area (`AuthController`, `CartController`, `InventoryController`, `VendorController`, `ReportController`, `TicketController`). Each exposes a single static `register_routes(httplib::Server&)` called from `src/main.cpp`. Handlers get the session via `SessionService::get_instance().require_session(req,res)` (any authenticated role) or `require_role(req,res,"admin")` (role-gated) — both return `nullptr` and have already called `res.set_redirect("/")` on failure, so the call site is just `if(!session) return;`. Route params come via regex capture groups (`req.matches[n]`) for path-style routes (e.g. `/cart/remove/(.+)`) or form fields via `req.get_param_value(...)`.
- **Services** (`include/services/`, `src/services/`) — Meyers singletons (`static X& get_instance()`), holding the business logic and all SQL. Every write/read against `DatabaseService::get_connection()` takes `DatabaseService::get_instance().get_mutex()` for the duration of the query, since there's a single shared `sqlite3*` connection. `SalesService::checkout()` wraps its whole write sequence (order, per-item sale + stock update, ticket + ticket items) in one `BEGIN IMMEDIATE`/`COMMIT`/`ROLLBACK` transaction and reuses one prepared statement per query across the per-item loop (`reset()` between iterations) instead of re-preparing per cart item.
- **`DatabaseService`** — singleton owning the one `sqlite3*` connection, its mutex, table creation (`CREATE TABLE IF NOT EXISTS ...`), and sample-data seeding.
- **`SessionService`** — sessions are an **in-memory** `std::map<session_id, Session>`, not persisted; they're lost on restart. The shopping cart lives on `Session::cart` (in memory) until `SalesService::checkout()` commits it to `ORDENES`/`VENTAS`/`TICKETS` and clears it. The admin password is a hardcoded constant in `include/services/SessionService.h` (`ADMIN_PASSWORD`); vendor credentials live in the `VENDEDORES` table instead. `require_session`/`require_role` (see Controllers above) are the shared auth-guard entry points.
- **Models** (`include/models/`) — plain structs only (`Session`, `CartItem`, `Product`, `Order`, `Sale`, `Ticket`, `Vendor`), no behavior.
- **`AppConfig`** (`include/utils/AppConfig.h`) — Meyers singleton holding white-label branding: `business_name`, `tagline`, `currency_symbol`, `primary_color`/`accent_color`/`bg_color`, and optional `logo_path`/`mascot_path`/`favicon_path` disk overrides. `AppConfig::get_instance().load("branding.conf")` is called once from `main.cpp`; a missing file or missing keys silently fall back to defaults (the look of the original hardcoded build). `render()`-time placeholders in `templates/style.css` and page templates pull from it.
- **Views** (`src/views/HtmlTemplates.cpp` + `templates/`) — HTML/CSS markup lives in `.html`/`.css` files under `templates/` (page-level files) and `templates/partials/` (repeated rows/conditional blocks, e.g. `product_row.html`, `ticket_item.html`). At build time, CMake's `embed_resource()` (see `CMakeLists.txt` / `cmake/EmbedResource.cmake`) converts each template into a `generated/<name>.h` byte-array header — same mechanism as the compiled-in image assets, so there's no runtime filesystem dependency, but changing a template requires a rebuild. `TemplateEngine::render()` (`include/utils/TemplateEngine.h`) does plain `{{KEY}}` substitution — no loops/conditionals in the engine; `HtmlTemplates.cpp` resolves those in C++ by looping over data to build a partial-per-row string, or choosing which partial (or an empty string) to substitute into a slot. `wrap_html()` renders `templates/layout.html` for the shared page shell.
- **`StaticAssets`** (`include/utils/StaticAssets.h`) — images/favicon/CSS are compiled in as byte arrays (generated headers `include/utils/*_png.h`, `favicon_ico.h`, `generated/style_css.h`) and served from memory by default. If `AppConfig`'s `logo_path`/`mascot_path`/`favicon_path` is non-empty and the file is readable, that file's bytes are read once at startup (`resolve_override()`) and served instead — no route paths change.

### Roles

Two roles gate access via `SessionService::require_role(req,res,"admin")`: `"admin"` (full access: inventory management, vendor management, sales reports) and `"vendor"` (cart/checkout, plus completing their own pending tickets). There is no separate bar role or bar screen — the vendor who takes an order is responsible for seeing and completing it; pending tickets and a "Completar" action appear directly on the vendor/admin home page (`/home`), backed by `POST /ticket/complete` in `TicketController`. Login flows differ per role — vendor login authenticates against the `VENDEDORES` table, admin login checks the hardcoded password in `SessionService`.

### Domain naming

SQLite tables/columns and DB-facing identifiers are Spanish (`PRODUCTOS`, `VENDEDORES`, `ORDENES`, `VENTAS`, `TICKETS`, `TICKET_ITEMS`), reflecting the domain (payment methods `'E'`=Efectivo, `'T'`=Tarjeta, `'C'`=Cortesía). C++ identifiers, function/class names are English. Keep new DB columns/tables in Spanish to stay consistent with the existing schema.

### Style notes

- Indentation is tabs throughout — match existing files.
- Logging goes through the `LOG_DEBUG/INFO/WARNING/ERROR` macros (`include/utils/Logger.h`), writing to `pos.log`; log lines conventionally start with `ClassName::method_name - message`.
