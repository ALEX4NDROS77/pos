# Handoff — continuing on another device

Paste this into a fresh Claude Code session on the other device (after `git pull`/`git clone` so you're on commit `7615575`).

## State

Everything is committed on `master` at commit `7615575` ("WIP: sales report product filter, isles/inventory transfers, live monitoring dashboard"). Working tree is clean. **The build has not been verified yet** — that's the very next step.

Two features landed in this WIP commit:

1. **Sales report product filter** (done, previously build-verified and curl-tested in an earlier part of this session — should still be fine, not touched since).
2. **Isles / inventory transfers / live monitoring dashboard** (new, large feature, implemented per the plan below — NOT yet compiled or tested).

## Read this first

Full design plan: `/home/alexy/.claude/plans/ancient-scribbling-toucan.md` — has the Context, schema, service/controller design, and a 10-step verification checklist at the bottom.

## What was built (feature 2)

- **Schema** (`src/services/DatabaseService.cpp`): new `ISLAS`, `INVENTARIO_ISLA`, `TRANSFERENCIAS` tables; `VENDEDORES.ISLA_ID` and `ORDENES.ISLA_ID` added via a guarded `ALTER TABLE` helper (`add_column_if_missing`); 2 sample isles + vendor assignments seeded in the fresh-DB sample-data branch.
- **Key design decision**: storage stock *is* the existing `PRODUCTOS.CANTIDAD` column (not migrated to a new table) — isles get a separate `INVENTARIO_ISLA` table fed by transfers out of storage. Admin's existing cart/checkout/inventory pages are untouched; only vendor sessions (which now carry `isla_id`) use the new isle-scoped stock path.
- **New services**: `IsleService` (isle CRUD, delete blocked if vendors/inventory attached), `TransferService` (atomic transfer between storage/`"ALMACEN"` and isles, `TRANSFERENCIAS` audit log).
- **Modified services**: `InventoryService` (+ `get_isla_stock`, `get_products_with_isla_stock`), `VendorService` (+ `set_vendor_isle`, isle_id in selects), `SalesService` (+ `get_isle_sales_summary`, `checkout()` now branches on `session->isla_id` — blocks vendor checkout with no isle assigned, decrements the isle's stock instead of storage).
- **New controllers**: `IsleController` (`/isles/manage`, `/isles/add`, `/isles/delete/:id`), `TransferController` (`/transfers`, `/transfers/create`), `MonitoringController` (`/monitoring` full page, `/monitoring/data` HTML-fragment endpoint for polling).
- **Live monitoring**: `templates/monitoring.html` polls `/monitoring/data` every 5s via plain `fetch`/`setInterval` (first such polling code in this codebase) and swaps `#monitoring-body` innerHTML. Per-isle cards show sales by payment method + inventory rows color-coded via a fixed threshold (`stock <= 0` critical/red, `<= 5` warning/orange) — see `stock_class_for()` in `src/views/HtmlTemplates.cpp`.
- All new templates registered in `CMakeLists.txt` via `embed_resource(...)`; all new `.cpp` files added to the `SOURCES` list.

## Next steps (in order)

1. `cmake -B build -S .` then `cmake --build build -j` — **fix any compile errors first**, there may be typos/signature mismatches since this was never compiled.
2. Once it builds, run through the plan file's 10-step verification checklist (fresh DB isle/vendor seeding, isle CRUD + delete-blocking, transfer with insufficient stock, vendor checkout scoped to their isle, `/monitoring` live updates, regression-check admin cart/reports/inventory pages still work as before).
3. Nothing has been pushed to the remote (`github/master`) — only committed locally on this branch.
