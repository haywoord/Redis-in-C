# Redis-in-C
A Redis clone implementation built in C. Built for learning purposes and as a portfolio project. The project implements core Redis commands and serves as a simplified version built from scratch, helping to deepen understanding of database internals and networking concepts.

# 🚀 Getting Started

## Prerequisites
* A C compiler (e.g., GCC, MinGW).
* CMake (3.10 or higher) must be installed and available in your system's PATH.
* This project is designed to be compiled and run on Windows operating systems only (so far).

## 🧑‍💻 Compiling
1. Clone the repository:
```sh
git clone https://github.com/your-username/Redis-in-C.git
cd Redis-in-C
``` 

2. Compile the source code using your C compiler. For example, with MinGW on Windows:
```sh
gcc server.c -lws2_32 -o redis_clone_server.exe
gcc client.c -lws2_32 -o redis_clone_client.exe
```
For now, compile only client.c and server.c.     
The -lws2_32 flag is to explicitly link to the Windows compiled library (ws2_32.lib).

## 🖥️ Runing the Server
1. After compiling, run the server and the client in 2 separate terminals
```sh
.\redis_clone_server.exe
.\redis_clone_client.exe
```
The server currently only supports 1 connection 

# 🧠 Core Commands Implemented
This project currently supports a only a small amount of standard Redis commands. 
* PING : Responds with PONG
* ECHO message : Returns the given string "message"
* QUIT : Closes connection with the server