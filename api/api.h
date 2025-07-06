#ifndef API_H

#define API_H

#define SUCESSO 0
#define MSG_MALFORMADA 1
#define FORA_LIMITE_MEMORIA 2
#define LIMITE_CONEXOES_ATINGIDO 3
#define ERRO_CRIACAO_SOCKET 4
#define ERRO_CONEXAO 5

int le(int posicao, char* buffer, int tamanho);

int escreve(int posicao, char* buffer, int tamanho);

#endif
