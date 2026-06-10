#ifndef UTILS_STATIC_ASSETS_H
#define	UTILS_STATIC_ASSETS_H	1

#include <httplib.h>
#include <utils/diablo_luna_png.h>
#include <utils/diablito_sentado_png.h>
#include <utils/favicon_ico.h>
#include <utils/mvte_png.h>

namespace StaticAssets {
	struct Resource {
		const unsigned char* data;
		unsigned int size;
		const char* mime_type;
	};

	inline Resource get_mvte_logo() {
		return {mvte_png,mvte_png_len,"image/png"};
	}

	inline Resource get_diablo_luna() {
		return {diablo_luna_png,diablo_luna_png_len,"image/png"};
	}

	inline Resource get_diablito_sentado() {
		return {diablito_sentado_png,diablito_sentado_png_len,"image/png"};
	}

	inline Resource get_favicon() {
		return {favicon_ico,favicon_ico_len,"image/x-icon"};
	}

	static void register_routes(httplib::Server& server) {
		server.Get("/favicon.ico",[](const httplib::Request&,httplib::Response& res) {
			auto resrc = StaticAssets::get_favicon();
			res.set_content(reinterpret_cast<const char*>(resrc.data), resrc.size, resrc.mime_type);
		});
		server.Get("/assets/mvte.png", [](const httplib::Request&, httplib::Response& res) {
			auto resrc = StaticAssets::get_mvte_logo();
			res.set_content(reinterpret_cast<const char*>(resrc.data), resrc.size, resrc.mime_type);
		});
		server.Get("/assets/diablo_luna.png", [](const httplib::Request&, httplib::Response& res) {
			auto resrc = StaticAssets::get_diablo_luna();
			res.set_content(reinterpret_cast<const char*>(resrc.data), resrc.size, resrc.mime_type);
		});
		server.Get("/assets/diablito_sentado.png", [](const httplib::Request&, httplib::Response& res) {
			auto resrc = StaticAssets::get_diablito_sentado();
			res.set_content(reinterpret_cast<const char*>(resrc.data),resrc.size,resrc.mime_type);
		});
	}
}

#endif
