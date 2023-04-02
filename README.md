# BioReaktor - układ sterowania
## _Instrukcja dla projektu układu elektronicznego_
Ten dokument zawiera instrukcje dzięki którym możliwe jest uruchomienie programu na płytce ArduinoUno oraz opis użytego protokołu komunikacyjnego wraz z przykładami.

### Użyte technologie:
- [ArduinoUno Rev3]
- [Platformio]
- [VisualStudio Code]

### Uruchomienie programu 

# Komunikacja

Komunikacja z komputerem odbywa się po przez wykorzystanie **portu szeregowego**, to obsługi protokołu można wykorzystać każdy terminal portu szeregowe. Dostępnę są takie rowziązania jak:

- **ArduinoIDE** - Serial Monitor
- **Tera Term**
- **Putty**

Prędkość tranmisji obecnie ustawiona jest na **115200** ale można ją zmienić w kodzie plik ```src/main.cpp``` .

```sh
...
// the setup function runs once when you press reset or power the board
void setup()
{
  // initialize serial communication at 115200 bits per second:
  Serial.begin(115200);

  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.
  }
...
```


# Odbierane wiadomości

❗Każda z ramek ma taki sam format przesyłanych wiadomości. Wiadomość zaczyna się od znaku `$<` następnym elementem jest typ obbieranej ramki obecnie są dwa: 
- `DF` - ramka szybka
- `DP` - ramka pomp

Po typie ramki pojawia się znak `?` a następnie przekazywane są dane, zależności od typu ramki dane różnią się. Wiadomość zakończona jest znakami `>&`.


### Ramka szybka
Opis ramki szybkiej `DF`

```sh
$<DF?PH:{Czujnik PH},TEMP:{Czujnik temperatury},GS:{Czujnik tlenu}>&
```
Wartości otrzymywane z czujników (```{Czujnik PH}```, ```{Czujnik temperatury}```,oraz ```{Czujnik tlenu}```)są podawane z dokładnością do **trzeciego** miejsca po przecinku.

`Przykład: `
```sh
$<DF?PH:1.736,TEMP,21.997,GS:1.210>&
```


### Ramka pomp
Opis ramki szybkiej `DP`
```sh
$<DP?1:{Pompa 1},2:{Pompa 2},3:{Pompa 3},4:{Pompa 4}>&
```
- mininlana wartość - **0**
- maksymalna wartość - **255**

Wartości otrzymywane z pomp (```{Pompa 1}```,```{Pompa 2}```,```{Pompa 3}```,oraz ```{Pompa 4}```), z zakresu ```0-255```  są tak naprawdę odzwierciedleniem wartości zadanych przez użytkownika, jest to ramka barzdziej pomocnicza. 

`Przykład: `
```sh
$<DP?1:25,2:137,3:151,4:92>&
```

# Komendy

### ?
**Komenda pomocy** - zwaraca informacje o wszystkich dotępnych komendach przydatna do pobranie informacji w przypadku braku dostepu do dokumentacji

`Przykład:`
```sh
?
```

```
***
❗❗❗ Wszystkie wysyłane komendy funckyjne  musi poprzedzać dodanie  `CMD`.
***
### DEBUG_FAST
**Komenda ramki szybkiej** - wyłącza oraz włącza wysyłanie danych szybkich **domyślnie włączona** 

```sh
CMD,DEBUG_FAST,{on/off} 
```
| Argument | Zakres wartości | Opis argumentów 
| -------- | --------------- | ----------- |
| {on/off}   | **0** / **1** | **0** (wyłączone) lub **1** (włączone)

`Przykład:`
```sh
CND,DEBUG_FAST,1 // Włączenie ramki szybkiej
```
***
### DEBUG_PUMP
**Komenda ramki szybkiej** - wyłącza oraz włącza wysyłanie danych z pomp  **domyślnie włączona** 

```sh
CMD,DEBUG_PUMP,{on/off} 
```
| Argument | Zakres wartości | Opis argumentów 
| -------- | --------------- | ----------- |
| {on/off}   | **0** / **1** | **0** (wyłączone) lub **1** (włączone)

`Przykład:`
```sh
CMD,DEBUG_PUMP,0 // Wyłączenie ramki pomp
```
***

### SET_PUMP
**Komenda ustawia prędkość silnika pomp** 

```sh
CMD,SET_PUMP,{n},{x}
```
| Argument | Zakres wartości | Opis argumentów 
| -------- | --------------- | ----------- |
| {n}   | **1 - 4** | pompy są ponumerowane od **1** do **4**
| {x}   | **0 - 255** | **0** (wyłączone), **255** (maksymalna wartość)

`Przykład:`
```sh
CMD,SET_PUMP,2,170 // Ustawienie pompy nr 2 na wartość 170
```
```sh
CMD,SET_PUMP,1,0 // Ustawienie pompy nr 2 na wartość 0 (wyłączenie)
```
```sh
CMD,SET_PUMP,4,255 // Ustawienie pompy nr 2 na wartość 255 (maksymalna wartość)
```
***

# Ramki odpowiedzi
*Każda otrzymana przez urządzenie ramka komendy odysła ramkę odpowiedzi.*



**Wyróżnione typy odpowiedzi:**
* OK
* Syntax Error - błąd składni
* Nieznana komenda
* Niepoprawne paremetry komendy

| Odpowiedź | Zmienna |Wartość 
| -------- | --------------- | --------------- |
| OK   | OK | **0** 
| Syntax Error  |  BAD_SYNTAX | **1** |
| Nieznana komenda | INVALID_COMMAND  | **2** | 
| Syntax Error | INVALID_PARAMETER | **3** |

Ramki odpowiedzi wysyłane są w jendym formacie 
```sh
{cmd}|ERROR|{x}
```

| Argument | Zakres wartości | Opis argumentów 
| -------- | --------------- | ----------- |
| {cmd}   | **tekst** | kopia przesłanej komendy
| {x}   | **0 - 3** | kod błędu

`Przykład:`
```sh
CMD,SET_PUMP,2,170|ERROR|0
```

## License

MIT

[//]: # (These are reference links used in the body of this note and get stripped out when the markdown processor does its job. There is no need to format nicely because it shouldn't be seen. Thanks SO - http://stackoverflow.com/questions/4823468/store-comments-in-markdown-syntax)

[ArduinoUno Rev3]: <https://store-usa.arduino.cc/products/arduino-uno-rev3>
[Platformio]: <https://platformio.org/>
[VisualStudio Code]: <https://code.visualstudio.com/>
