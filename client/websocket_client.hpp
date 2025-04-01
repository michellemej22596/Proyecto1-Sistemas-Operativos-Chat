#ifndef WEBSOCKET_CLIENT_HPP
#define WEBSOCKET_CLIENT_HPP

#include <SFML/Network.hpp>
#include <vector>
#include <string>
#include <thread>
#include <mutex>

class WebSocketClient {
public:
    WebSocketClient(const std::string& url);
    ~WebSocketClient();

    void sendUserName(const std::string& username);
    void sendMessage(const std::string& message);
    void updateStatus(const std::string& status);
    std::vector<std::string> getMessages();
    std::vector<std::string> getUsers();
    bool isConnected() const;

    void close();  // Método para cerrar conexión

private:
    sf::TcpSocket socket;
    std::vector<std::string> messages;
    std::vector<std::string> users;
    std::string username;
    bool running;

    std::thread receiverThread;
    std::mutex dataMutex;

    bool performHandshake(const std::string& url);
    void receiveLoop();
    void parseFrame(const std::vector<char>&);
    void sendBinaryFrame(const std::vector<char>& data);  // ✅ Añadido
};

#endif
