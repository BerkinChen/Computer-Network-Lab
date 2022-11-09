/**
 * @file myFTP_server.h
 * @author Berkin Chen (berkinchen@gmail.com)
 * @brief The header file of myFTP_server
 * @version 0.4
 * @date 2022-10-10
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <thread>
#include <mutex>
#include <vector>
#include <map>
#include "myFTP_file.h"

std::mutex global_mutex;
struct file_mutex
{
    std::mutex mutex;
    std::string file_name;
};
std::vector<file_mutex> v;
std::map<std::string, std::shared_ptr<std::mutex>> mutex_map;


int Send_Message(int type, int& client, char* data = nullptr, bool auth = false);
int Recv_Message(int& client);
int Send_File(int& client, const char* filename);
int Recv_File(int& client, const char* filename);

/**
 * @brief The function to receive the message from the client
 * 
 * @param client : The socket of the client
 * @return int : The status of the function
 */
int Recv_Message(int& client) {
  char buffer[MAXLINE];
  myFTP_Message message;
  memset(buffer, 0, sizeof(buffer));
  if (recv(client, buffer, sizeof(message.m_header), 0) <= 0) return 0;
  memcpy(&message.m_header, buffer, sizeof(message.m_header));
  if (memcmp(message.m_header.m_protocol, "\xe3myftp", MAGIC_NUMBER_LENGTH) !=
      0) {
    printf("\033[0m$ \033[31mInvalid protocol\n");
    return 0;
  }
  int length = (int)(ntohl(message.m_header.m_length) - sizeof(myFTP_Header));
  if (length > 0) {
    message.m_data = new char[length];
    memset(message.m_data, 0, length);
  }
  int total = 0;
  while (length > 0) {
    memset(buffer, 0, sizeof(buffer));
    int cnt = recv(client, buffer, length > MAXLINE ? MAXLINE : length, 0);
    if (cnt == -1) return 0;
    memcpy(message.m_data + total, buffer, cnt);
    total += cnt;
    length -= cnt;
  }
  if (message.m_header.m_type == 0xA1) {
    printf("\033[0m$ \033[33mRecived connection request\n");
    Send_Message(OPEN, client);
    return OPEN;
  }
  if (message.m_header.m_type == 0xA3) {
    printf("\033[0m$ \033[33mRecived authentication request\n");
    if (strcmp(message.m_data, "user 123123") == 0) {
      printf("\033[0m$ \033[32mAuthentication request accepted\n");
      Send_Message(AUTH, client, nullptr, true);
    } else
      Send_Message(AUTH, client);
    return AUTH;
  }
  if (message.m_header.m_type == 0xA5) {
    printf("\033[0m$ \033[33mRecived list request\n");
    FILE* fp;
    if ((fp = popen("ls", "r")) == nullptr) {
      printf("\033[0m$ \033[31mFailed to execute command\n");
      return 0;
    }
    char* data = new char[MAXLINE];
    memset(data, 0, MAXLINE);
    if (fread(data, 1, MAXLINE, fp) == -1) {
      printf("\033[0m$ \033[31mFailed to read command output\n");
      return 0;
    }
    Send_Message(LIST, client, data);
    return LIST;
  }
  if (message.m_header.m_type == 0xA7) {
    printf("\033[0m$ \033[33mRecived get request\n");
    FILE* fp;
    if ((fp = fopen(message.m_data, "rb")) == nullptr) {
      printf("\033[0m$ \033[31mFailed to open file\n");
      Send_Message(GET, client, nullptr, false);
      return 0;
    }
    Send_Message(GET, client, nullptr, true);
    fclose(fp);
    Send_File(client, message.m_data);
    return GET;
  }
  if (message.m_header.m_type == 0xA9) {
    printf("\033[0m$ \033[33mRecived put request\n");
    Send_Message(PUT, client);
    if (mutex_map.find(message.m_data) == mutex_map.end()) {
      std::shared_ptr<std::mutex> ptr(new std::mutex);
      mutex_map[message.m_data] = ptr;
    }
    mutex_map[message.m_data]->lock();
    Recv_File(client, message.m_data);
    mutex_map[message.m_data]->unlock();
    //free and delete the mutex
    mutex_map.erase(message.m_data);
    return PUT;
  }
  if (message.m_header.m_type == 0xAB) {
    printf("\033[0m$ \033[33mRecived quit request\n");
    Send_Message(QUIT, client);
    close(client);
    return QUIT;
  }
  return 0;
}

/**
 * @brief The function to send the message to the client
 * 
 * @param type : The type of the message
 * @param client : The socket of the client
 * @param data : The data of the message
 * @param auth : The authentication status
 * @return int : The status of the function
 */
int Send_Message(int type, int& client, char* data, bool auth) {
  char buffer[MAXLINE];
  myFTP_Message message;
  if (type == OPEN) Set_Header(message.m_header, 0xA2, 1, 12);
  if (type == AUTH) Set_Header(message.m_header, 0xA4, auth ? 1 : 0, 12);
  if (type == LIST) Set_Header(message.m_header, 0xA6, 1, 13 + strlen(data));
  if (type == GET) Set_Header(message.m_header, 0xA8, auth ? 1 : 0, 12);
  if (type == PUT) Set_Header(message.m_header, 0xAA, 1, 12);
  if (type == QUIT) Set_Header(message.m_header, 0xAC, 0, 12);
  memset(buffer, 0, sizeof(buffer));
  memcpy(buffer, &message.m_header, sizeof(message.m_header));
  send(client, buffer, sizeof(message.m_header), 0);
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
    int cnt = send(client, buffer, length > MAXLINE ? MAXLINE : length, 0);
    if (cnt == -1) return -1;
    total += cnt;
    length -= cnt;
  }
  return 0;
}
