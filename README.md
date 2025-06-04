# Problem ucztujących filozofów 
## Sformułowanie problemu[^1]
Problem ucztujących filozofów jest jednym z klasycznych problemów teorii współbieżności. Podstawowe sformułowanie problemu jest następujące:
- **n** filozofów zasiada przy okrągłym stole,
- pomiędzy każdym z filozofów lezy jeden widelec (łącznie jest **n** widelców),
- każdy z filozofów naprzemiennie je lub rozmyśla,
- każda z tych czynności jest skończona,
- aby zjeść, filozof musi podnieść oba sąsiadujące widelce.
[^1]: [Bartos Baliś,](https://home.agh.edu.pl/~balis/dydakt/tw/lab8/tw-5fil.pdf)
## Założenia projektowe 
Program został napisany w języku C/C++. Wątki tworzone są w ramach klasy std::thread. Do synchronizacji wątków używane są klasy sem_t oraz mutex odpowiednio zadane plikami nagłówkowymi <semaphore.h>, <mutex>. Nagłówek <semaphore.h> pochodzący z C został użyty przez trudności związane z dynamiczną alokacją semaforów klasy counting_semaphore z nagłówka &lt;semaphore&gt;. Stąd mieszane używanie nagłówków z C i C++, gdyż początkowo zakładano używanie narzędzi ściśle związanych z C++. Żeby sem_t nie było samotnie, towarzyszy mu funkcja printf z języka C.  

Program przyjmuje na wejściu jeden lub dwa argumenty. W przypadku braku argumentów, program powiadomi nas o błędzie. W przypadku jednego argumentu, traktowany jest on jako liczba filozofów zasiadających przy stole. W przypadku dwóch argumentów, pierwszy argument odpowiada liczbie filozofów, natomiast drugi ilości sekund, przez którą ucztują filozofowie. Domyślnie nasza platońska uczta zakończy się po upływie ok. 35 minut. 

Do zarządzania zasobem współdzielonym w postaci widelców użyto semaforów binarnych, natomiast w przypadku strumienia wyjścia użyto mutexów. Aby zapobiec zakleszczeniu, zaimplementowano mechanizm kelnera przy pomocy mutexu. Każda czynność (jedzenie i myślenie) trwa od 100 do 200 ms. 
Dodatkowo zaimplementowano funkcję wypisującą, kiedy dany filozof jest głodny oraz funkcję odliczającą czas, która kończy program i wypisuje ilość spożytych posiłków przez każdego filozofa. Zaimplementowano także wstępne sprawdzanie, czy pierwszy argument wpisywany przy uruchamianiu programu ma jakikolwiek sens (np. czy liczba filozofów nie wynosi 0, -300, czy "lollmao"). 
## Uruchomienie projektu 
W celu uruchomienia projektu, należy uruchomić w powłoce systemowej plik Filozofowie.exe znajdujący się w katalogu cmake-build-debug. Przykładowo ```$./Filozofowie.exe 5 ```. Takie wywoływanie programu zostało sprawdzone zarówno w PowerShellu jak i Bashu (Git Bash), z uwagą, że w przypadku Basha następowało pewne opóźnienie w drukowaniu na ekranie statusów filozofów. 
W razie problemów, całość kodu znajduje się w pliku main.cpp, co umożliwia szybkie ręczne skompilowanie. 
## Wątki 
Każdy z filozofów otrzymuje osobny wątek, tak więc po uruchomieniu programu tworzone jest **n** wątków postaci: 
```
 std::vector<std::thread> philosophers;
    philosophers.reserve(quantity);
    for(int i =0;i<quantity;i++)
    {
        philosophers.emplace_back(philosopher, i,quantity);
    }
```
Dodatkowo tworzony jest wątek liczący czas wykonywania się programu i kończący go: 
```
std::thread stop(stopper,seconds,quantity);
```
### Sekcje krytyczne 
W programie występują sekcje krytyczne dwojakiego rodzaju: 
- dostęp do wektora semaforów (widelców),
- dostęp do strumienia wyjścia.

Jak wcześniej wspomniano, pierwszy rodzaj rozwiązywany jest przy uzyciu semaforów, drugi natomiast przy użyciu mutexu. Dodatkowo aby zapobiec zakleszczeniu, przy sekcjach krytycznych dotyczących widelców występuje mutex w postaci kelnera.
Wszystkie sekcje krytyczne występujące w kodzie:
- funkcja eat (z wyłączeniem inkrementacji meals[id] oraz sleep_for())
```
void eat(int id, int left, int right)
{

    waiter.lock();
    sem_wait(&forks[left]);
    sem_wait(&forks[right]);
    waiter.unlock();

        print_mutex.lock();
        printf("Philosopher %d is eating...\n",id+1);
        print_mutex.unlock();

        meals[id]++;

        std::this_thread::sleep_for(std::chrono::milliseconds(100+rand()%100));

    sem_post(&forks[left]);
    sem_post(&forks[right]);
}
``` 
- funkcja think (z wyłączeniem sleep_for())
```
void think(int id)
{
    print_mutex.lock();
    printf("Philosopher %d is thinking...\n",id+1);
    print_mutex.unlock();
    std::this_thread::sleep_for(std::chrono::milliseconds(100+rand()%100));
}
```
- funkcja whine
```
void whine(int id)
{

    print_mutex.lock();
    printf("Philosopher %d is hungry!\n",id+1);
    print_mutex.unlock();
}
```
- funkcja stopper (z wyłączeniem pierwszej pętli oraz exit(0))
```
void stopper(int seconds,int quantity){

    int i=0;
    while(i<seconds)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        i++;
    }

    print_mutex.lock();
    printf("Eaten meals:\n");
    for(int i =0;i<quantity;i++)
    {
        printf("%3d ", i+1);
    }
    printf("\n");
    for(int i =0;i<quantity;i++)
    {
        printf("%3d ", meals[i]);
    }
    delete[] meals;
    print_mutex.unlock();
    exit(0);
}
```
# Serwer czatu 
## Założenia projektowe 
Program został napisany w języku C/C++. Do obsługi wątków zastosowano tym razem bibliotekę <pthread.h> oraz <atomic>. <pthread.h> wykorzystane zostało do tworzenia i zarządzania wątkami oraz synchronizacji za pomocą muteksów (pthread_mutex_t), natomiast <atomic> do utworzenia wątkowo-bezpiecznych flag. Jak widać ponownie wymieszane zostają ze sobą biblioteki C i C++. Z racji jednak, na to że zarówno wątki jak i muteksy pochodzą z biblioteki C, tym razem zamiast printf skorzystano z <iostream>, aby <atomic> nie czuł się samotnie jako biblioteka C++. Do obsługi komunikacji sieciowej skorzystano z biblioteki <winsock2.h>. Serwer tworzy osobny wątek dla każdego klienta i dba o synchronizację wiadomości od klientów. Klient widzi wiadomości w czacie i ma możliwość wysyłania wiadomości. Serwer ma możliwość wyłączenia się oraz wyrzucenia klienta z czatu. Klient może jedynie opuścić czat. Serwer i klient nie przyjmują przy wywołaniu żadnych argumentów, natomiast po uruchomieniu poproszą użytkownika o określenie portu na którym serwer ma działać, lub do którego klient ma się połączyć.  
Prace rozpoczęto od zaimplementowania serwera. Następnie zaimplementowano klienta. Finalnie dodano obsługę błędów oraz używanie komend (/quit, /kick).  

## Uruchomienie projektu 
W celu uruchomienia projektu, należy raz (jeśli chcemy korzystać z jednego serwera; nic nie stoi na przeszkodzie by uruchomić ich dowolną ilość na wolnych portach) uruchomić w powłoce systemowej plik Server.exe i podać numer portu, którego chcemy używać, a następnie dowolną ilość razy plik Client.exe z podaniem numeru portu uruchomionego serwera. 
## Wątki 
### Serwer 
Po uruchomieniu programu, tworzony jest wątek do obsługi konsoli. 
```
    pthread_t console_thread;
    pthread_create(&console_thread, nullptr, console_listener, nullptr);
    pthread_detach(console_thread);
```
Po uruchomieniu serwera na zadanym porcie, uruchamiana jest pętla, która nasłuchuje czy nikt nie chce się połączyć z serwerem. Nasłuchiwanie odświeżane jest co 100 milisekund. Odświeżanie, zamiast zwykłego oczekiwania na klienta zastosowano, aby móc prawidłowo obsłużyć komendę /quit. Jeśli klient chce się połączyć, tworzony jest dla niego nowy wątek. 
```
    pthread_t tid;
    SOCKET* new_sock = new SOCKET(client_socket);
    pthread_create(&tid, nullptr, handle_client, new_sock);
    pthread_detach(tid);
```
W przeciwieństwie do pierwszego projektu, nie przechowujemy listy ani wektora wątków. Zamiast tego przechowywany jest wektor gniazd używanych przez dany wątek. Wątki są odłączane, aby serwer nie musiał czekać na klientów, by zakończyć działanie. 
### Klient 
