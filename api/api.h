#ifndef API_H

#define API_H

#define SUCESSO 0
#define MSG_MALFORMADA 1
#define FORA_LIMITE_MEMORIA 2
#define ERRO_CRIACAO_SOCKET 3
#define ERRO_CONEXAO 4

int le(int posicao, char* buffer, int tamanho);

int escreve(int posicao, char* buffer, int tamanho);

#endif
