#ifndef WEBSOCKET_CLIENT_HPP
#define WEBSOCKET_CLIENT_HPP

#include <SFML/Network.hpp>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>

class WebSocketClient {
public:
    WebSocketClient(const std::string& url);
    ~WebSocketClient();

    void sendMessage(const std::string& message);
    void updateStatus(const std::string& status);
    
    std::vector<std::string> getMessages();
    std::vector<std::string> getUsers();

    bool isConnected() const;

private:
    void receiveLoop();
    bool performHandshake(const std::string& url);
    void parseFrame(const std::vector<char>& frameData);

    sf::TcpSocket socket;
    std::thread receiverThread;
    std::atomic<bool> running;
    
    std::vector<std::string> messages;
    std::vector<std::string> users;
    std::mutex dataMutex;

    std::string username;
};

#endif // WEBSOCKET_CLIENT_HPP
