# Redis-in-C
A Redis clone implementation built in C. Built for learning purposes and as a portfolio project. The project implements core Redis commands and serves as a simplified version built from scratch, helping to deepen understanding of database internals and networking concepts.

# 🚀 Getting Started

## Prerequisites
* A C compiler (e.g., GCC, MinGW).
* CMake (3.10 or higher) must be installed and available in your system's PATH.
* This project is designed to be compiled and run on Windows operating systems only (so far). 
* Windows Specific: If compiling on Windows, you must have a modern toolchain like MinGW or MSYS2 installed and configured to use the gcc command. Winsock (ws2_32) linking is handled automatically by the provided CMake files.

## 🧑‍💻 Installation and Compilation
* Compilation Instructions
We use an out-of-source build to keep the source directory clean. All commands should be run from the root directory of the project 

1. First, clone the repository from GitHub to get all the necessary source files:
```sh
git clone https://github.com/haywoord/Redis-in-C.git
cd Redis-in-C
``` 

2. Configure the Build Directory
First, create a separate directory (conventionally named build) and run cmake from inside it to configure the project files.
```sh
mkdir build
cd build
cmake ..
```

3. Build the Executables
Next, run the build command. CMake will automatically compile all source files (server.c, client.c, linlist.c, etc.) and link them into two separate programs: server.exe and client.exe.
```sh
# This command automatically detects your build system (like MinGW or Make)
cmake --build .
```
After compilation, you will find the final executables inside the build/src/server/ directory (or similar, depending on your environment).

## 🖥️ Running the Application
Since the client and server are separate programs, they must be run in separate terminal sessions.

1. Start the Server

* Open the first terminal and navigate to the output directory to start the server. This program will block, waiting for client connections.
```sh
# Example path, may vary slightly
./src/server/server.exe 
```
The server currently only supports 1 connection.

2. Run the Client

* Open a second terminal and run the client to connect to the listening server.
```sh
# Example path, may vary slightly
./src/server/client.exe 
```
The client will connect, and you can begin testing RESP commands.

# 🧠 Core Commands Implemented
This project currently supports a only a small amount of standard Redis commands. 
* PING : Responds with "PONG".
* ECHO message : Returns the given string "message".
* QUIT : Closes connection with the server.
* SET key value : Set key to hold the string value. If key already holds a value, it will not be overwritten.
* GET key : Get the string value of key. If the key does not exist, the error is returned.
* DEL key : Removes the specified keys.
* KEYS : Returns all keys matching pattern.