# Memocomdist

Programa que simula um sistema de memória compartilhada distribuída, com a criação de um espaço de endereçamento dado em blocos distribuido entre processos.

## Compilação

Para compilar a aplicação, utilize o seguinte comando na raiz do projeto:

```bash
make all
```

## Como Executar

Execute o programa da seguinte forma:

```bash
./memcomdist -p <numero_processos> -b <numero_blocos> -t <tamanho_blocos>
```

- `<numero_processos>`: Número de processos do sistema que compartilham a memória.
- `<numero_blocos>`: Número total de blocos da memória, repartido entre os processos.
- `<tamanho_blocos>`: Tamanho em bytes dos blocos.

### Exemplo:

```bash
./memcomdist -p 4 -b 12 -t 3
```

## Utilização

A utilização do protótipo de memória compartilhada distribuída pode seguir de duas formas:

- Conexão com um dos nós (processos) do grupo usando sua porta e comunicação com o sistema através do protocolo definido.
- Uso da API presente no arquivo `api.h` em um código `.c`.

### Processos

Se conectar a um processo é possível usando sua porta, as portas vão de `50000` até `50000 + (numero_processos - 1)`.

Exemplo:
```bash
telnet 127.0.0.1 50002
```

O comando acima se conecta ao terceiro processo.

### Protocolo

Os métodos principais para uso do sistema são:

- `FETCH <posicao> <num_bytes>`
- `STORE <posicao> <num_bytes> <dados>`

Onde o primeiro serve para obter `<num_bytes>` bytes a partir de uma posição `<posicao>`, e o outro serve para salvar `<dados>` nesses bytes.

Existem outros métodos no protocolo mas são para uso interno na comunicação dos processos.

## Observações

- Aplicação desenvolvida para sistemas baseados em Linux.
