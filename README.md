# BioReaktor - sterowanie
## _Instrukcja dla projektu układu elektronicznego_
Ten dokument zawiera instrukcje dzięki którym możliwe jest uruchomienie programu na płytce ArduinoUno oraz opis użytego protokołu komunikacyjnego wraz z przykładamai

### Użyte technologie:
- [ArduinoUno Rev3] - 
- [Platformio] -
- [VisualStudio Code] - 
- [freeRTOS] - 

### Uruchomienie programu 


### Odbierane wiadomości

# Komendy

❗ Wszystkie komendy wysyłane musi poprzedzać dodanie  `CMD`.

##### ==DEBUG_FAST==
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
##### ==DEBUG_PUMP==
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
