#include "websocket_client.hpp"

// Constructor de la clase WebSocketClient
WebSocketClient::WebSocketClient(const std::string& uri) {
    // Desactiva los logs de acceso para evitar mensajes innecesarios
    ws_client.set_access_channels(websocketpp::log::alevel::none);

    // Inicializa el cliente WebSocket con ASIO (biblioteca de redes)
    ws_client.init_asio();

    // Define la función que manejará los mensajes entrantes
    ws_client.set_message_handler(
        websocketpp::lib::bind(&WebSocketClient::onMessage, this,
                               websocketpp::lib::placeholders::_1,
                               websocketpp::lib::placeholders::_2));

    // Llama a la función para conectar con el servidor WebSocket
    connectWebSocket(uri);
}

// Función para conectar con el servidor WebSocket
void WebSocketClient::connectWebSocket(const std::string& uri) {
    websocketpp::lib::error_code ec;

    // Obtiene una conexión al servidor WebSocket
    connection = ws_client.get_connection(uri, ec);

    // Verifica si hubo un error al intentar conectarse
    if (ec) {
        std::cerr << "❌ Error al conectar: " << ec.message() << std::endl;
        return;
    }

    // Establece la conexión
    ws_client.connect(connection);

    // Inicia el cliente WebSocket en un hilo separado para evitar bloqueos
    client_thread = std::thread([this]() { ws_client.run(); });
}

// Función que maneja los mensajes recibidos del servidor
void WebSocketClient::onMessage(websocketpp::connection_hdl hdl, client::message_ptr msg) {
    std::lock_guard<std::mutex> lock(mutex); // Bloqueo para evitar condiciones de carrera
    std::string payload = msg->get_payload(); // Obtiene el contenido del mensaje

    // Verifica si el mensaje es una lista de usuarios conectados
    if (payload.rfind("[USERS] ", 0) == 0) {
        users.clear(); // Limpia la lista de usuarios anteriores
        std::string userList = payload.substr(8); // Elimina el prefijo "[USERS] "
        size_t pos = 0;

        // Divide la cadena de usuarios y los almacena en la lista
        while ((pos = userList.find(',')) != std::string::npos) {
            users.push_back(userList.substr(0, pos));
            userList.erase(0, pos + 1);
        }
        users.push_back(userList); // Agrega el último usuario de la lista
    } else {
        // Agrega los mensajes normales al historial de chat
        messages.push_back("📩 " + payload);
    }
}

// Función para enviar un mensaje al servidor WebSocket
void WebSocketClient::sendMessage(const std::string& message) {
    if (connection) { // Verifica si la conexión está establecida
        connection->send(message);
    }
}

// Función para obtener los mensajes almacenados en el historial del chat
std::vector<std::string> WebSocketClient::getMessages() {
    std::lock_guard<std::mutex> lock(mutex); // Bloqueo para evitar condiciones de carrera
    return messages;
}

// Función para obtener la lista de usuarios conectados
std::vector<std::string> WebSocketClient::getUsers() {
    std::lock_guard<std::mutex> lock(mutex); // Bloqueo para evitar condiciones de carrera
    return users;
}
