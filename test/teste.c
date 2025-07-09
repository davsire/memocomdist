#include <stdio.h>
#include <string.h>
#include "../api/api.h"

// Rodar programa com os seguintes parâmetros para os testes:
// -p=4 -b=8 -t=2 -c=4
// Total de 16 bytes, 4 bytes por processo

#define TAM_BUFFER 4096

void leitura_escrita_sucesso() {
  int status;
  char buffer[TAM_BUFFER] = "prog distribuida";
  printf("Conteúdo no início: %s\n", buffer);

  status = escreve(0, buffer, strlen(buffer));
  if (status == SUCESSO) {
    printf("Escrita realizada com sucesso!\n");
  }

  status = le(5, buffer, 11);
  if (status == SUCESSO) {
    printf("Leitura realizada com sucesso! conteúdo: %s\n", buffer);
  }
}

void leitura_escrita_fora_limite() {
  int status;
  char buffer[TAM_BUFFER] = "programacao distribuida";

  status = escreve(0, buffer, strlen(buffer));
  if (status == FORA_LIMITE_MEMORIA) {
    printf("Ultrapassou o limite na escrita...\n");
  }

  status = le(5, buffer, 20);
  if (status == FORA_LIMITE_MEMORIA) {
    printf("Ultrapassou o limite na leitura...\n");
  }
}

void leituras_sucessivas() {
  int status;
  char buffer[TAM_BUFFER];

  status = le(5, buffer, 11);
  if (status == SUCESSO) {
    printf("Conteúdo (1): %s\n", buffer);
  }

  status = le(5, buffer, 11);
  if (status == SUCESSO) {
    printf("Conteúdo (2): %s\n", buffer);
  }
}

int main() {
  int caso_teste = 0;

  do {
    printf("Digite o caso de teste: \n");
    printf("1 - Leitura e escrita com sucesso\n");
    printf("2 - Leitura e escrita fora do limite da memória\n");
    printf("3 - Leituras sucessivas (uso e invalidação de cache)\n");
    scanf("%d", &caso_teste);
  } while (caso_teste < 1 || caso_teste > 3);

  switch (caso_teste) {
    case 1:
      leitura_escrita_sucesso();
      break;
    case 2:
      leitura_escrita_fora_limite();
      break;
    case 3:
      leituras_sucessivas();
      break;
    default:
      break;
  }
}
