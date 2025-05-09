# 🖥️ Simulador de Núcleo de Sistema Operacional (Bianrri)

Este projeto é um simulador de um núcleo de Sistema Operacional, implementado em linguagem C. Ele integra conceitos importantes como gerenciamento de processos, escalonamento, sincronização com semáforos, controle de E/S, gerenciamento de memória com segmentação e paginação, além de uma interface interativa via terminal.

---

## 📁 Estrutura do Projeto

📂 so/

├── so.h              # Header principal com definições e structs do kernel

└── so.c              # Implementação das funções principais do sistema

📂 terminal/

├── terminal.h        # Header com definições de cor e funções auxiliares

└── terminal.c        # Interface de interação com o usuário via terminal

---

## 📚 Bibliotecas Utilizadas

* `pthread.h` – Threads POSIX
* `semaphore.h` – Controle de concorrência
* `stdio.h`, `stdlib.h`, `math.h` – Funções padrão
* `unistd.h`, `string.h` – Utilitários diversos

---

## 🛠 Funcionalidades

### 🧵 Threads e Concorrência
- Threads para simular execução paralela de processos, acesso a disco, impressão e carregamento de memória;
- Sincronização com mutexes e semáforos (`pthreads`, `semaphore.h`).

### 🧩 Gerenciamento de Processos
- Criação e execução de processos sintéticos via arquivos de entrada;
- Estados: `NOVO`, `PRONTO`, `EXECUTANDO`, `BLOQUEADO`, `CONCLUÍDO`;
- Gerenciamento por PCB e escalonamento circular com prioridades.

### ⚙️ Escalonador Round-Robin com Prioridades
- Quantum proporcional à prioridade (`quantum = 5000 / prioridade`);
- Suporte a interrupções por fim de quantum, E/S e término de processo.

### 🧠 Gerenciamento de Memória
- Memória total: 1GB, com páginas de 8KB;
- Segmentação com alocação por processo;
- Paginação com algoritmo de substituição **Second Chance**;
- Controle de uso e liberação de páginas.

### 💾 Simulação de Disco
- Agendador de disco com movimentação de trilha;
- Operações de leitura e escrita em trilha com controle temporal.

### 🖨 Fila de Impressão
- Simulação de operação de impressão com buffer limitado;
- Visualização da fila de impressões realizadas.

### 🔐 Semáforos
- Suporte a operações `P(s)` e `V(s)` com bloqueio/desbloqueio de processos;
- Cada processo pode declarar seus próprios semáforos.

---

## 📄 Exemplo de Programa Sintético

```txt
programa1 0 1 64 S1 S2
exec 1000
P(S1)
read 0
V(S1)
exec 2000
print 500
````

* **Cabeçalho:** nome, segmento, prioridade, tamanho, semáforos.
* **Comandos suportados:** `exec`, `read`, `write`, `print`, `P(S)`, `V(S)`.

---

## ▶️ Como Compilar e Executar

1. **Compilar:**

```bash
gcc -o simulador terminal/terminal.c so/so.c -lpthread
```

2. **Executar:**

```bash
./simulador
```

3. **Navegar pelo menu no terminal:**

```
1 - Adicionar processo
2 - Ver processos em execução
3 - Ver estado dos processos
4 - Ver memória
5 - Ver fila de impressão
0 - Sair
```

---

## 👨‍💻 Autores

Bianca Lançoni & Lucas Furriel
