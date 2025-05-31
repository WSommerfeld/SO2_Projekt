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
//wektor socketów klientów
std::vector<SOCKET> clients;
//muteks do listy socketów klientów
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;



#include <atomic>
std::atomic<bool> server_running(true); //flaga działania serwera (jako atomic, żeby było bezpiecznie dla wątków)

// broadcast
void broadcast(const char* message, SOCKET sender) {
    //zablokowanie muteksa
    pthread_mutex_lock(&clients_mutex);
    //wysłanie wiadomości na każdy socket poza nadawcą
    for (SOCKET client : clients) {
        if (client != sender) {
            send(client, message, strlen(message), 0);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    //odblokowanie muteksa
}

// obsługa klienta
//wskaźnik na dowolny typ (dla create_pthread)
void* handle_client(void* arg) {

    //rzutowanie voida na socket
    SOCKET client_socket = *(SOCKET*)arg;
    delete (SOCKET*)arg;

    char buffer[BUFFER_SIZE];

    while (true) {
        //zerowanie bufora
        memset(buffer, 0, BUFFER_SIZE);
        //odbieranie danych z socketa
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        //zakończ jeśli 0 lub -1 (błędy)
        if (bytes_received <= 0) {
            break;
        }
        //ustawienie znacznika końca wiadomości
        buffer[bytes_received] = '\0';
        //wyświetlenie co broadcastujemy
        std::cout << "Message: " << buffer << std::endl;

        //boradcast naszej wiadomości
        broadcast(buffer, client_socket);
    }

    // usuwanie klienta po rozlaczeniu
    //blokada muteksa
    pthread_mutex_lock(&clients_mutex);
    //usuniecie klienta
    clients.erase(std::remove(clients.begin(), clients.end(), client_socket), clients.end());
    //odblokowanie muteksa
    pthread_mutex_unlock(&clients_mutex);

    closesocket(client_socket);
    return nullptr;
}


//nasłuchiwanie poleceń z konsoli
void* console_listener(void*) {
    std::string input;
    while (true) {
        std::getline(std::cin, input);
        if (input == "/quit") {
            server_running = false;
            std::cout << "Shutting down server..." << std::endl;
            break;
        }
    }
    return nullptr;
}


int main() {

    //Podanie numeru portu
    std::cout << "Enter port number: ";
    std::cin >> PORT;
    if (PORT < 1 || PORT > 65535) {
        std::cerr << "Invalid port number. Use 1-65535." << std::endl;
        return 1;
    }


    //wątek do obsługi konsoli
    pthread_t console_thread;
    pthread_create(&console_thread, nullptr, console_listener, nullptr);
    pthread_detach(console_thread);


    //inicjalizacja winsock
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    //gniazdo serwera
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);

    u_long mode = 1;
    ioctlsocket(server_socket, FIONBIO, &mode); // tryb nieblokujący




    //konfiguracja adresu
    sockaddr_in server_addr{}; //inicjalizacja adresu
    server_addr.sin_family = AF_INET; //ipv4
    server_addr.sin_addr.s_addr = INADDR_ANY;//nasłuchiwanie na wszystkich możłiwych interfejsach sieciowych
    server_addr.sin_port = htons(PORT); //ustawienie portu (z zamianą na kolejność sieciową)

    //bindowanie gniazda z adresem IP i portem
    bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr));
    //rozpoczęcie nasłuchiwania
    listen(server_socket, SOMAXCONN);

    std::cout << "Server is working on port " << PORT << std::endl;

    //akceptowanie klientów
    while (server_running) {
        sockaddr_in client_addr{};//adres klienta
        int client_size = sizeof(client_addr);
        //accept zwraca deskryptor gniazda (od razu, w przypadku braku klientów INVALID_SOCKET)
        SOCKET client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_size);

        if (client_socket == INVALID_SOCKET) {
            //"błąd" w postaci braku braku połączenia do zaakceptowania
            if (WSAGetLastError() == WSAEWOULDBLOCK) {
                Sleep(100);
                continue;
            } else {
                //inne błędy
                std::cerr << "Accept failed: " << WSAGetLastError() << std::endl;
                break;
            }
        }

        std::cout << "New client: " << inet_ntoa(client_addr.sin_addr) << std::endl;

        //blokada muteksu
        pthread_mutex_lock(&clients_mutex);
        //dodanie klienta do wektora socketów
        clients.push_back(client_socket);
        //odblokowanie muteksu
        pthread_mutex_unlock(&clients_mutex);

        //nowy wątek
        pthread_t tid;//id wątku
        SOCKET* new_sock = new SOCKET(client_socket);//wskaźnik na socket (abu przekazać go do pthread_create)
        //utworzenie nowego wątku
        pthread_create(&tid, nullptr, handle_client, new_sock);
        //odłączenie wątku (nie trzeba ręcznie po nim sprzątać)
        pthread_detach(tid);
    }

    //sprzątanie
    closesocket(server_socket);
    WSACleanup();
    return 0;
}
