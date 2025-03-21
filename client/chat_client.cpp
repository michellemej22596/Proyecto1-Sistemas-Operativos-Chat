#include "websocket_client.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <vector>
#include <string>

// Función para cargar la fuente del texto
sf::Font loadFont() {
    sf::Font font;
    if (!font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
        std::cerr << "⚠️ No se pudo cargar la fuente. Verifica la instalación de fuentes." << std::endl;
        exit(1);
    }
    return font;
}

int main() {
    // Crear el cliente WebSocket
    WebSocketClient wsClient("ws://127.0.0.1:8080");
    // Crear la ventana de la aplicación
    sf::RenderWindow window(sf::VideoMode(900, 600), "💬 Chat Cliente", sf::Style::Titlebar | sf::Style::Close);
    sf::Font font = loadFont();

    // Cuadro de chat
    sf::RectangleShape chatBox(sf::Vector2f(600, 400));
    chatBox.setPosition(20, 50);
    chatBox.setFillColor(sf::Color(45, 45, 45));
    chatBox.setOutlineThickness(2);
    chatBox.setOutlineColor(sf::Color(80, 80, 80));

    // Cuadro de usuarios conectados
    sf::RectangleShape usersBox(sf::Vector2f(250, 400));
    usersBox.setPosition(630, 50);
    usersBox.setFillColor(sf::Color(35, 35, 35));
    usersBox.setOutlineThickness(2);
    usersBox.setOutlineColor(sf::Color(80, 80, 80));

    // Título de usuarios conectados
    sf::Text usersTitle("👥 Usuarios Conectados", font, 22);
    usersTitle.setPosition(640, 20);
    usersTitle.setFillColor(sf::Color(0, 180, 255));

    // Cuadro de entrada de texto
    sf::RectangleShape inputBox(sf::Vector2f(700, 50));
    inputBox.setPosition(20, 500);
    inputBox.setFillColor(sf::Color(60, 60, 60));
    inputBox.setOutlineThickness(2);
    inputBox.setOutlineColor(sf::Color(100, 100, 100));

    // Texto ingresado por el usuario
    sf::Text inputText("", font, 24);
    inputText.setPosition(30, 515);
    inputText.setFillColor(sf::Color::White);

    // Mensajes en el chat
    sf::Text chatMessages("", font, 18);
    chatMessages.setPosition(30, 60);
    chatMessages.setFillColor(sf::Color::White);

    // Lista de usuarios conectados
    sf::Text userListText("", font, 18);
    userListText.setPosition(640, 60);
    userListText.setFillColor(sf::Color::Green);

    // Botón de enviar mensaje
    sf::RectangleShape sendButton(sf::Vector2f(160, 50));
    sendButton.setPosition(730, 500);
    sendButton.setFillColor(sf::Color(0, 180, 255));
    sendButton.setOutlineThickness(2);
    sendButton.setOutlineColor(sf::Color(200, 200, 200));

    sf::Text sendButtonText("Enviar", font, 22);
    sendButtonText.setPosition(770, 515);
    sendButtonText.setFillColor(sf::Color::White);

    // Botón de salida
    sf::RectangleShape exitButton(sf::Vector2f(40, 30));
    exitButton.setPosition(10, 10);
    exitButton.setFillColor(sf::Color(150, 50, 50));
    exitButton.setOutlineThickness(1);
    exitButton.setOutlineColor(sf::Color(180, 180, 180));

    sf::Text exitButtonText("X", font, 18);
    exitButtonText.setPosition(20, 15);
    exitButtonText.setFillColor(sf::Color::White);

    std::string inputString;

    // Bucle principal de la aplicación
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            // Cerrar la ventana si se presiona la X
            if (event.type == sf::Event::Closed)
                window.close();
            // Capturar la entrada del usuario
            else if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode == 13) { // Presionar Enter para enviar mensaje
                    if (!inputString.empty()) {
                        wsClient.sendMessage(inputString);
                        inputString.clear();
                    }
                } else if (event.text.unicode == 8) { // Borrar con Backspace
                    if (!inputString.empty()) {
                        inputString.pop_back();
                    }
                } else {
                    inputString += static_cast<char>(event.text.unicode);
                }
            } else if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    // Enviar mensaje si se hace clic en el botón de enviar
                    if (sendButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                        if (!inputString.empty()) {
                            wsClient.sendMessage(inputString);
                            inputString.clear();
                        }
                    }
                    // Cerrar la ventana si se hace clic en el botón de salida
                    if (exitButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                        window.close();
                    }
                }
            }
        }

        // Actualizar los mensajes del chat
        std::vector<std::string> messages = wsClient.getMessages();
        std::string chatHistory;
        for (const auto& msg : messages) {
            chatHistory += "📝 " + msg + "\n";
        }
        chatMessages.setString(chatHistory);
        inputText.setString(inputString);

        // Actualizar la lista de usuarios
        std::vector<std::string> users = wsClient.getUsers();
        std::string userList;
        for (const auto& user : users) {
            userList += "🟢 " + user + "\n";
        }
        userListText.setString(userList);

        // Dibujar la ventana y sus elementos
        window.clear(sf::Color(30, 30, 30));
        window.draw(chatBox);
        window.draw(usersBox);
        window.draw(usersTitle);
        window.draw(inputBox);
        window.draw(chatMessages);
        window.draw(inputText);
        window.draw(userListText);
        window.draw(sendButton);
        window.draw(sendButtonText);
        window.draw(exitButton);
        window.draw(exitButtonText);
        window.display();
    }
    return 0;
}
