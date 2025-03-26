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
Program został napisany w języku C/C++. Wątki tworzone są w ramach klasy std::thread. Do sycnhronizacji wątków używane są klasy sem_t oraz mutex odpowiednio zadane plikami nagłówkowymi <semaphore.h>, <mutex>. Nagłówek <semaphore.h> pochodzący z C został użyty przez trudności związane z dynamiczną alokacją semaforów klasy counting_semaphore z nagłówka <semaphore>. Stąd mieszane używanie nagłówków z C i C++, gdyż początkowo zakładano używanie narzędzi ściśle związanych z C++. Żeby sem_t nie było samotnie, towarzyszy mu funkcja printf z języka C.  

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


