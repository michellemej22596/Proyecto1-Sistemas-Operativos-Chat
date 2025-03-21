#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <iostream>
#include <string>
#include <system_error> // Para manejar errores con std::error_code

typedef websocketpp::client<websocketpp::config::asio_client> client;

// Manejador de mensajes entrantes
void on_message(websocketpp::connection_hdl hdl, websocketpp::client<websocketpp::config::asio_client>::message_ptr msg) {
    std::cout << "📩 Mensaje recibido: " << msg->get_payload() << std::endl;
}

int main() {
    client c;
    
    try {
        // Configuración del cliente
        c.set_access_channels(websocketpp::log::alevel::none);
        c.clear_access_channels(websocketpp::log::alevel::frame_payload);
        
        c.init_asio();
        c.set_message_handler(&on_message);

        std::string uri = "ws://127.0.0.1:8080";  

        std::cerr << "🔄 Intentando conectar a " << uri << "..." << std::endl;

        websocketpp::lib::error_code ec;
        client::connection_ptr con = c.get_connection(uri, ec);
        
        if (ec) {
            std::cerr << "❌ Error al obtener la conexión: " << ec.message() << std::endl;
            return 1;
        }

        std::cerr << "✅ Conexión establecida con éxito!" << std::endl;

        c.connect(con);
        c.run(); // Mantiene la conexión abierta

    } catch (websocketpp::exception const &e) {
        std::cerr << "⚠️ Error: " << e.what() << std::endl;
    }
    
    return 0;
}
