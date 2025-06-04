#include <iostream>
#include <pthread.h>
#include <vector>
#include <algorithm>
#include <cstring>
#include <winsock2.h>
//linkowanie winscoketów
#pragma comment(lib, "ws2_32.lib")

#define BUFFER_SIZE 1024

int PORT;
//clients' sockets vector
std::vector<SOCKET> clients;
//mutex for clients
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
//mutex for stdout
pthread_mutex_t cout_mutex = PTHREAD_MUTEX_INITIALIZER;




#include <atomic>
std::atomic<bool> server_running(true); //server running flag (atomic for thread-safety)

// broadcast
void broadcast(const char* message, SOCKET sender) {
    //lock mutex
    pthread_mutex_lock(&clients_mutex);

    //sending message to every socket except sender
    for (SOCKET client : clients) {
        if (client != sender) {
            send(client, message, strlen(message), 0);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    //unlock mutex
}

// client handling
//used void pointer, because create_pthread requires void*
void* handle_client(void* arg) {

    //void->socket
    SOCKET client_socket = *(SOCKET*)arg;
    delete (SOCKET*)arg;

    // blocking mode on client socket
    u_long mode = 0;
    ioctlsocket(client_socket, FIONBIO, &mode);



    char buffer[BUFFER_SIZE];


    while (server_running) {
        //clearing buffer
        memset(buffer, 0, BUFFER_SIZE);
        //recieveing data from socket
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received == 0) {
            pthread_mutex_lock(&cout_mutex);
            std::cout << "Client disconnected cleanly.\n";
            pthread_mutex_unlock(&cout_mutex);
            break;
        }
        if (bytes_received < 0) {

            int err = WSAGetLastError();
            //blocking error
            if (err == WSAEWOULDBLOCK) {
                Sleep(50); //time between next recv() attempt
                continue;
            }
            else if (err==0)
            {
                pthread_mutex_lock(&cout_mutex);
                std::cout << "Client disconnected cleanly.\n";
                pthread_mutex_unlock(&cout_mutex);
                break;
            }
            else if (err==10053||err==10038)
            {
                //client kicked
                break;
            }
            else if(err==10054)
            {
                pthread_mutex_lock(&cout_mutex);
                std::cout << "Client disconnected.\n";
                pthread_mutex_unlock(&cout_mutex);
                break;
            }
            else {
                //other errors
                pthread_mutex_lock(&cout_mutex);
                std::cerr << "recv() error: " << err << "\n";
                pthread_mutex_unlock(&cout_mutex);
                break;
            }


        }
        //end of message footer
        buffer[bytes_received] = '\0';
        //print broadcast message
        pthread_mutex_lock(&cout_mutex);
        std::cout << client_socket <<": "<< buffer << std::endl;
        pthread_mutex_unlock(&cout_mutex);

        //broadcast message
        broadcast(buffer, client_socket);
    }

    // deleteing client
    //mutex lock
    pthread_mutex_lock(&clients_mutex);

    clients.erase(std::remove(clients.begin(), clients.end(), client_socket), clients.end());

    //mutex unlock
    pthread_mutex_unlock(&clients_mutex);

    closesocket(client_socket);

    return nullptr;
}


//listnening for console input
void* console_listener(void*) {
    std::string input;
    while (true) {
        std::getline(std::cin, input);
        if (input == "/quit") {
            server_running = false;
            pthread_mutex_lock(&cout_mutex);
            std::cout << "Shutting down server..." << std::endl;
            pthread_mutex_unlock(&cout_mutex);

            // closing all clients (recv() would still block shutting down the server)
            pthread_mutex_lock(&clients_mutex);
            for (SOCKET client : clients) {
                closesocket(client);
            }
            clients.clear();
            pthread_mutex_unlock(&clients_mutex);

            break;
        }

        // /kick command
        if (input.rfind("/kick ", 0) == 0) { // check if starts with "/kick "
            std::string arg = input.substr(6);
            SOCKET sock_to_kick = static_cast<SOCKET>(std::stoi(arg));

            pthread_mutex_lock(&clients_mutex);
            auto it = std::find(clients.begin(), clients.end(), sock_to_kick);
            if (it != clients.end()) {
                closesocket(sock_to_kick);
                clients.erase(it);
                std::cout << "Kicked client socket: " << sock_to_kick << std::endl;
            } else {
                std::cout << "Client socket " << sock_to_kick << " not found.\n";
            }
            pthread_mutex_unlock(&clients_mutex);
        }


    }



    return nullptr;
}


int main() {

    //enter the port number
    std::cout << "Enter port number: ";
    std::cin >> PORT;

    if (PORT < 1 || PORT > 65535) {
        std::cerr << "Invalid port number. Use 1-65535." << std::endl;
        return 1;
    }


    //console handling thread
    pthread_t console_thread;
    pthread_create(&console_thread, nullptr, console_listener, nullptr);
    pthread_detach(console_thread);


    //winsock init
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    //server socket
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);

    u_long mode = 1;
    ioctlsocket(server_socket, FIONBIO, &mode); // non blocking mode




    //address config
    sockaddr_in server_addr{}; //address init
    server_addr.sin_family = AF_INET; //ipv4
    server_addr.sin_addr.s_addr = INADDR_ANY;//listening on all interfaces
    server_addr.sin_port = htons(PORT); //configure port number

    //binding socket with IP address
    bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr));
    //start listening
    listen(server_socket, SOMAXCONN);


    std::cout << "Server is working on port " << PORT << std::endl;


    //clients accepting
    while (server_running) {
        sockaddr_in client_addr{};//adres klienta
        int client_size = sizeof(client_addr);
        //accept returns socket descriptor (instantly, if no clients -> INVALID_SOCKET)
        SOCKET client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_size);

        if (client_socket == INVALID_SOCKET) {
            //"error" (no connections to accept)
            if (WSAGetLastError() == WSAEWOULDBLOCK) {
                Sleep(100);
                continue;
            } else {
                //other errors
                pthread_mutex_lock(&cout_mutex);
                std::cerr << "Accept failed: " << WSAGetLastError() << std::endl;
                pthread_mutex_unlock(&cout_mutex);
                break;
            }
        }

        pthread_mutex_lock(&cout_mutex);
        std::cout << "New client: " << inet_ntoa(client_addr.sin_addr) << std::endl;
        pthread_mutex_unlock(&cout_mutex);

        //mutex lock
        pthread_mutex_lock(&clients_mutex);
        //add client to sockets vector
        clients.push_back(client_socket);
        //mutex unlock
        pthread_mutex_unlock(&clients_mutex);

        //new thread
        pthread_t tid;//thread id
        SOCKET* new_sock = new SOCKET(client_socket);//socket pointer (for pthread_create)
        //creating new thread
        pthread_create(&tid, nullptr, handle_client, new_sock);
        //detaching thread (no need to manually clean after it)
        pthread_detach(tid);
    }

    //cleaning
    closesocket(server_socket);
    WSACleanup();
    return 0;
}
