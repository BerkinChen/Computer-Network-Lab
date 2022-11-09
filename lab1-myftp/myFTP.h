/**
 * @file myFTP.h
 * @author Berkin Chen (berkinchen@gmail.com)
 * @brief The header file of myFTP
 * @version 0.3
 * @date 2022-10-06
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <arpa/inet.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/file.h>
#include <stdarg.h>
#define MAXLINE 2048
#define MAGIC_NUMBER_LENGTH 6
#define OPEN 1
#define AUTH 2
#define LIST 3
#define GET 4
#define PUT 5
#define QUIT 6

/**
 * @brief The struct of the myFTP header
 * 
 */
typedef struct myFTP_Header {
  uint8_t m_protocol[MAGIC_NUMBER_LENGTH]; /* protocol magic number (6 bytes) */
  uint8_t m_type;                          /* type (1 byte) */
  uint8_t m_status;                        /* status (1 byte) */
  uint32_t m_length;                       /* length (4 bytes) in Big endian*/
} __attribute__((packed)) myFTP_Header;

/**
 * @brief The struct of the myFTP message
 * 
 */
typedef struct myFTP_Message {
  myFTP_Header m_header;
  char* m_data;
} myFTP_Message;

/**
 * @brief The function to set the head of the message
 * 
 * @param header : The header of the message 
 * @param type : The type of the message
 * @param status : The status of the message
 * @param length : The length of the message
 */
void Set_Header(myFTP_Header& header, uint8_t type, uint8_t status, uint32_t length) {
  memset(&header, 0, sizeof(myFTP_Header));
  memcpy(header.m_protocol, "\xe3myftp", MAGIC_NUMBER_LENGTH);
  header.m_length = htonl(length);
  header.m_status = status;
  header.m_type = type;
}
