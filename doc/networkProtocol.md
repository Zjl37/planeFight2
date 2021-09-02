# PlaneFight protocol

Zjl37's planeFight protocol

version 0.3.0

## Overview

This is the network protocol that supports planeFight multiplayer game. This document mainly describes what data to transmit over the network, in what form and when.

This new protocol is documented for the convenience of those who want to look into our networking part and who want to develop their own client application. We hope this document is helpful and you can use our protocol if you're developing your own client (application) of planeFight (the same game). If you have question or want to extend something, please fill an issue in the corresponding GitHub repository or email the author.

The versioning of this protocol roughly follows the rule of [Semantic Versioning](https://semver.org/). However, a client application is not required to follow semantic versioning, since most probably it doesn't declare a public API.

## Connection

The planeFight protocol is based on TCP connection. The preferred server port is 51937. (This should be the default port. However, other ports are ok if customizable.)

### Version verification

After the connection is established, both sides should send this request header before they can transmit any packets:

```
PLAY Zjl37's planeFight protocol <protocol-version-number>
<zero-or-more-header>

```

Each line should end with CR LF (`"\r\n"`). After all headers there should be an empty line. For example:

```
PLAY Zjl37's planeFight protocol 0.3.0
User-Agent: Zjl37's planeFight console application 2.6 (Zjl37/planefight2)

```

Here we define these headers (this header):

- `User-Agent`, a human-readable plain text string of the client application name and version.

The implementation may define other headers, which will probably be ignored by other clients.

The **client side** should not expect server's request to arrive before it sends its request.

When receiving the request, each side should do a version check and then disconnect if necessary. If the request line does not start with `PLAY Zjl37's planeFight protocol`, the clients' behavior afterward is undefined. If the Major version the server side receives is larger than its own, it must kill the connection. If the both sides' Major and Minor version are the same, both sides must not disconnect.

After the version check, if the connection is still alive, the protocol version used in this session is the smaller one of the two sides' proposals.

### Disconnection

Disconnection may happen

- at failure of version verification or,
- after one player Gives up before game starts or,
- after both side has received other's map at gameover.

In other circumstances than mentioned above a disconnection is seen as an network error and the game should abort abnormally.

## Packet

```
 0            4                    8
+------------+--------------------+-----------/ /-+
| uint32 len | uint32 packet-type | <packet body> |
+------------+--------------------+-----------/ /-+
```

A packet is a consecutive byte sequence starting with a 32-bit unsigned integer indicating the size of the packet body. The 4~7th byte indicates the packet type. All supported packet type value are listed below. Unsupported packet will be ignored.

> NOTE:
>
> All integers mentioned below, when stored as byte sequence, is in **big endian**.

### 0x6a6f696e Join

This is the first packet the client side should send. The packet body can be empty (body size can be zero).

### 0x676d6966 Game-info

```
 8          10         12         14          15           19
+----------+----------+----------+-----------+------------+
| uint16 w | uint16 h | uint16 n | field8 st | uint32 gid |
+----------+----------+----------+-----------+------------+
```

This should be sent by server when receving Join packet. This packet contains information of the current game to play. Its size should be no less than 7.

- `w`, `h`: map size in x, y dimension
- `n`: number of planes
- `st`
	- 1st bit: enable Cross-border mode: If Cross-border mode is enabled, this bit is on.
	- 2nd bit: enable Completely-destroy: If Completely-destroy enabled, this bit is on.
	- 3rd bit: is server first: If the server goes first, this bit is on.
- `gid`: game id (currently unused)

> NOTE:
>
> The `i`-th bit of byte (char) `x` is `x>>(i-1) & 1`.

### 0x6e616d65 Name

```
 8
+---------------/ /-+
| string playername |
+---------------/ /-+
```

This should be sent by both sides soon after they receive/send Game-info packet.

- `playername`: player name. The length of this string is `len`.

> NOTE:
>
> All strings should **NOT** be null terminated (as C style strings are). 
>
> All strings should be encoded in UTF-8. 

### 0x67767570 Give-up

This should be sent when the user quit when the game has not started. The packet body can be empty.

### 0x61727264 Ready

This should be sent when the user has its map set up (arranged its battle-field) and determined not to change it. The packet body can be empty.

The game automatically starts when both players are ready.

### 0x69737264 Surrender

This should be sent when the user surrender. The packet body can be empty.

### 0x6174616b Attack

```
 8          10         12
+----------+----------+
| uint16 x | uint16 y |
+----------+----------+
```

This should be sent only when it is the user's turn, otherwise the behavior of the other client is undefined. The size of the packet should be 4.

- `x`, `y`: the attack coordinate. The coordinate number should start from 0. (However, the client application can display coordinate otherwise)

### 0x72736c74 Attack-result

```
 8           9
+-----------+
| uint8 res |
+-----------+
```

This should be sent soon after Attack packet arrived. The size of this packet should be 1. When this is sent/received, the turn number increases by 1.

- `res`: Only three value are accepted: 0 for void, 1 for hit and 2 for destroy

### 0x6d796d70 Exchange-map

```
 8          10         12        13    8+5*n
+----------+----------+---------+-/ /-+
| uint16 x | uint16 y | uint8 d | ... |
+----------+----------+---------+-/ /-+
\...............v.............../
             n times
```

Send the map setup (battle-field arrangement) to the other player, described in an array of plane head information.

- `x`, `y`: the position of a plane head
- `d`: the facing (direction) of a plane. Only four value are accepted: 0 for head facing -y (y axis negative direction), 1 for +x, 2 for +y and 3 for -x.

This should be sent soon after gameover. A game is over when:

- one player Surrenders
- the number of destroyed planes of one player reaches the number of planes of the current game after an Attack.