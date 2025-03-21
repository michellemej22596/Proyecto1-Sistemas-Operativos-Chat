#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <mutex>

typedef websocketpp::client<websocketpp::config::asio_client> client;
std::vector<std::string> messages;  // Almacena los mensajes recibidos
std::vector<std::string> users;     // Lista de usuarios conectados
std::mutex messages_mutex;

// Función para manejar los mensajes recibidos
void on_message(websocketpp::connection_hdl hdl, client::message_ptr msg) {
    std::lock_guard<std::mutex> lock(messages_mutex);
    std::string payload = msg->get_payload();

    if (payload.rfind("[USERS] ", 0) == 0) {
        users.clear();
        std::string userList = payload.substr(8);
        size_t pos = 0;
        while ((pos = userList.find(',')) != std::string::npos) {
            users.push_back(userList.substr(0, pos));
            userList.erase(0, pos + 1);
        }
        users.push_back(userList); // Último usuario
    } else {
        messages.push_back("📩 " + payload);
    }
}

int main() {
    client c;
    std::string uri = "ws://127.0.0.1:8080";

    try {
        c.set_access_channels(websocketpp::log::alevel::none);
        c.init_asio();
        c.set_message_handler(&on_message);

        websocketpp::lib::error_code ec;
        client::connection_ptr con = c.get_connection(uri, ec);
        if (ec) {
            std::cerr << "❌ Error al conectar: " << ec.message() << std::endl;
            return 1;
        }

        c.connect(con);
        std::thread client_thread([&c]() { c.run(); });

        // Interfaz con SFML
        sf::RenderWindow window(sf::VideoMode(900, 600), "💬 Chat Cliente", sf::Style::Close);
        sf::Font font;
        if (!font.loadFromFile("/usr/share/fonts/truetype/msttcorefonts/Arial.ttf")) {
            std::cerr << "⚠️ No se pudo cargar la fuente Arial.ttf" << std::endl;
            return 1;
        }

        // Estilos de texto
        sf::Text chatText, inputTextDisplay, usersList;
        chatText.setFont(font);
        chatText.setCharacterSize(18);
        chatText.setPosition(20, 20);
        chatText.setFillColor(sf::Color::White);

        inputTextDisplay.setFont(font);
        inputTextDisplay.setCharacterSize(24);
        inputTextDisplay.setPosition(20, 550);
        inputTextDisplay.setFillColor(sf::Color::Green);

        usersList.setFont(font);
        usersList.setCharacterSize(18);
        usersList.setPosition(700, 20);
        usersList.setFillColor(sf::Color::Cyan);

        sf::RectangleShape inputBox(sf::Vector2f(860, 40));
        inputBox.setPosition(20, 540);
        inputBox.setFillColor(sf::Color(50, 50, 50));

        std::string inputText;

        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    window.close();
                else if (event.type == sf::Event::TextEntered) {
                    if (event.text.unicode == 13) {  // Enter para enviar mensaje
                        if (!inputText.empty()) {
                            con->send(inputText);
                            std::lock_guard<std::mutex> lock(messages_mutex);
                            messages.push_back("📝 Tú: " + inputText);
                            inputText.clear();
                        }
                    } else if (event.text.unicode == 8) {  // Backspace
                        if (!inputText.empty()) {
                            inputText.pop_back();
                        }
                    } else {
                        inputText += static_cast<char>(event.text.unicode);
                    }
                }
            }

            // Actualizar chat
            std::lock_guard<std::mutex> lock(messages_mutex);
            std::string chatHistory;
            for (const auto& msg : messages) {
                chatHistory += msg + "\n";
            }
            chatText.setString(chatHistory);
            inputTextDisplay.setString(inputText);

            // Mostrar usuarios conectados
            std::string userList = "👥 Usuarios Conectados:\n";
            for (const auto& user : users) {
                userList += "🟢 " + user + "\n";
            }
            usersList.setString(userList);

            window.clear(sf::Color(30, 30, 30));
            window.draw(chatText);
            window.draw(inputBox);
            window.draw(inputTextDisplay);
            window.draw(usersList);
            window.display();
        }

        client_thread.join();

    } catch (websocketpp::exception const &e) {
        std::cerr << "⚠️ Error: " << e.what() << std::endl;
    }

    return 0;
}
