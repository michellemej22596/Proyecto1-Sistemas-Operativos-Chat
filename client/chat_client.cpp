#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include "websocket_client.hpp"

// Función para cargar la fuente del texto
sf::Font loadFont() {
    sf::Font font;
    if (!font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
        std::cerr << "⚠ No se pudo cargar la fuente. Verifica la instalación de fuentes." << std::endl;
        exit(1);
    }
    return font;
}

int main() {
    WebSocketClient wsClient("ws://127.0.0.1:9000");
    sf::RenderWindow window(sf::VideoMode(900, 600), "💬 Chat Cliente", sf::Style::Titlebar | sf::Style::Close);
    sf::Font font = loadFont();

    // Crear los elementos gráficos de la interfaz
    sf::RectangleShape chatBox(sf::Vector2f(600, 400));
    chatBox.setPosition(20, 50);
    chatBox.setFillColor(sf::Color(45, 45, 45));
    chatBox.setOutlineThickness(2);
    chatBox.setOutlineColor(sf::Color(80, 80, 80));

    sf::RectangleShape usersBox(sf::Vector2f(250, 400));
    usersBox.setPosition(630, 50);
    usersBox.setFillColor(sf::Color(35, 35, 35));
    usersBox.setOutlineThickness(2);
    usersBox.setOutlineColor(sf::Color(80, 80, 80));

    sf::Text usersTitle("👥 Usuarios Conectados", font, 22);
    usersTitle.setPosition(640, 20);
    usersTitle.setFillColor(sf::Color(0, 180, 255));

    sf::RectangleShape inputBox(sf::Vector2f(700, 50));
    inputBox.setPosition(20, 500);
    inputBox.setFillColor(sf::Color(60, 60, 60));
    inputBox.setOutlineThickness(2);
    inputBox.setOutlineColor(sf::Color(100, 100, 100));

    sf::Text inputText("", font, 24);
    inputText.setPosition(30, 515);
    inputText.setFillColor(sf::Color::White);

    sf::Text chatMessages("", font, 18);
    chatMessages.setPosition(30, 60);
    chatMessages.setFillColor(sf::Color::White);

    sf::Text userListText("", font, 18);
    userListText.setPosition(640, 60);
    userListText.setFillColor(sf::Color::Green);

    sf::RectangleShape sendButton(sf::Vector2f(160, 50));
    sendButton.setPosition(730, 500);
    sendButton.setFillColor(sf::Color(0, 180, 255));
    sendButton.setOutlineThickness(2);
    sendButton.setOutlineColor(sf::Color(200, 200, 200));

    sf::Text sendButtonText("Enviar", font, 22);
    sendButtonText.setPosition(770, 515);
    sendButtonText.setFillColor(sf::Color::White);

    sf::RectangleShape exitButton(sf::Vector2f(40, 30));
    exitButton.setPosition(10, 10);
    exitButton.setFillColor(sf::Color(150, 50, 50));
    exitButton.setOutlineThickness(1);
    exitButton.setOutlineColor(sf::Color(180, 180, 180));

    sf::Text exitButtonText("X", font, 18);
    exitButtonText.setPosition(20, 15);
    exitButtonText.setFillColor(sf::Color::White);

    // Botones de estado
    sf::RectangleShape statusActivo(sf::Vector2f(100, 30));
    statusActivo.setPosition(20, 560);
    statusActivo.setFillColor(sf::Color(0, 180, 0));
    sf::Text labelActivo("ACTIVO", font, 16);
    labelActivo.setPosition(35, 565);
    labelActivo.setFillColor(sf::Color::White);

    sf::RectangleShape statusOcupado(sf::Vector2f(100, 30));
    statusOcupado.setPosition(130, 560);
    statusOcupado.setFillColor(sf::Color(200, 140, 0));
    sf::Text labelOcupado("OCUPADO", font, 16);
    labelOcupado.setPosition(140, 565);
    labelOcupado.setFillColor(sf::Color::White);

    sf::RectangleShape statusInactivo(sf::Vector2f(100, 30));
    statusInactivo.setPosition(240, 560);
    statusInactivo.setFillColor(sf::Color(100, 100, 100));
    sf::Text labelInactivo("INACTIVO", font, 16);
    labelInactivo.setPosition(245, 565);
    labelInactivo.setFillColor(sf::Color::White);

    std::string estadoActual = "ACTIVO";
    sf::Text currentStatus("Estado: ACTIVO", font, 16);
    currentStatus.setPosition(360, 565);
    currentStatus.setFillColor(sf::Color::Cyan);

    std::string inputString;

    // Pedir nombre de usuario
    std::string username;
    std::cout << "Introduce tu nombre de usuario: ";
    std::cin >> username;
    wsClient.sendUserName(username); // Enviar nombre de usuario al servidor

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode == 13) {
                    if (!inputString.empty()) {
                        wsClient.sendMessage(inputString); // Enviar mensaje al servidor
                        inputString.clear();
                    }
                } else if (event.text.unicode == 8) {
                    if (!inputString.empty()) {
                        inputString.pop_back(); // Eliminar el último carácter al presionar "backspace"
                    }
                } else {
                    inputString += static_cast<char>(event.text.unicode);
                }
            } else if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    if (sendButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                        if (!inputString.empty()) {
                            wsClient.sendMessage(inputString); // Enviar mensaje al servidor
                            inputString.clear();
                        }
                    }
                    if (exitButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                        window.close();
                    }

                    // Estado: ACTIVO
                    if (statusActivo.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                        estadoActual = "ACTIVO";
                        currentStatus.setString("Estado: ACTIVO");
                        wsClient.updateStatus(estadoActual); // Enviar estado al servidor
                    }
                    // Estado: OCUPADO
                    if (statusOcupado.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                        estadoActual = "OCUPADO";
                        currentStatus.setString("Estado: OCUPADO");
                        wsClient.updateStatus(estadoActual); // Enviar estado al servidor
                    }
                    // Estado: INACTIVO
                    if (statusInactivo.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                        estadoActual = "INACTIVO";
                        currentStatus.setString("Estado: INACTIVO");
                        wsClient.updateStatus(estadoActual); // Enviar estado al servidor
                    }
                }
            }
        }

        // Obtener mensajes del servidor y mostrarlos
        std::vector<std::string> messages = wsClient.getMessages();
        std::string chatHistory;
        for (const auto& msg : messages) {
            chatHistory += "📝 " + msg + "\n";
        }

        // Actualizar la interfaz gráfica con los mensajes
        chatMessages.setString(chatHistory);
        inputText.setString(inputString);

        // Obtener usuarios conectados
        std::vector<std::string> users = wsClient.getUsers();
        std::string userList;
        for (const auto& user : users) {
            userList += "🟢 " + user + "\n";
        }
        userListText.setString(userList);

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

        // Dibujar botones de estado
        window.draw(statusActivo);
        window.draw(labelActivo);
        window.draw(statusOcupado);
        window.draw(labelOcupado);
        window.draw(statusInactivo);
        window.draw(labelInactivo);
        window.draw(currentStatus);

        window.display();
    }

    return 0;
}