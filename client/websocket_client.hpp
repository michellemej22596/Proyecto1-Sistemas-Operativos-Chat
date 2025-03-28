#ifndef WEBSOCKET_CLIENT_HPP
#define WEBSOCKET_CLIENT_HPP

#include <SFML/Network.hpp>
#include <vector>
#include <string>

class WebSocketClient {
public:
    WebSocketClient(const std::string& url);
    
    void sendUserName(const std::string& username);
    void sendMessage(const std::string& message);
    void updateStatus(const std::string& status);
    
    std::vector<std::string> getMessages();
    std::vector<std::string> getUsers();
    bool isConnected();

private:
    sf::TcpSocket socket;   
    std::vector<std::string> messages;
    std::vector<std::string> users;
};

#endif // WEBSOCKET_CLIENT_HPP