#include <iostream>
#include <winsock2.h>
#include <cstring>
#include <thread>

//linkowanie winsocketów
#pragma comment(lib, "ws2_32.lib")

#define BUFFER_SIZE 1024
int PORT;

// odbieranie wiadomości
void receive_messages(SOCKET sock) {
    char buffer[BUFFER_SIZE];

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0) {
            std::cerr << "Connection lost or closed.\n";
            break;
        }
        buffer[bytes_received] = '\0';
        std::cout << "[Received] Message from server: " << buffer << std::endl;
    }

    //closesocket(sock);
}

// wysyłanie wiadomości
void send_messages(SOCKET sock) {
    char message[BUFFER_SIZE];
    while (true) {
        std::cout << "Enter message: ";
        std::cin.getline(message, BUFFER_SIZE);

        send(sock, message, strlen(message), 0);
    }

    //closesocket(sock);
}

int main() {

    std::cout << "Enter port number: ";
    std::cin >> PORT;
    std::cin.ignore();
    if (PORT < 1 || PORT > 65535) {
        std::cerr << "Invalid port number. Use 1-65535." << std::endl;
        return 1;
    }

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
        return 1;
    }

    // gniazdo klienta
    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        std::cerr << "Error creating socket.\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  //localhost

    // łączenie z serwerem
    if (connect(client_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Error connecting to server.\n";
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to server.\n";

    // wątek do odbierania i wątek do wysyłania
    std::thread receive_thread(receive_messages, client_socket);
    std::thread send_thread(send_messages, client_socket);

    // oczekiwanie na zakończenie wątków
    receive_thread.join();
    send_thread.join();

    closesocket(client_socket);
    WSACleanup();
    return 0;
}
