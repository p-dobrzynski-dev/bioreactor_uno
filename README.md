# BioReaktor - sterowanie
## _Instrukcja dla projektu układu elektronicznego_
Ten dokument zawiera instrukcje dzięki którym możliwe jest uruchomienie programu na płytce ArduinoUno oraz opis użytego protokołu komunikacyjnego wraz z przykładamai

### Użyte technologie:
- [ArduinoUno Rev3] - 
- [Platformio] -
- [VisualStudio Code] - 
- [freeRTOS] - 

### Uruchomienie programu 


# Odbierane wiadomości

❗Każda z ramek ma taki sam format przesyłanych wiadomości. Wiadomość zaczyna się od znaku `$<` następnym elementem jest typ obbieranej ramki obecnie są dwa: 
- `DF` - ramka szybka
- `DP` - ramka pomp

Po typie ramki pojawia się znak `?` a następnie przekazywane są dane, zależności od typu ramki dane różnią się. Wiadomość zakończona jest znakami `>&`.


### Ramka szybka
Opis ramki szybkiej `DF`

```sh
<DF?PH:{Czujnik PH},TEMP:{Czujnik temperatury},GS:{Czujnik tlenu}>&
```
Wartości otrzymywane z czujników (```{Czujnik PH}```, ```{Czujnik temperatury}```,oraz ```{Czujnik tlenu}```)są podawane z dokładnością do **trzeciego** miejsca po przecinku.

`Przykład: `
```sh
<DF?PH:1.736,TEMP,21.997,GS:1.210>&
```


### Ramka pomp
Opis ramki szybkiej `DP`
```sh
$<DM?1:{Pompa 1},2:{Pompa 2},3:{Pompa 3},4:{Pompa 4}>&
```
- mininlana wartość - **0**
- maksymalna wartość - **255**

Wartości otrzymywane z pomp (```{Pompa 1}```,```{Pompa 2}```,```{Pompa 3}```,oraz ```{Pompa 4}```), z zakresu ```0-255```  są tak naprawdę odzwierciedleniem wartości zadanych przez użytkownika, jest to ramka barzdziej pomocnicza. 

`Przykład: `
```sh
$<DM?1:25,2:137,3:151,4:92>&
```

# Komendy

❗ Wszystkie komendy wysyłane musi poprzedzać dodanie  `CMD`.

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

## License

MIT

[//]: # (These are reference links used in the body of this note and get stripped out when the markdown processor does its job. There is no need to format nicely because it shouldn't be seen. Thanks SO - http://stackoverflow.com/questions/4823468/store-comments-in-markdown-syntax)

[ArduinoUno Rev3]: <https://store-usa.arduino.cc/products/arduino-uno-rev3>
[Platformio]: <https://platformio.org/>
[freeRTOS]: <https://www.freertos.org/>
[VisualStudio Code]: <https://code.visualstudio.com/>
