#include <iostream>
#include <winsock2.h>
#include <pthread.h>
#include <cstring>
#include <conio.h>

//linkowanie winsocketów
#pragma comment(lib, "ws2_32.lib")

#define BUFFER_SIZE 1024
int PORT;
#include <atomic>
std::atomic<bool> client_running(true); //client running flag (atomic for thread-safety)
std::atomic<bool> quitting(false); //quitting flag (atomic for thread-safety)

// message receiving
void* receive_messages(void* arg) {
    SOCKET sock = *(SOCKET*)arg;
    char buffer[BUFFER_SIZE];

    while (client_running) {
        //zero buffer
        memset(buffer, 0, BUFFER_SIZE);
        //receive message from socket
        int bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);

        //handling lost connection
        if (bytes_received <= 0) {

            if(quitting)
            {
                //quitting
                break;
            }

            //not quitting (error) (so quitting involuntarly)
            std::cerr << "Connection lost or closed.\n";
            std::cout<< "Press enter to continue...\n";
            getch();
            client_running=false;
            break;
        }
        //adding null terminator
        buffer[bytes_received] = '\0';
        std::cout << "[Received] Message from server: " << buffer << std::endl;
    }

    return nullptr;
}

// sending message
void* send_messages(void* arg) {
    SOCKET sock = *(SOCKET*)arg;
    std::string message;
    while (client_running) {
        std::cout << "Enter message: ";
        std::getline(std::cin, message);


        //handling /quit command
        if (message == "/quit") {
            client_running = false;
            quitting=true;
            std::cout << "Disconnecting..." << std::endl;
            //shuting down socket to stop recv in receive_message
            shutdown(sock, SD_BOTH);
            std::cout<< "Press enter to continue...\n";
            getch();

            break;
        }

        //sending message
        send(sock, message.c_str(), message.size(), 0);
    }
    return nullptr;
}





int main() {

    //choosing port
    std::cout << "Enter port number: ";
    std::cin >> PORT;
    std::cin.ignore();
    if (PORT < 1 || PORT > 65535) {
        std::cerr << "Invalid port number. Use 1-65535." << std::endl;
        return 1;
    }



    //winsocket init
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
        std::cout<< "Press enter to continue...\n";
        getch();
        return 1;
    }

    // client socket
    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        std::cerr << "Error creating socket.\n";
        std::cout<< "Press enter to continue...\n";
        getch();
        WSACleanup();
        return 1;
    }

    //address config
    sockaddr_in server_addr{};//address
    server_addr.sin_family = AF_INET;//ipv4
    server_addr.sin_port = htons(PORT);//port number
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  //localhost

    // connecting to server
    if (connect(client_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Error connecting to server.\n";
        std::cout<< "Press enter to continue...\n";
        getch();
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to server.\n";



    // thread for receiving and thread for sending
    pthread_t receive_thread, send_thread;

    // creating threads
    pthread_create(&receive_thread, nullptr, receive_messages, &client_socket);
    pthread_create(&send_thread, nullptr, send_messages, &client_socket);

    // attaching threads to main thread (so join waits for the threads to stop)
    pthread_join(receive_thread, nullptr);
    pthread_join(send_thread, nullptr);

    closesocket(client_socket);
    WSACleanup();
    return 0;
}
