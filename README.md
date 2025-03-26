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
