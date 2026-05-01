#include <iostream>
#include <httplib.h>

int main(void) {
    httplib::Server server;

    // Serve the welcome page
    server.Get("/", [](const httplib::Request &, httplib::Response &res) {
		    FILE* fp = fopen("assets/welcome.html","r");
		    if(!fp) {
			std::cout << "Ocurrio un error al abrir el archivo html" << std::endl;
			return;
		    }

		    char buffer[1024];
		    std::string content;
		    while(fgets(buffer,sizeof(buffer),fp)) {
			    content += buffer;
		    }
		    fclose(fp);

		    res.set_content(content,"text/html");
    });

    // Start the server
    std::cout << "Server is running on http://0.0.0.0:8080" << std::endl;
    server.listen("0.0.0.0", 8080);

    return 0;
}
