#ifndef WEBSOCKET_CLIENT_HPP
#define WEBSOCKET_CLIENT_HPP

// Librerías necesarias para el cliente WebSocket
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <mutex>  // Para evitar condiciones de carrera
#include <thread> // Para manejar el cliente en un hilo separado
#include <memory> // Para manejar punteros inteligentes

// Definición del tipo de cliente WebSocket basado en ASIO
typedef websocketpp::client<websocketpp::config::asio_client> client;

// Definición de la clase WebSocketClient
class WebSocketClient {
public:
    // Constructor que recibe la URI del servidor WebSocket
    WebSocketClient(const std::string& uri);

    // Método para enviar un mensaje al servidor WebSocket
    void sendMessage(const std::string& message);

    // Método para obtener los mensajes recibidos desde el servidor
    std::vector<std::string> getMessages();

    // Método para obtener la lista de usuarios conectados
    std::vector<std::string> getUsers();

private:
    // Método que maneja los mensajes entrantes del servidor WebSocket
    void onMessage(websocketpp::connection_hdl hdl, client::message_ptr msg);

    // Método que establece la conexión con el servidor WebSocket
    void connectWebSocket(const std::string& uri);

    client ws_client; // Instancia del cliente WebSocket
    std::shared_ptr<client::connection_type> connection; // Conexión WebSocket con el servidor

    std::vector<std::string> messages; // Vector para almacenar los mensajes del chat
    std::vector<std::string> users; // Vector para almacenar la lista de usuarios conectados

    std::mutex mutex; // Mutex para evitar condiciones de carrera en los datos compartidos
    std::thread client_thread; // Hilo para manejar la ejecución del cliente WebSocket
};

#endif // WEBSOCKET_CLIENT_HPP
