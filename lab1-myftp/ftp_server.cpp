/**
 * @file ftp_server.cpp
 * @author Berkin Chen (berkinchen@gmail.com)
 * @brief The file of the server
 * @version 0.3
 * @date 2022-10-06
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "myFTP_server.h"
#define MAX_THREAD_SIZE 10

int client_cnt = 0;
int client_list[MAX_THREAD_SIZE] = {0};

void* work(int* client) {
  // use loop to receive the message
  printf("\033[0m$ \033[33mClient %d connected\n", *client);
  while (true) {
    int t = Recv_Message(*client);
    if (t == -1 || t == QUIT) break;
  }
  close(*client);
  global_mutex.lock();
  client_cnt--;
  global_mutex.unlock();
  return NULL;
}

int main(int argc, char** argv) {
  if (argc != 3) {
    printf("Usage: ./exec <server_ip> <server_port>, argc = %d\n", argc);
    exit(0);
  }
  char* ip = argv[1];
  int port = atoi(argv[2]);
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  // Open the client socket
  struct sockaddr_in addr;
  addr.sin_port = htons(port);
  addr.sin_family = AF_INET;
  inet_pton(AF_INET, ip, &addr.sin_addr);
  bind(sock, (struct sockaddr*)&addr, sizeof(addr));
  listen(sock, MAX_THREAD_SIZE);
  // use loop to accept the client
  while (true) {
    int i = 0;
    for (;i < MAX_THREAD_SIZE; i++) {
      if (client_list[i] == 0) break;
    }
    client_list[i] = accept(sock, NULL, NULL);
    // create a thread to handle the client
    /**/
    global_mutex.lock();
    client_cnt++;
    global_mutex.unlock();
    std::thread t(work, &client_list[i]);
    t.detach();
  }
  return 0;
}