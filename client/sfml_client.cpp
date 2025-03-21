#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Network.hpp>
#include <iostream>

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Cliente de Chat");

    // Conexión WebSocket
    sf::TcpSocket socket;
    socket.connect("localhost", 8080);  

    // Mensaje de entrada
    sf::String inputText;
    sf::Text inputTextDisplay;
    inputTextDisplay.setFont(sf::Font::getDefaultFont());  
    inputTextDisplay.setCharacterSize(24);
    
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode == 13) {  // Enter key to send message
                    // Enviar el mensaje al servidor
                    socket.send(inputText.toAnsiString().c_str(), inputText.getSize() + 1);
                    inputText = "";  // Limpiar el campo de entrada
                } else {
                    inputText += event.text.unicode;  // Capturar lo que el usuario escribe
                }
            }
        }

        window.clear();
        inputTextDisplay.setString(inputText);
        window.draw(inputTextDisplay);
        window.display();
    }

    return 0;
}
