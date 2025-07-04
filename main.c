#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <signal.h>
#include <errno.h>

#define PARAM_NUM_PROCESSOS "-p="
#define PARAM_NUM_BLOCOS "-b="
#define PARAM_TAM_BLOCOS "-t="
#define IGUAL "="
#define ESPACO " "
#define LOCALHOST "127.0.0.1"
#define PORTA_INICIAL 50000
#define LIMITE_CONEXOES 10
#define MAX_BUFFER 4096
#define VALOR_VAZIO -1
#define FETCH "FETCH"
#define FETCH_BLOCO "FETCH_BLOCO"
#define STORE "STORE"
#define STORE_BLOCO "STORE_BLOCO"

// Lista de TO-DOs (remover na medida que concluir)
// - Tratar armazenamento de conteudos com espaço no protocolo
// - Tratar envio de lixo de buffer nas mensagens do protocolo
// - Implementar validações no protocolo para evitar mensagens malformadas
// - Implementar cache
// - Corrigir detalhes de buffer com valor 'vazio' (rever esse valor vazio)
// - Criar arquivo lib para API
// - Testar MUITO

typedef struct Bloco {
  int id;
  char* enderecos;
} bloco_t;

int fim_programa = 0;
int num_processos = 0;
int num_blocos = 0;
int tam_blocos = 0;
int num_blocos_processo = 0;
int id_processo = 0;
bloco_t* blocos;
int* mapeamento_portas;

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
    if (fork()) {
      break;
    }
    id_processo = i;
  }
}

void criar_blocos_processo() {
  num_blocos_processo = num_blocos / num_processos;
  blocos = malloc(sizeof(bloco_t) * num_blocos_processo);
  for (int i = 0; i < num_blocos_processo; i++) {
    blocos[i].id = i + (id_processo * num_blocos_processo);
    blocos[i].enderecos = malloc(sizeof(char) * tam_blocos);
    memset(blocos[i].enderecos, VALOR_VAZIO, tam_blocos);
  }
}

void mapear_portas() {
  mapeamento_portas = malloc(sizeof(int) * num_processos);
  for (int i = 0; i < num_processos; i++) {
    mapeamento_portas[i] = PORTA_INICIAL + i;
  }
}

void limpar_processo() {
  for (int i = 0; i < num_blocos_processo; i++) {
    free(blocos[i].enderecos);
  }
  free(blocos);
  free(mapeamento_portas);
}

void obter_dados_bloco(int id_bloco, char* destino) {
  int processo_bloco = id_bloco / num_blocos_processo;

  if (processo_bloco == id_processo) {
    memcpy(destino, blocos[id_bloco % num_blocos_processo].enderecos, tam_blocos);
    return;
  }

  char buffer[MAX_BUFFER];
  int sock_fd;
  struct sockaddr_in server_end;

  server_end.sin_family = AF_INET;
  server_end.sin_addr.s_addr = inet_addr(LOCALHOST);
  server_end.sin_port = htons(mapeamento_portas[processo_bloco]);

  if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    printf("[PROCESSO %d] Erro ao criar socket para obter dados de bloco\n", id_processo);
    return;
  }

  if (connect(sock_fd, (struct sockaddr*)&server_end, sizeof(server_end)) == -1) {
    printf("[PROCESSO %d] Erro ao se conectar com processo %d\n", id_processo, processo_bloco);
    close(sock_fd);
    return;
  }

  sprintf(buffer, "%s %d", FETCH_BLOCO, id_bloco);
  write(sock_fd, &buffer, MAX_BUFFER);
  read(sock_fd, &buffer, tam_blocos);
  close(sock_fd);

  memcpy(destino, buffer, tam_blocos);
}

void salvar_dados_bloco(int id_bloco, char* origem) {
  int processo_bloco = id_bloco / num_blocos_processo;

  if (processo_bloco == id_processo) {
    memcpy(blocos[id_bloco % num_blocos_processo].enderecos, origem, tam_blocos);
    return;
  }

  char buffer[MAX_BUFFER];
  int sock_fd;
  struct sockaddr_in server_end;

  server_end.sin_family = AF_INET;
  server_end.sin_addr.s_addr = inet_addr(LOCALHOST);
  server_end.sin_port = htons(mapeamento_portas[processo_bloco]);

  if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    printf("[PROCESSO %d] Erro ao criar socket para salvar dados de bloco\n", id_processo);
    return;
  }

  if (connect(sock_fd, (struct sockaddr*)&server_end, sizeof(server_end)) == -1) {
    printf("[PROCESSO %d] Erro ao se conectar com processo %d\n", id_processo, processo_bloco);
    close(sock_fd);
    return;
  }

  sprintf(buffer, "%s %d %s", STORE_BLOCO, id_bloco, origem);
  write(sock_fd, &buffer, MAX_BUFFER);
  close(sock_fd);
}

void fetch_dados(char* parametros, char* buffer) {
  int posicao_inicial = atoi(strtok(parametros, ESPACO));
  int n_bytes = atoi(strtok(NULL, ESPACO));

  bloco_t bloco_atual;
  bloco_atual.id = VALOR_VAZIO;
  bloco_atual.enderecos = malloc(sizeof(char) * tam_blocos);

  for (int i = 0; i < n_bytes; i++) {
    int id_bloco = (posicao_inicial + i) / tam_blocos;
    int endereco_bloco = (posicao_inicial + i) % tam_blocos;

    if (bloco_atual.id != id_bloco) {
      bloco_atual.id = id_bloco;
      obter_dados_bloco(id_bloco, bloco_atual.enderecos);
    }

    buffer[i] = bloco_atual.enderecos[endereco_bloco];
  }

  free(bloco_atual.enderecos);
  buffer[n_bytes] = '\0';
}

void fetch_bloco(char* parametros, char* buffer) {
  int id_bloco = atoi(parametros);
  memcpy(buffer, blocos[id_bloco % num_blocos_processo].enderecos, tam_blocos);
}

void store_dados(char* parametros) {
  int posicao_inicial = atoi(strtok(parametros, ESPACO));
  int n_bytes = atoi(strtok(NULL, ESPACO));
  char* conteudo = strtok(NULL, ESPACO);
  int tamanho_conteudo = strlen(conteudo);
  n_bytes = n_bytes > tamanho_conteudo ? tamanho_conteudo : n_bytes;

  bloco_t bloco_atual;
  bloco_atual.id = VALOR_VAZIO;
  bloco_atual.enderecos = malloc(sizeof(char) * tam_blocos);

  for (int i = 0; i < n_bytes; i++) {
    int id_bloco = (posicao_inicial + i) / tam_blocos;
    int endereco_bloco = (posicao_inicial + i) % tam_blocos;

    if (bloco_atual.id != id_bloco) {
      if (bloco_atual.id != VALOR_VAZIO) {
        salvar_dados_bloco(bloco_atual.id, bloco_atual.enderecos);
      }

      bloco_atual.id = id_bloco;
      obter_dados_bloco(id_bloco, bloco_atual.enderecos);
    }

    bloco_atual.enderecos[endereco_bloco] = conteudo[i];
  }

  salvar_dados_bloco(bloco_atual.id, bloco_atual.enderecos);
}

void store_bloco(char* parametros) {
  int id_bloco = atoi(strtok(parametros, ESPACO));
  char* conteudo = strtok(NULL, ESPACO);
  memcpy(blocos[id_bloco % num_blocos_processo].enderecos, conteudo, tam_blocos);
}

void finalizar_programa() {
  fim_programa = 1;
}

int main(int argc, char** argv) {
  signal(SIGINT, finalizar_programa);
  obter_parametros_aplicacao(argc, argv);
  criar_processos();
  criar_blocos_processo();
  mapear_portas();

  int sock_fd, cliente_fd;
  struct sockaddr_in endereco, end_cliente;
  socklen_t tam_end_cliente = sizeof(end_cliente);
  char buffer[MAX_BUFFER];
  int n_bytes;

  if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    printf("[PROCESSO %d] Erro ao criar socket principal\n", id_processo);
    exit(EXIT_FAILURE);
  }

  endereco.sin_family = AF_INET;
  endereco.sin_addr.s_addr = inet_addr(LOCALHOST);
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
    clientes[i].fd = VALOR_VAZIO;
  }

  clientes[0].fd = sock_fd;
  clientes[0].events = POLLIN;

  printf("[PROCESSO %d] Processo iniciado!\n", id_processo);
  while (!fim_programa) {
    if (poll(clientes, num_processos, -1) == -1) {
      if (errno == EINTR) {
        break;
      }
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
        if (clientes[i].fd == VALOR_VAZIO) {
          clientes[i].fd = cliente_fd;
          clientes[i].events = POLLIN;
          break;
        }
      }
    }

    for (int i = 1; i < num_processos; i++) {
      if (clientes[i].fd != VALOR_VAZIO && (clientes[i].revents & POLLIN)) {
        n_bytes = read(clientes[i].fd, &buffer, MAX_BUFFER);

        if (n_bytes == 0) {
          close(clientes[i].fd);
          clientes[i].fd = VALOR_VAZIO;
          continue;
        }

        buffer[n_bytes] = '\0';
        printf("[PROCESSO %d] Mensagem recebida: %s\n", id_processo, buffer);

        if (strstr(buffer, FETCH_BLOCO)) {
          fetch_bloco(buffer + strlen(FETCH_BLOCO), buffer);
          write(clientes[i].fd, &buffer, tam_blocos);
          continue;
        }

        if (strstr(buffer, FETCH)) {
          fetch_dados(buffer + strlen(FETCH), buffer);
          write(clientes[i].fd, &buffer, strlen(buffer));
          continue;
        }

        if (strstr(buffer, STORE_BLOCO)) {
          store_bloco(buffer + strlen(STORE_BLOCO));
          continue;
        }

        if (strstr(buffer, STORE)) {
          store_dados(buffer + strlen(STORE));
          continue;
        }
      }
    }
  }

  limpar_processo();
  for (int i = 0; i < num_processos; i++) {
    close(clientes[i].fd);
  }
  exit(EXIT_SUCCESS);
}
