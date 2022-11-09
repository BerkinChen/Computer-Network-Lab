/**
 * @file ftp_client.cpp
 * @author Berkin Chen (berkinchen@gmail.com)
 * @brief The file of the client
 * @version 0.3
 * @date 2022-10-06
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "myFTP_client.h"

int main(int argc, char** argv) {
  char buf[MAXLINE];
  int sock;
  printf("\033[34mWelcome to myFTP client!\n");
  while (true) {
    printf("\033[0m$ ");
    // Get the command line
    if (fgets(buf, MAXLINE, stdin) == NULL) {
      printf("\033[0m$ \033[31mfgets error\n");
      return 0;
    }
    if (buf[0] == '\n') continue;
    buf[strlen(buf) - 1] = '\0';
    // Get the command
    char* token = strtok(buf, " ");
    if (strcmp(token, "open") == 0) {
      char ip[MAXLINE], port[MAXLINE];
      int cnt = 0;
      while (token != NULL) {
        token = strtok(NULL, " ");
        if (token == NULL) break;
        if (cnt == 0) {
          strcpy(ip, token);
          cnt++;
        } else if (cnt == 1) {
          strcpy(port, token);
          cnt++;
        }
      }
      // Check the number of arguments
      if (cnt != 2) {
        printf("\033[0m$ \033[1mUsage: open <server_ip> <server_port>\n");
        continue;
      }
      // Connect to the server
      Open(sock, ip, atoi(port));
      Send_Message(OPEN, sock);
      Recv_Message(sock);
    }
    else if (strcmp(token, "auth") == 0) {
      char username[MAXLINE], password[MAXLINE];
      int cnt = 0;
      while (token != NULL) {
        token = strtok(NULL, " ");
        if (token == NULL) break;
        if (cnt == 0) {
          strcpy(username, token);
          cnt++;
        } else if (cnt == 1) {
          strcpy(password, token);
          cnt++;
        }
      }
      if (cnt != 2) {
        printf("\033[0m$ \033[1mUsage: auth <username> <password>\n");
        continue;
      }
      if (!connected) {
        printf("\033[0m$ \033[31mPlease connect to server first\n");
        continue;
      }
      // Send the authentication request
      char* data = strcat(strcat(username, " "), password);
      Send_Message(AUTH, sock, data);
      Recv_Message(sock);
    }
    else if (strcmp(token, "ls") == 0) {
      int cnt = 0;
      while (token != NULL) {
        token = strtok(NULL, " ");
        if (token == NULL) break;
        cnt++;
      }
      if (cnt != 0) {
        printf("\033[0m$ \033[1mUsage: ls\n");
        continue;
      }
      if (!connected) {
        printf("\033[0m$ \033[31mPlease connect to server first\n");
        continue;
      }
      if (!auth) {
        printf("\033[0m$ \033[31mPlease authenticate first\n");
        continue;
      }
      // Send the ls request
      Send_Message(LIST, sock);
      Recv_Message(sock);
    }
    else if (strcmp(token, "get") == 0) {
      char filename[MAXLINE];
      int cnt = 0;
      while (token != NULL) {
        token = strtok(NULL, " ");
        if (token == NULL) break;
        if (cnt == 0) {
          strcpy(filename, token);
          cnt++;
        }
      }
      if (cnt != 1) {
        printf("\033[0m$ \033[1mUsage: get <filename>\n");
        continue;
      }
      if (!connected) {
        printf("\033[0m$ \033[31mPlease connect to server first\n");
        continue;
      }
      if (!auth) {
        printf("\033[0m$ \033[31mPlease authenticate first\n");
        continue;
      }
      // Send the get request
      Send_Message(GET, sock, filename);
      if (Recv_Message(sock) == GET)
        Recv_File(sock, filename);
      else
        printf("\033[0m$ \033[31mFile not found\n");
    }
    else if (strcmp(token, "put") == 0) {
      char filename[MAXLINE];
      int cnt = 0;
      while (token != NULL) {
        token = strtok(NULL, " ");
        if (token == NULL) break;
        if (cnt == 0) {
          strcpy(filename, token);
          cnt++;
        }
      }
      if (cnt != 1) {
        printf("\033[0m$ \033[1mUsage: put <filename>\n");
        continue;
      }
      if (!connected) {
        printf("\033[0m$ \033[31mPlease connect to server first\n");
        continue;
      }
      if (!auth) {
        printf("\033[0m$ \033[31mPlease authenticate first\n");
        continue;
      }
      FILE* fp;
      if ((fp = fopen(filename, "rb")) == NULL) {
        printf("\033[0m$ \033[31mFile not found\n");
        continue;
      }
      // Send the put request
      Send_Message(PUT, sock, filename);
      if (Recv_Message(sock) == PUT) Send_File(sock, filename);
    }
    else if (strcmp(token, "quit") == 0) {
      Send_Message(QUIT, sock);
      Recv_Message(sock);
      break;
    }
    else if (strcmp(token, "help") == 0) {
      printf("\033[0m$ \033[1mUsage: open <server_ip> <server_port>\n");
      printf("         auth <username> <password>\n");
      printf("         ls\n");
      printf("         get <filename>\n");
      printf("         put <filename>\n");
      printf("         quit\n");
    }
    else {
      printf("\033[0m$ \033[1mUsage: open <server_ip> <server_port>\n");
      printf("         auth <username> <password>\n");
      printf("         ls\n");
      printf("         get <filename>\n");
      printf("         put <filename>\n");
      printf("         quit\n");
    }
  }
  printf("\033[34mGoodbye!\n");
  return 0;
}