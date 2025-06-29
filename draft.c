

int* dados;
int* cache;
int n;  // numero de nós
int k;  // numero de blocos por nó
int t;  // tamanho de bloco
int r;  // rank do nó

int main() {
  dados = malloc(k * sizeof(int));
  // cria um socket e fica aguardando conexões
}

int le(int pos, int* buffer, int len) {
  while (len > 0) {
    leBloco(pos, buffer, min(len, t));
    pos += 1;
    len -= t;
  }
  return 0;
}

int leBloco(int pos, int* buffer, int len) {
  int noDestino = pos / k;
  if (noDestino == r) {
    escreveBuffer(dados[pos], buffer, len);
    return 0;
  } else if (noDestino >= n) {
    return 1;  // pos > endereço máximo
  } else {
    int status = leRede(pos, buffer, len, noDestino);
    return status;
  }
}

int leRede(int pos, int* buffer, int len, int noDestino) {
  // pega endereço do nó destino
  // cria socket e conecta
  // envia `FETCH pos`
  // recebe o conteúdo do bloco `pos`
  // pega os primeiros `len` bytes e joga eles no buffer
}