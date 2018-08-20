# PDS_Server
This repository contains the __Server part__ of the school project due for the _Programmazione di Sistema_ class at _Politecnico di Torino_, Master of Science in _Computer Engineering_.

## Requirements
The project consists in a client-server solution for remote control of one or more Windows machines.
The system is made of two parts: a server, written in C++, and a client, written in C#.

### Server part
The server shall:
- [x] expose a socket and listen for a client on a specific port. No security is required for the communication between the two peers.
- [x] obtain the list of all the running applications having a graphical user interface and their processes
- [x] get the icon associated to each running application
- [x] know which application is currently on focus
- [x] send, to the application currently on focus, key combinations made of none or more modifiers (<kbd>CTRL</kbd> / <kbd>ALT</kbd> / <kbd>SHIFT</kbd>) followed by the corresponding keycode (<kbd>a</kbd> / <kbd>b</kbd> / <kbd>c</kbd> /... / <kbd>1</kbd> / <kbd>2</kbd> / <kbd>3</kbd> / ... / <kbd>BACKSPACE</kbd> / <kbd>DEL</kbd> / <kbd>ESC</kbd> / ...)
- [x] send the list of running application to the client, as soon as the connection is accepted, and next send all the important events occurring on the server machine, such as change of focus, closing of an application, starting of a new application
- [x] have a minimal graphical user interface living in the system tray area

### Client part
The client shall:
- [x] have a graphical user interface
- [x] get the list of applications running on the server, the full name of the process and the associated icon (if available)
- [x] receive a notification whenever the above-mentioned list or focus change
- [x] visualize a graphical summary of the server's current activities and the percentage of time each application has been on focus
- [x] send a key combination to the application currently on focus on the server
- [x] [OPTIONAL] be able to connect to many servers, visualize their statistics and send key combinations to all the servers currently running a specific application (e.g. "send <kbd>CTRL</kbd>-<kbd>X</kbd> to all the server running Firefox")
