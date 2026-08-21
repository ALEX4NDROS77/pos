#ifndef UTILS_STATIC_ASSETS_H
#define	UTILS_STATIC_ASSETS_H	1

#include <httplib.h>
#include <utils/diablo_luna_png.h>
#include <utils/diablito_sentado_png.h>
#include <utils/favicon_ico.h>
#include <utils/mvte_png.h>
#include <utils/AppConfig.h>
#include <utils/TemplateEngine.h>
#include <style_css.h>
#include <fstream>
#include <sstream>
#include <utility>

namespace StaticAssets {
	struct Resource {
		const unsigned char* data;
		unsigned int size;
		const char* mime_type;
	};

	inline const char* mime_type_for_path(const std::string& path) {
		auto ends_with = [&](const char* suffix) {
			size_t len = std::string(suffix).size();
			return path.size() >= len && path.compare(path.size() - len,len,suffix) == 0;
		};
		if(ends_with(".ico")) return "image/x-icon";
		if(ends_with(".jpg") || ends_with(".jpeg")) return "image/jpeg";
		if(ends_with(".gif")) return "image/gif";
		if(ends_with(".svg")) return "image/svg+xml";
		return "image/png";
	}

	// Reads a disk-override asset once at startup; returns an empty first element if the
	// path is blank or the file can't be read, so callers fall back to the compiled-in asset.
	inline std::pair<std::string,const char*> resolve_override(const std::string& path) {
		if(path.empty()) return {"",nullptr};
		std::ifstream file(path,std::ios::binary);
		if(!file.good()) return {"",nullptr};
		std::ostringstream ss;
		ss << file.rdbuf();
		return {ss.str(),mime_type_for_path(path)};
	}

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
		server.Get("/assets/style.css",[](const httplib::Request&,httplib::Response& res) {
			static std::string css = TemplateEngine::render(TemplateEngine::load(style_css,style_css_len),{
				{"PRIMARY_COLOR",AppConfig::get_instance().primary_color()},
				{"ACCENT_COLOR",AppConfig::get_instance().accent_color()},
				{"BG_COLOR",AppConfig::get_instance().bg_color()},
			});
			res.set_content(css,"text/css");
		});
		auto favicon_override = resolve_override(AppConfig::get_instance().favicon_path());
		auto logo_override = resolve_override(AppConfig::get_instance().logo_path());
		auto mascot_override = resolve_override(AppConfig::get_instance().mascot_path());

		server.Get("/favicon.ico",[favicon_override](const httplib::Request&,httplib::Response& res) {
			if(!favicon_override.first.empty()) {
				res.set_content(favicon_override.first,favicon_override.second);
				return;
			}
			auto resrc = StaticAssets::get_favicon();
			res.set_content(reinterpret_cast<const char*>(resrc.data), resrc.size, resrc.mime_type);
		});
		server.Get("/assets/mvte.png", [logo_override](const httplib::Request&, httplib::Response& res) {
			if(!logo_override.first.empty()) {
				res.set_content(logo_override.first,logo_override.second);
				return;
			}
			auto resrc = StaticAssets::get_mvte_logo();
			res.set_content(reinterpret_cast<const char*>(resrc.data), resrc.size, resrc.mime_type);
		});
		server.Get("/assets/diablo_luna.png", [mascot_override](const httplib::Request&, httplib::Response& res) {
			if(!mascot_override.first.empty()) {
				res.set_content(mascot_override.first,mascot_override.second);
				return;
			}
			auto resrc = StaticAssets::get_diablo_luna();
			res.set_content(reinterpret_cast<const char*>(resrc.data), resrc.size, resrc.mime_type);
		});
		server.Get("/assets/diablito_sentado.png", [mascot_override](const httplib::Request&, httplib::Response& res) {
			if(!mascot_override.first.empty()) {
				res.set_content(mascot_override.first,mascot_override.second);
				return;
			}
			auto resrc = StaticAssets::get_diablito_sentado();
			res.set_content(reinterpret_cast<const char*>(resrc.data),resrc.size,resrc.mime_type);
		});
	}
}

#endif
