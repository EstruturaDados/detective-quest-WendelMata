#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/*
  Detective Quest — Nível Novato
  Árvore binária fixa de cômodos.
  Exploração interativa: esquerda (e), direita (d), sair (s).
*/

typedef struct Sala {
    char nome[64];
    struct Sala *esq;
    struct Sala *dir;
} Sala;

/* cria dinamicamente uma sala com nome */
static Sala* criarSala(const char *nome) {
    Sala *s = (Sala*)malloc(sizeof(Sala));
    if (!s) { perror("malloc"); exit(EXIT_FAILURE); }
    strncpy(s->nome, nome, sizeof(s->nome)-1);
    s->nome[sizeof(s->nome)-1] = '\0';
    s->esq = s->dir = NULL;
    return s;
}

/* conecta filhos: atual->esq, atual->dir */
static void conectarSalas(Sala *atual, Sala *esq, Sala *dir) {
    if (atual) { atual->esq = esq; atual->dir = dir; }
}

/* exibe caminho e permite navegação até nó-folha ou sair */
static void explorarSalas(Sala *raiz) {
    if (!raiz) return;
    Sala *atual = raiz;
    printf("Exploracao iniciada a partir de: %s\n", atual->nome);
    while (true) {
        printf("\nVoce esta em: %s\n", atual->nome);
        bool folha = (atual->esq == NULL && atual->dir == NULL);
        if (folha) {
            printf("Fim do caminho. Voce alcancou um comodo sem saidas.\n");
            break;
        }
        printf("Opcoes: [e] esquerda%s  [d] direita%s  [s] sair\n",
               atual->esq ? "" : " (indisponivel)",
               atual->dir ? "" : " (indisponivel)");
        printf("Escolha: ");
        int c = getchar();
        // consumir \n
        int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}

        if (c == 's' || c == 'S') {
            printf("Exploracao encerrada.\n");
            break;
        } else if ((c == 'e' || c == 'E') && atual->esq) {
            atual = atual->esq;
        } else if ((c == 'd' || c == 'D') && atual->dir) {
            atual = atual->dir;
        } else {
            printf("Movimento invalido.\n");
        }
    }
}

/* desaloca toda a arvore (pos-ordem) */
static void liberarArvore(Sala *no) {
    if (!no) return;
    liberarArvore(no->esq);
    liberarArvore(no->dir);
    free(no);
}

/* monta a mansao fixa e inicia a exploracao */
int main(void) {
    // Nomes de exemplo. Ajuste se desejar.
    Sala *hall       = criarSala("Hall de Entrada");
    Sala *biblioteca = criarSala("Biblioteca");
    Sala *salaEstar  = criarSala("Sala de Estar");
    Sala *cozinha    = criarSala("Cozinha");
    Sala *jardim     = criarSala("Jardim");
    Sala *sotao      = criarSala("Sotao");
    Sala *porao      = criarSala("Porao");

    // Estrutura (exemplo):
    //               Hall
    //          /              \
    //     Biblioteca        Sala de Estar
    //     /       \          /           \
    //  Sotao    Cozinha   Jardim        Porao

    conectarSalas(hall, biblioteca, salaEstar);
    conectarSalas(biblioteca, sotao, cozinha);
    conectarSalas(salaEstar, jardim, porao);

    explorarSalas(hall);

    liberarArvore(hall);
    return 0;
}
