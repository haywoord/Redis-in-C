#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdarg.h>
#ifndef RESP_H
#define RESP_H

/**
 * @brief Sends a Redis Simple String response to a client.
 * * The function formats the given C-style string as a Redis Simple String
 * protocol message (+<string>\r\n) and sends it over the socket.
 * * @param osckt The socket descriptor of the client.
 * @param msg The null-terminated C string to send as the simple string.
 * @return int 0 on success, or -1 on error
 */
int sendSimpleString(SOCKET sock, const char *msg);

/**
 * @brief Sends a Redis Simple Error response to a client.
 * * The function formats the given C-style string as a Redis Simple Error
 * * protocol message (-Error message\r\n) and sends it over the socket.
 * @param sock The socket descriptor of the client.
 * @param msg The null-terminated C string to send as the simple error.
 * @return int 0 on success, or -1 on error.
 */
int sendSimpleError(SOCKET sock, const char *msg);

/**
 * @brief Sends a Redis Integer response to a client.
 * * The function formats the integer value as a string and sends it as a 
 * * Redis Integer protocol message (:value\r\n).
 * @param sock The socket descriptor of the client.
 * @param val The integer value to send.
 * @return int 0 on success, or -1 on error.
 */
int sendInteger(SOCKET sock, int val);

/**
 * @brief Sends a Redis Bulk String response to a client.
 * * This involves sending the length header ($<len>\r\n), the payload, 
 * * and the trailing delimiter (\r\n) in three separate socket writes.
 * @param sock The socket descriptor of the client.
 * @param msg The null-terminated C string payload.
 * @return int 0 on success, or -1 on error.
 */
int sendBulkString(SOCKET sock, const char *msg);

/**
 * @brief Sends a Redis Null Bulk String response to a client.
 * * This is typically used to indicate a key was not found or a nil value.
 * * The response format is simply "$-1\r\n".
 * @param sock The socket descriptor of the client.
 * @return int 0 on success, or -1 on error.
 */
int sendNull(SOCKET sock); 

/**
 * @brief Sends a Redis Array response to a client.
 * * Sends the array header (*<count>\r\n) followed by recursively sending 
 * * each element, typically as a Bulk String using sendBulkString.
 * @param sock The socket descriptor of the client.
 * @param count The number of elements in the array. If negative, sends a Null Array.
 * @param elements The array of C strings (char**) to be sent as Bulk Strings.
 * @return int 0 on success, or -1 on error.
 */
int sendArray(SOCKET sock, int count, char** elements);

// --- Client Command Sending ---

/**
 * @brief Formats and sends a client command as a RESP Array.
 * * Used by the client to serialize a command array header (*<argc>\r\n) 
 * * followed by arguments as bulk strings.
 * @param sock The server socket descriptor.
 * @param argc The number of arguments (including the command name).
 * @param args The 2D array of command arguments.
 * @return int 0 on success, or -1 on error.
 */
int sendCommand(SOCKET sock, int numArgs, char args[][100]);

/**
 * @brief Recursively parses a single RESP response from the server and prints it to the console.
 * * Reads the initial line, determines the type (+,-,:,$,*), and handles subsequent data reads 
 * * (for Bulk Strings and Arrays) recursively using helper functions.
 * @param sock The server socket.
 * @return int 0 on success, or -1 on error.
 */
int parseResponse(SOCKET sock);


// --- Utility and Receiving Functions ---

/**
 * @brief Reads exactly 'n' bytes from a socket into a buffer.
 * * Handles partial reads by repeatedly calling recv until 'n' bytes are read.
 * @param sock The socket descriptor.
 * @param buf The buffer to read the data into.
 * @param n The exact number of bytes to read.
 * @return int 0 on success, or -1 on error or if the connection closes prematurely.
 */
int read_n_bytes(SOCKET sock, char *buf, int n);

/**
 * @brief Reads a single line from a socket up to and including the CRLF delimiter (\r\n).
 * * Reads byte-by-byte until CRLF is found or the buffer is full.
 * @param sock The socket descriptor.
 * @param buf The buffer to store the line.
 * @param max The maximum size of the buffer.
 * @return int The number of bytes read (including CRLF), 0 if connection closed, or a negative value on error.
 */
int recvLine(SOCKET sock, char *buf, int max);

/**
 * @brief Parses a full RESP Array (command and arguments) from a client socket.
 * * Reads the array count header, then loops, reading the length header ($<len>\r\n) 
 * * and the payload for each argument into a predefined buffer array.
 * @param sock The client socket.
 * @param argc Output pointer to store the number of arguments found.
 * @param argv The 2D array buffer (char[][100]) to store the parsed arguments.
 * @return int 0 on success, or -1 on error (protocol violation, buffer overflow, etc.).
 */
int parseArrayFromSocket(SOCKET sock, int *argc, char argv[][100]); 

#endif