#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>

#define PARAM_NUM_PROCESSOS "-p="
#define PARAM_NUM_BLOCOS "-b="
#define PARAM_TAM_BLOCOS "-t="
#define IGUAL "="
#define PORTA_INICIAL 50000
#define LIMITE_CONEXOES 10
#define MAX_BUFFER 4096

int num_processos = 0;
int num_blocos = 0;
int tam_blocos = 0;
int id_processo = 0;

typedef struct Bloco {
  int id;
  char enderecos[];
} bloco_t;

void obter_parametros_aplicacao(int argc, char** argv) {
  if (argc < 4) {
    printf("Informe os parâmetros:\n-p (número de processos)\n-b (número de blocos)\n-t (tamanho dos blocos)\n");
    exit(EXIT_FAILURE);
  }
  for (int i = 1; i < argc; i++) {
    if (strstr(argv[i], PARAM_NUM_PROCESSOS) != NULL) {
      num_processos = atoi(argv[i] + strcspn(argv[i], IGUAL) + 1);
      continue;
    }
    if (strstr(argv[i], PARAM_NUM_BLOCOS) != NULL) {
      num_blocos = atoi(argv[i] + strcspn(argv[i], IGUAL) + 1);
      continue;
    }
    if (strstr(argv[i], PARAM_TAM_BLOCOS) != NULL) {
      tam_blocos = atoi(argv[i] + strcspn(argv[i], IGUAL) + 1);
      continue;
    }
  }
  if (num_processos <= 0) {
    printf("Informe o número de processos (-p)\n");
    exit(EXIT_FAILURE);
  }
  if (num_blocos <= 0) {
    printf("Informe o número de blocos (-b)\n");
    exit(EXIT_FAILURE);
  }
  if (tam_blocos <= 0) {
    printf("Informe o tamanho dos blocos (-t)\n");
    exit(EXIT_FAILURE);
  }
}

void criar_processos() {
  for (int i = 1; i < num_processos; i++) {
    if (!fork()) {
      id_processo = i;
      continue;
    }
    break;
  }
}

bloco_t* obter_blocos_processo() {
  int num_blocos_processo = num_blocos / num_processos;
  bloco_t* blocos = malloc((sizeof(bloco_t) + sizeof(char) * tam_blocos) * num_blocos_processo);
  for (int i = 0; i < num_blocos_processo; i++) {
    blocos[i].id = i + (id_processo * num_blocos_processo);
    memset(blocos[i].enderecos, -1, tam_blocos);
  }
  return blocos;
}

void mapear_portas(int mapeamento_portas[]) {
  for (int i = 0; i < num_processos; i++) {
    mapeamento_portas[i] = PORTA_INICIAL + i;
  }
}

int main(int argc, char** argv) {
  obter_parametros_aplicacao(argc, argv);
  criar_processos();

  bloco_t* blocos = obter_blocos_processo();
  int mapeamento_portas[num_processos];
  mapear_portas(mapeamento_portas);

  int sock_fd, cliente_fd;
  struct sockaddr_in endereco, end_cliente;
  socklen_t tam_end_cliente = sizeof(end_cliente);
  char buffer[MAX_BUFFER];
  int nbytes;
  
  if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    printf("[PROCESSO %d] Erro ao criar socket\n", id_processo);
    exit(EXIT_FAILURE);
  }

  endereco.sin_family = AF_INET;
  endereco.sin_addr.s_addr = htons(INADDR_ANY);
  endereco.sin_port = htons(PORTA_INICIAL + id_processo);

  if (bind(sock_fd, (struct sockaddr *)&endereco, sizeof(endereco)) == -1) {
    printf("[PROCESSO %d] Erro ao associar porta ao socket\n", id_processo);
    close(sock_fd);
    exit(EXIT_FAILURE);
  }

  if (listen(sock_fd, LIMITE_CONEXOES) == -1) {
    printf("[PROCESSO %d] Erro ao criar limite de conexões\n", id_processo);
    close(sock_fd);
    exit(EXIT_FAILURE);
  }

  struct pollfd clientes[num_processos];
  for (int i = 0; i < num_processos; i++) {
    clientes[i].fd = -1;
  }

  clientes[0].fd = sock_fd;
  clientes[0].events = POLLIN;
  
  printf("[PROCESSO %d] Processo iniciado!\n", id_processo);
  while (1) {
    if (poll(clientes, num_processos, -1) == -1) {
      printf("[PROCESSO %d] Erro ao fazer polling\n", id_processo);
      close(sock_fd);
      exit(EXIT_FAILURE);  
    }

    if (clientes[0].revents & POLLIN) {
      cliente_fd = accept(sock_fd, (struct sockaddr *)&end_cliente, &tam_end_cliente);

      if (cliente_fd < 0) {
        printf("[PROCESSO %d] Erro ao aceitar novo cliente\n", id_processo);
        continue;
      }

      for (int i = 1; i < num_processos; i++) {
        if (clientes[i].fd == -1) {
          clientes[i].fd = cliente_fd;
          clientes[i].events = POLLIN;
        }
      }
    }

    for (int i = 1; i < num_processos; i++) {
      if (clientes[i].fd != -1 && (clientes[i].revents & POLLIN)) {
        nbytes = read(clientes[i].fd, &buffer, MAX_BUFFER);

        if (nbytes == 0) {
          close(clientes[i].fd);
          clientes[i].fd = -1;
          continue;
        }

        // @TODO: lidar com comunicação, oq tem abaixo é só pra testes
        buffer[nbytes - 1] = '\0';
        printf("[PROCESSO %d] msg: %s\n", id_processo, buffer);
      }
    }
  }

  close(sock_fd);
  exit(EXIT_SUCCESS);
}
