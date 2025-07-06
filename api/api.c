#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "api.h"

#define FETCH "FETCH"
#define STORE "STORE"
#define LOCALHOST "127.0.0.1"
#define PORTA_PROCESSO_PADRAO 50000
#define MAX_BUFFER 4096

int obter_status_resposta(char* resposta) {
  return atoi(resposta);
}

char* obter_conteudo_resposta(char* resposta) {
  return strchr(resposta, '\n') + 1;
}

int le(int posicao, char* buffer, int tamanho) {
  int sock_fd;
  struct sockaddr_in server_end;
  int n_bytes;
  char mensagem[MAX_BUFFER];

  server_end.sin_family = AF_INET;
  server_end.sin_addr.s_addr = inet_addr(LOCALHOST);
  server_end.sin_port = htons(PORTA_PROCESSO_PADRAO);

  if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    return ERRO_CRIACAO_SOCKET;
  }

  if (connect(sock_fd, (struct sockaddr*)&server_end, sizeof(server_end)) == -1) {
    return ERRO_CONEXAO;
  }

  sprintf(mensagem, "%s %d %d", FETCH, posicao, tamanho);
  write(sock_fd, &mensagem, strlen(mensagem));
  n_bytes = read(sock_fd, &mensagem, MAX_BUFFER);
  close(sock_fd);

  int status = obter_status_resposta(mensagem);
  if (status == SUCESSO) {
    memcpy(buffer, obter_conteudo_resposta(mensagem), n_bytes);
    return SUCESSO;
  } else {
    return status;
  }
}

int escreve(int posicao, char* buffer, int tamanho) {
  int sock_fd;
  struct sockaddr_in server_end;
  char mensagem[MAX_BUFFER];

  server_end.sin_family = AF_INET;
  server_end.sin_addr.s_addr = inet_addr(LOCALHOST);
  server_end.sin_port = htons(PORTA_PROCESSO_PADRAO);

  if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    return ERRO_CRIACAO_SOCKET;
  }

  if (connect(sock_fd, (struct sockaddr*)&server_end, sizeof(server_end)) == -1) {
    return ERRO_CONEXAO;
  }

  sprintf(mensagem, "%s %d %d %s", STORE, posicao, tamanho, buffer);
  write(sock_fd, &mensagem, strlen(mensagem));
  read(sock_fd, &mensagem, MAX_BUFFER);
  close(sock_fd);

  return obter_status_resposta(mensagem);
}
