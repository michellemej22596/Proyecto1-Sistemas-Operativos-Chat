#include "websocket_client.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <SFML/Network.hpp>

WebSocketClient::WebSocketClient(const std::string& url) {
    // Aquí deberías desglosar la URL para obtener la dirección y el puerto
    std::string serverAddress = "localhost";  // Esta es una suposición; ajusta según sea necesario
    unsigned short port = 8080;  // Esta es una suposición; ajusta según sea necesario

    sf::IpAddress ipAddress(serverAddress);

    // Intentamos conectar al servidor usando el socket
    if (socket.connect(ipAddress, port) != sf::Socket::Status::Done) {
        std::cerr << "Error al conectar al servidor WebSocket." << std::endl;
    }
}

void WebSocketClient::sendUserName(const std::string& username) {
    std::string message = "USER: " + username;
    if (socket.send(message.c_str(), message.size()) != sf::Socket::Status::Done) {
        std::cerr << "Error al enviar el nombre de usuario." << std::endl;
    }
}

void WebSocketClient::sendMessage(const std::string& message) {
    if (socket.send(message.c_str(), message.size()) != sf::Socket::Status::Done) {
        std::cerr << "Error al enviar el mensaje." << std::endl;
    }
}

void WebSocketClient::updateStatus(const std::string& status) {
    std::string statusMessage = "STATUS: " + status;
    if (socket.send(statusMessage.c_str(), statusMessage.size()) != sf::Socket::Status::Done) {
        std::cerr << "Error al actualizar el estado." << std::endl;
    }
}

std::vector<std::string> WebSocketClient::getMessages() {
    return messages;  // Asegúrate de llenar este vector con los mensajes recibidos
}

std::vector<std::string> WebSocketClient::getUsers() {
    return users;  // Asegúrate de llenar este vector con los usuarios
}

bool WebSocketClient::isConnected() {
    // El estado de la conexión se determina al hacer la conexión con el servidor
    return (socket.connect(sf::IpAddress("localhost"), 8080) == sf::Socket::Status::Done);
}