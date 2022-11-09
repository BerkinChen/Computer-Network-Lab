/**
 * @file myFTP_client.h
 * @author Berkin Chen (berkinchen@gmail.com)
 * @brief The header file of myFTP_client
 * @version 0.3
 * @date 2022-10-06
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "myFTP_file.h"

bool auth = false;       // The flag of the authentication
bool connected = false;  // The flag of the connection

int Open(int& sock, const char* ip, int port);
int Recv_Message(int& sock);
int Send_Message(int type, int& sock, char* data);
int Recv_File(int& sock, const char* filename);
int Send_File(int& sock, const char* filename);

/**
 * @brief The function to open the connection
 * 
 * @param sock : The socket of the connection
 * @param ip : The ip address of the server
 * @param port : The port of the server
 * @return int : The status of the function
 */
int Open(int& sock, const char* ip, int port) {
  sock = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in addr;
  addr.sin_port = htons(port);
  addr.sin_family = AF_INET;
  inet_pton(AF_INET, ip, &addr.sin_addr);
  return connect(sock, (struct sockaddr*)&addr, sizeof(addr));
}

/**
 * @brief The function of sending the message to the server
 * 
 * @param type : The type of the message
 * @param sock : The socket of the connection
 * @param data : The data of the message
 * @return int : The status of the function
 */
int Send_Message(int type, int& sock, char* data = nullptr) {
  char buffer[MAXLINE];
  myFTP_Message message;
  if (type == OPEN) Set_Header(message.m_header, 0xA1, 0, 12);
  if (type == AUTH) Set_Header(message.m_header, 0xA3, 0, 13 + strlen(data));
  if (type == LIST) Set_Header(message.m_header, 0xA5, 0, 12);
  if (type == GET) Set_Header(message.m_header, 0xA7, 0, 13 + strlen(data));
  if (type == PUT) Set_Header(message.m_header, 0xA9, 0, 13 + strlen(data));
  if (type == QUIT) Set_Header(message.m_header, 0xAB, 0, 12);
  message.m_data = data;
  memset(buffer, 0, sizeof(buffer));
  memcpy(buffer, &message.m_header, sizeof(message.m_header));
  send(sock, buffer, sizeof(message.m_header), 0);
  int length = ntohl(message.m_header.m_length) - sizeof(myFTP_Header);
  if (length > 0) {
    message.m_data = new char[length];
    memset(message.m_data, 0, length);
    memcpy(message.m_data, data, length);
  }
  int total = 0;
  while (length > 0) {
    memset(buffer, 0, sizeof(buffer));
    memcpy(buffer, message.m_data + total, length > MAXLINE ? MAXLINE : length);
    int cnt = send(sock, buffer, length > MAXLINE ? MAXLINE : length, 0);
    if (cnt == -1) return -1;
    total += cnt;
    length -= cnt;
  }
  return 0;
}

/**
 * @brief The function of receiving the message from the server
 * 
 * @param sock : The socket of the connection
 * @return int : The status of the function
 */
int Recv_Message(int& sock) {
  char buffer[MAXLINE];
  myFTP_Message message;
  if (recv(sock, buffer, sizeof(message.m_header), 0) == -1) return 0;
  memcpy(&message.m_header, buffer, sizeof(message.m_header));
  if (memcmp(message.m_header.m_protocol, "\xe3myftp", MAGIC_NUMBER_LENGTH) !=
      0) {
    printf("\033[0m$ \033[31mInvalid protocol\n");
    return -1;
  }
  int length = ntohl(message.m_header.m_length) - sizeof(myFTP_Header);
  if (length > 0) {
    message.m_data = new char[length];
    memset(message.m_data, 0, length);
  }
  int total = 0;
  while (length > 0) {
    memset(buffer, 0, sizeof(buffer));
    int cnt = recv(sock, buffer, length > MAXLINE ? MAXLINE : length, 0);
    if (cnt == -1) return -1;
    memcpy(message.m_data + total, buffer, cnt);
    total += cnt;
    length -= cnt;
  }
  if (message.m_header.m_type == 0xA2 && message.m_header.m_status == 1) {
    printf("\033[0m$ \033[32mConnection request accepted\n");
    connected = true;
    return OPEN;
  }
  if (message.m_header.m_type == 0xA4) {
    if (message.m_header.m_status == 1) {
      printf("\033[0m$ \033[32mAuthentication request accepted\n");
      auth = true;
    } else {
      printf("\033[0m$ \033[31mAuthentication request denied\n");
      close(sock);
      exit(0);
    }
    return AUTH;
  }
  if (message.m_header.m_type == 0xA6) {
    printf("\033[33m----- file list start -----\n%s", message.m_data);
    printf("----- file list end -----\n");
    return LIST;
  }
  if (message.m_header.m_type == 0xA8) {
    if (message.m_header.m_status == 1) {
      printf("\033[0m$ \033[32mFile transfer request accepted\n");
      return GET;
    } else {
      printf("\033[0m$ \033[31mFile transfer request denied\n");
      return 0;
    }
  }
  if (message.m_header.m_type == 0xAA) {
    printf("\033[0m$ \033[32mFile transfer request accepted\n");
    return PUT;
  }
  if (message.m_header.m_type == 0xAC) {
    printf("\033[0m$ \033[33mConnection closed\n");
    close(sock);
  }
  return 0;
}