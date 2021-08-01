Page number is stored in `enum class PfPage`.

For each page the state of lower-level elements or other information is stored in one of the index of int array `::tab[]`.

## Page 0

A welcoming page with username input and language choose.

## Page 1

A main page with a gamemode menu.

### `tab[0]`

Multiplayer gamemode (server or client) option is shown if `tab[0]` is non-zero.

## Page 2

Pre-game page. Plane arragement and gamerule settings.

### `tab[0]`

Indicates mode.

| Value | Meaning           |
| ----- | ----------------- |
|   0   | Plane arrangement |
|   1   | Gamerule settings |

### `tab[1]`

Indicates the current selected plane direction.

| Value | Meaning |
| ----- | ------- |
|   0   |   up    |
|   1   |  right  |
|   2   |  down   |
|   3   |  left   |

## Page 4

Map size adjust page.

## Page 5

About page.

## Page 10

Game page. Attack, mark and erase.

### `tab[0]`

Indicates mode.

| Value | Meaning |
| ----- | ------- |
|   0   | Attack  |
|   1   |  Mark   |
|   2   |  Erase  |

### `tab[1]`

Indicates the current selected marker. The value is an index of string array `::marker[]`.

## Page 19

Post-game page, with winning or losing message.

## Page 41

Gamerule settings page, only for server.

## Page 42

Server inital page.

## Page 51

Client inital page with IP input.