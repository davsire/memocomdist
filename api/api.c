#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "api.h"

#define FETCH "FETCH"
#define STORE "STORE"
#define LOCALHOST "127.0.0.1"
#define MAX_BUFFER 4096

int le(int posicao, char* buffer, int tamanho) {
  int sock_fd;
  struct sockaddr_in server_end;
  int n_bytes;
  char mensagem[MAX_BUFFER];

  server_end.sin_family = AF_INET;
  server_end.sin_addr.s_addr = inet_addr(LOCALHOST);
  server_end.sin_port = htons(50000);

  if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    return 1;
  }

  if (connect(sock_fd, (struct sockaddr*)&server_end, sizeof(server_end)) == -1) {
    return 2;
  }

  sprintf(mensagem, "%s %d %d", FETCH, posicao, tamanho);
  write(sock_fd, &mensagem, strlen(mensagem));
  n_bytes = read(sock_fd, &mensagem, MAX_BUFFER);
  memcpy(buffer, mensagem, n_bytes);
}

int escreve(int posicao, char* buffer, int tamanho) {
  int sock_fd;
  struct sockaddr_in server_end;
  int n_bytes;
  char mensagem[MAX_BUFFER];

  server_end.sin_family = AF_INET;
  server_end.sin_addr.s_addr = inet_addr(LOCALHOST);
  server_end.sin_port = htons(50000);

  if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    return 1;
  }

  if (connect(sock_fd, (struct sockaddr*)&server_end, sizeof(server_end)) == -1) {
    return 2;
  }

  sprintf(mensagem, "%s %d %d %s", STORE, posicao, tamanho, buffer);
  write(sock_fd, &mensagem, strlen(mensagem));
  n_bytes = read(sock_fd, &mensagem, MAX_BUFFER);
}
