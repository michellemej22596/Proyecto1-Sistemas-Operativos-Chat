#include "websocket_client.hpp"
#include <iostream>
#include <sstream>
#include <random>
#include <openssl/sha.h>
#include <iomanip>
#include <algorithm>
#include <cstring>

WebSocketClient::WebSocketClient(const std::string& url) {
    running = false;

    // Extraer nombre de usuario de la URL
    std::size_t namePos = url.find("?name=");
    if (namePos != std::string::npos) {
        username = url.substr(namePos + 6);
    }

    // Conexión
    std::string address = "127.0.0.1";
    unsigned short port = 8080;

    if (socket.connect(address, port) != sf::Socket::Done) {
        std::cerr << "❌ Error al conectar con el servidor." << std::endl;
        return;
    }

    if (!performHandshake(url)) {
        std::cerr << "❌ Falló el handshake WebSocket." << std::endl;
        socket.disconnect();
        return;
    }

    running = true;
    receiverThread = std::thread(&WebSocketClient::receiveLoop, this);
}

WebSocketClient::~WebSocketClient() {
    running = false;
    socket.disconnect();
    if (receiverThread.joinable())
        receiverThread.join();
}

bool WebSocketClient::performHandshake(const std::string& url) {
    // Generar clave aleatoria base64
    std::string secWebSocketKey = "x3JJHMbDL1EzLkh9GBhXDw=="; // simplificada

    std::ostringstream request;
    request << "GET " << url.substr(url.find("/")) << " HTTP/1.1\r\n"
            << "Host: localhost\r\n"
            << "Upgrade: websocket\r\n"
            << "Connection: Upgrade\r\n"
            << "Sec-WebSocket-Key: " << secWebSocketKey << "\r\n"
            << "Sec-WebSocket-Version: 13\r\n\r\n";

    std::string reqStr = request.str();
    if (socket.send(reqStr.c_str(), reqStr.size()) != sf::Socket::Done)
        return false;

    char buffer[2048];
    std::size_t received = 0;
    if (socket.receive(buffer, sizeof(buffer), received) != sf::Socket::Done)
        return false;

    std::string response(buffer, received);
    return response.find("101 Switching Protocols") != std::string::npos;
}

void WebSocketClient::sendMessage(const std::string& message) {
    std::vector<char> frame;
    frame.push_back(0x81); // FIN = 1, Text frame

    size_t len = message.size();
    if (len <= 125) {
        frame.push_back(static_cast<char>(len | 0x80)); // con máscara
    } else if (len <= 65535) {
        frame.push_back(126 | 0x80);
        frame.push_back((len >> 8) & 0xFF);
        frame.push_back(len & 0xFF);
    } else {
        frame.push_back(127 | 0x80);
        for (int i = 7; i >= 0; --i)
            frame.push_back((len >> (8 * i)) & 0xFF);
    }

    // Clave de máscara
    char mask[4] = {0x12, 0x34, 0x56, 0x78};
    frame.insert(frame.end(), mask, mask + 4);

    for (size_t i = 0; i < len; ++i) {
        frame.push_back(message[i] ^ mask[i % 4]);
    }

    socket.send(frame.data(), frame.size());
}

void WebSocketClient::updateStatus(const std::string& status) {
    if (status == "ACTIVO") sendMessage(std::string(1, 2) + "1");
    else if (status == "OCUPADO") sendMessage(std::string(1, 2) + "2");
    else if (status == "INACTIVO") sendMessage(std::string(1, 2) + "3");
}

std::vector<std::string> WebSocketClient::getMessages() {
    std::lock_guard<std::mutex> lock(dataMutex);
    return messages;
}

std::vector<std::string> WebSocketClient::getUsers() {
    std::lock_guard<std::mutex> lock(dataMutex);
    return users;
}

bool WebSocketClient::isConnected() const {
    return running;
}

void WebSocketClient::receiveLoop() {
    while (running) {
        char hdr[2];
        std::size_t received;

        if (socket.receive(hdr, 2, received) != sf::Socket::Done || received < 2)
            continue;

        bool fin = hdr[0] & 0x80;
        uint8_t opcode = hdr[0] & 0x0F;
        uint8_t len = hdr[1] & 0x7F;
        bool masked = hdr[1] & 0x80;

        uint64_t payloadLen = len;
        if (len == 126) {
            char ext[2];
            if (socket.receive(ext, 2, received) != sf::Socket::Done) continue;
            payloadLen = (uint8_t(ext[0]) << 8) | uint8_t(ext[1]);
        } else if (len == 127) {
            char ext[8];
            if (socket.receive(ext, 8, received) != sf::Socket::Done) continue;
            payloadLen = 0;
            for (int i = 0; i < 8; ++i) {
                payloadLen = (payloadLen << 8) | static_cast<unsigned char>(ext[i]);
            }
        }

        char maskKey[4] = {};
        if (masked) {
            if (socket.receive(maskKey, 4, received) != sf::Socket::Done) continue;
        }

        std::vector<char> payload(payloadLen);
        size_t offset = 0;
        while (offset < payloadLen) {
            if (socket.receive(&payload[offset], payloadLen - offset, received) != sf::Socket::Done)
                break;
            offset += received;
        }

        if (masked) {
            for (uint64_t i = 0; i < payloadLen; ++i) {
                payload[i] ^= maskKey[i % 4];
            }
        }

        parseFrame(payload);
    }
}

void WebSocketClient::parseFrame(const std::vector<char>& frameData) {
    if (frameData.empty()) return;

    uint8_t code = static_cast<uint8_t>(frameData[0]);
    std::lock_guard<std::mutex> lock(dataMutex);

    if (code == 50) { // error
        std::string msg(frameData.begin() + 2, frameData.end());
        messages.push_back("⚠️ Error: " + msg);
    } else if (code == 51) { // lista de usuarios
        users.clear();
        uint8_t count = frameData[1];
        size_t idx = 2;
        for (int i = 0; i < count && idx < frameData.size(); ++i) {
            uint8_t len = frameData[idx++];
            std::string name(frameData.begin() + idx, frameData.begin() + idx + len);
            users.push_back(name);
            idx += len + 1; // nombre + status
        }
    } else if (code == 55) { // mensaje privado
        uint8_t nameLen = frameData[1];
        std::string sender(frameData.begin() + 2, frameData.begin() + 2 + nameLen);
        uint8_t msgLen = frameData[2 + nameLen];
        std::string msg(frameData.begin() + 3 + nameLen, frameData.begin() + 3 + nameLen + msgLen);
        messages.push_back(sender + ": " + msg);
    } else if (code == 56) { // historial
        uint8_t count = frameData[1];
        size_t idx = 2;
        for (int i = 0; i < count && idx < frameData.size(); ++i) {
            uint8_t len = frameData[idx++];
            std::string msg(frameData.begin() + idx, frameData.begin() + idx + len);
            messages.push_back("📜 " + msg);
            idx += len;
        }
    } else {
        messages.push_back("📩 Mensaje recibido (código desconocido): " + std::to_string(code));
    }
}
