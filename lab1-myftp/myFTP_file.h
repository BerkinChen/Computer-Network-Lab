/**
 * @file myFTP_file.h
 * @author Berkin Chen (berkinchen@gmail.com)
 * @brief The head file of myFTP_file
 * @version 0.3
 * @date 2022-10-06
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "myFTP.h"

/**
 * @brief The function to receive the file from the server/client
 * 
 * @param conn : The socket of the connection
 * @param filename : The name of the file
 * @return int : The status of the function
 */
int Recv_File(int& conn, const char* filename) {
  FILE* fp;
  fp = fopen(filename, "wb");
  char buffer[MAXLINE];
  myFTP_Message message;
  if (recv(conn, buffer, sizeof(message.m_header), 0) == -1) return -1;
  memcpy(&message.m_header, buffer, sizeof(message.m_header));
  if (memcmp(message.m_header.m_protocol, "\xe3myftp", MAGIC_NUMBER_LENGTH) !=
      0) {
    printf("$ \033[31mInvalid protocol\n");
    return -1;
  }
  int length = ntohl(message.m_header.m_length) - sizeof(myFTP_Header);
  int cnt = 0;
  while (length > 0) {
    memset(buffer, 0, sizeof(buffer));
    cnt = recv(conn, buffer, length > MAXLINE ? MAXLINE : length, 0);
    if (cnt == -1) return -1;
    cnt = fwrite(buffer, 1, length > MAXLINE ? MAXLINE : length, fp);
    if (cnt == -1) return -1;
    length -= cnt;
  }
  return fclose(fp);
}

/**
 * @brief The function to send the file to the server/client
 * 
 * @param conn : The socket of the connection
 * @param filename : The name of the file
 * @return int : The status of the function
 */
int Send_File(int& conn, const char* filename) {
  FILE* fp;
  fp = fopen(filename, "rb");
  fseek(fp, 0, SEEK_END);
  int length = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  myFTP_Message message;
  Set_Header(message.m_header, 0xFF, 1, 12 + length);
  char buffer[MAXLINE];
  memset(buffer, 0, sizeof(buffer));
  memcpy(buffer, &message.m_header, sizeof(message.m_header));
  send(conn, buffer, sizeof(message.m_header), 0);
  int cnt = 0;
  while (length > 0) {
    memset(buffer, 0, sizeof(buffer));
    cnt = fread(buffer, 1, length > MAXLINE ? MAXLINE : length, fp);
    if (cnt == -1) return -1;
    cnt = send(conn, buffer, length > MAXLINE ? MAXLINE : length, 0);
    if (cnt == -1) return -1;
    length -= cnt;
  }
  fclose(fp);
  return 0;
}