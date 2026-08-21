#ifndef	UTILS_APPCONFIG_H
#define	UTILS_APPCONFIG_H	1

#include <string>

class AppConfig {
	public:
		static AppConfig& get_instance();

		void load(const std::string& path);

		const std::string& business_name() const { return m_business_name; }
		const std::string& tagline() const { return m_tagline; }
		const std::string& currency_symbol() const { return m_currency_symbol; }
		const std::string& primary_color() const { return m_primary_color; }
		const std::string& accent_color() const { return m_accent_color; }
		const std::string& bg_color() const { return m_bg_color; }
		const std::string& logo_path() const { return m_logo_path; }
		const std::string& mascot_path() const { return m_mascot_path; }
		const std::string& favicon_path() const { return m_favicon_path; }

	private:
		AppConfig() = default;
		AppConfig(const AppConfig&) = delete;
		AppConfig& operator=(const AppConfig&) = delete;

		std::string m_business_name = "POS System";
		std::string m_tagline = "Punto de Venta";
		std::string m_currency_symbol = "$";
		std::string m_primary_color = "#d41414";
		std::string m_accent_color = "#ff4500";
		std::string m_bg_color = "#000000";

		// Optional filesystem overrides for compiled-in assets; empty means "use the default".
		std::string m_logo_path;
		std::string m_mascot_path;
		std::string m_favicon_path;
};

#endif

