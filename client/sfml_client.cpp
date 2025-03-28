#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>

typedef websocketpp::client<websocketpp::config::asio_client> client;
typedef websocketpp::connection_hdl connection_hdl;

client ws_client;

void on_message(client* c, connection_hdl hdl, client::message_ptr msg) {
    std::cout << "Mensaje recibido: " << msg->get_payload() << std::endl;
}

int main() {
    // Inicializa el cliente WebSocket
    ws_client.init_asio();
    ws_client.set_message_handler(&on_message);
    
    // Conecta al servidor WebSocket
    std::string uri = "ws://localhost:8080";
    websocketpp::lib::error_code ec;
    client::connection_ptr con = ws_client.get_connection(uri, ec);

    if (ec) {
        std::cerr << "Error al conectar al servidor WebSocket: " << ec.message() << std::endl;
        return 1;
    }

    ws_client.connect(con);

    // Configuración de la ventana de SFML
    sf::RenderWindow window(sf::VideoMode(800, 600), "Cliente de Chat");

    sf::String inputText;
    sf::Text inputTextDisplay;
    sf::Font font;
    
    if (!font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
        std::cerr << "No se pudo cargar la fuente." << std::endl;
        return -1;
    }

    inputTextDisplay.setFont(font);
    inputTextDisplay.setCharacterSize(24);

    // Bucle principal
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode == 13) {  // Enter key to send message
                    // Enviar el mensaje al servidor WebSocket
                    std::string message = inputText.toAnsiString();
                    ws_client.send(con, message, websocketpp::frame::opcode::text);
                    inputText = "";  // Limpiar el campo de entrada
                } else if (event.text.unicode == 8 && inputText.getSize() > 0) {  // Backspace
                    inputText = inputText.substring(0, inputText.getSize() - 1);
                } else {
                    inputText += event.text.unicode;  // Capturar lo que el usuario escribe
                }
            }
        }

        // Limpiar y dibujar
        window.clear();
        inputTextDisplay.setString(inputText);
        window.draw(inputTextDisplay);
        window.display();
    }

    // Cerrar la conexión WebSocket y salir
    ws_client.stop();
    return 0;
}