#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include "websocket_client.hpp"

// Cargar fuente del sistema
sf::Font loadFont() {
    sf::Font font;
    if (!font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
        std::cerr << "⚠ No se pudo cargar la fuente." << std::endl;
        exit(1);
    }
    return font;
}

int main() {
    std::string username;
    std::cout << "Introduce tu nombre de usuario: ";
    std::cin >> username;

    std::string url = "/?name=" + username;
    WebSocketClient wsClient(url);

    if (!wsClient.isConnected()) {
        std::cerr << "❌ No se pudo establecer conexión con el servidor." << std::endl;
        return 1;
    }

    sf::RenderWindow window(sf::VideoMode(900, 600), "💬 Chat Cliente", sf::Style::Titlebar | sf::Style::Close);
    sf::Font font = loadFont();

    // UI elementos
    sf::RectangleShape chatBox({600, 400});
    chatBox.setPosition(20, 50);
    chatBox.setFillColor(sf::Color(45, 45, 45));
    chatBox.setOutlineThickness(2);
    chatBox.setOutlineColor(sf::Color(80, 80, 80));

    sf::RectangleShape usersBox({250, 400});
    usersBox.setPosition(630, 50);
    usersBox.setFillColor(sf::Color(35, 35, 35));
    usersBox.setOutlineThickness(2);
    usersBox.setOutlineColor(sf::Color(80, 80, 80));

    sf::Text usersTitle("👥 Usuarios", font, 22);
    usersTitle.setPosition(640, 20);
    usersTitle.setFillColor(sf::Color(0, 180, 255));

    sf::RectangleShape inputBox({700, 50});
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

    sf::RectangleShape sendButton({160, 50});
    sendButton.setPosition(730, 500);
    sendButton.setFillColor(sf::Color(0, 180, 255));
    sf::Text sendButtonText("Enviar", font, 22);
    sendButtonText.setPosition(770, 515);
    sendButtonText.setFillColor(sf::Color::White);

    // Estado actual y botones
    std::string estadoActual = "ACTIVO";
    sf::Text currentStatus("Estado: ACTIVO", font, 16);
    currentStatus.setPosition(360, 565);
    currentStatus.setFillColor(sf::Color::Cyan);

    sf::RectangleShape statusBtns[3];
    sf::Text statusLabels[3];
    std::string estados[3] = { "ACTIVO", "OCUPADO", "INACTIVO" };
    sf::Color colores[3] = { sf::Color(0, 180, 0), sf::Color(200, 140, 0), sf::Color(100, 100, 100) };

    for (int i = 0; i < 3; ++i) {
        statusBtns[i].setSize({100, 30});
        statusBtns[i].setPosition(20 + i * 110, 560);
        statusBtns[i].setFillColor(colores[i]);

        statusLabels[i].setFont(font);
        statusLabels[i].setString(estados[i]);
        statusLabels[i].setCharacterSize(16);
        statusLabels[i].setPosition(30 + i * 110, 565);
        statusLabels[i].setFillColor(sf::Color::White);
    }

    std::string inputString;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode == 13 && !inputString.empty()) {
                    wsClient.sendMessage(inputString);
                    inputString.clear();
                } else if (event.text.unicode == 8 && !inputString.empty()) {
                    inputString.pop_back();
                } else if (event.text.unicode < 128 && event.text.unicode != 13 && event.text.unicode != 8) {
                    inputString += static_cast<char>(event.text.unicode);
                }
            } else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                if (sendButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                    if (!inputString.empty()) {
                        wsClient.sendMessage(inputString);
                        inputString.clear();
                    }
                }
                for (int i = 0; i < 3; ++i) {
                    if (statusBtns[i].getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                        estadoActual = estados[i];
                        currentStatus.setString("Estado: " + estadoActual);
                        wsClient.updateStatus(estadoActual);
                    }
                }
            }
        }

        std::vector<std::string> messages = wsClient.getMessages();
        std::string chat;
        for (const auto& msg : messages) chat += msg + "\n";
        chatMessages.setString(chat);

        inputText.setString(inputString);

        std::vector<std::string> users = wsClient.getUsers();
        std::string userList;
        for (const auto& user : users) userList += "🟢 " + user + "\n";
        userListText.setString(userList);

        window.clear(sf::Color(30, 30, 30));
        window.draw(chatBox);
        window.draw(chatMessages);
        window.draw(usersBox);
        window.draw(usersTitle);
        window.draw(userListText);
        window.draw(inputBox);
        window.draw(inputText);
        window.draw(sendButton);
        window.draw(sendButtonText);
        for (int i = 0; i < 3; ++i) {
            window.draw(statusBtns[i]);
            window.draw(statusLabels[i]);
        }
        window.draw(currentStatus);
        window.display();
    }

    return 0;
}
