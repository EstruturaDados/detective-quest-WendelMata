#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/*
  Detective Quest — Nível Aventureiro
  Mansão fixa (árvore binária) + BST de pistas (strings).
  Inserção de pistas ocorre ao visitar salas específicas.
  Opções: 1 explorar, 2 listar pistas, 0 sair.
*/

/* ---------- árvore de cômodos ---------- */
typedef struct Sala {
    char nome[64];
    struct Sala *esq, *dir;
} Sala;

static Sala* criarSala(const char *nome){
    Sala *s = (Sala*)malloc(sizeof(Sala));
    if(!s){ perror("malloc"); exit(EXIT_FAILURE); }
    strncpy(s->nome, nome, sizeof(s->nome)-1);
    s->nome[sizeof(s->nome)-1] = '\0';
    s->esq = s->dir = NULL;
    return s;
}
static void conectar(Sala *p, Sala *e, Sala *d){ if(p){ p->esq=e; p->dir=d; } }

/* ---------- BST de pistas ---------- */
typedef struct NoPista {
    char texto[64];
    struct NoPista *esq, *dir;
} NoPista;

static NoPista* novoNo(const char *t){
    NoPista *n = (NoPista*)malloc(sizeof(NoPista));
    if(!n){ perror("malloc"); exit(EXIT_FAILURE); }
    strncpy(n->texto, t, sizeof(n->texto)-1);
    n->texto[sizeof(n->texto)-1] = '\0';
    n->esq = n->dir = NULL;
    return n;
}

/* inserção em BST de strings */
static NoPista* inserirBST(NoPista *raiz, const char *t){
    if(!raiz) return novoNo(t);
    int cmp = strcmp(t, raiz->texto);
    if(cmp < 0) raiz->esq = inserirBST(raiz->esq, t);
    else if(cmp > 0) raiz->dir = inserirBST(raiz->dir, t);
    /* iguais: ignora duplicata */
    return raiz;
}

static bool buscaBST(NoPista *r, const char *t){
    while(r){
        int c = strcmp(t, r->texto);
        if(c==0) return true;
        r = (c<0)? r->esq : r->dir;
    }
    return false;
}

/* em-ordem: imprime em ordem alfabética */
static void emOrdem(NoPista *r){
    if(!r) return;
    emOrdem(r->esq);
    printf("- %s\n", r->texto);
    emOrdem(r->dir);
}

static void liberarBST(NoPista *r){
    if(!r) return;
    liberarBST(r->esq); liberarBST(r->dir); free(r);
}

/* ---------- lógica de exploração + coleta ---------- */

/* adiciona pistas por sala; personalize à vontade */
static void coletarPistasEm(Sala *s, NoPista **bst){
    if(!s) return;
    if(strcmp(s->nome,"Biblioteca")==0)
        *bst = inserirBST(*bst, "Livro rasgado");
    else if(strcmp(s->nome,"Cozinha")==0)
        *bst = inserirBST(*bst, "Faca com marcas");
    else if(strcmp(s->nome,"Jardim")==0)
        *bst = inserirBST(*bst, "Pegadas na terra");
    else if(strcmp(s->nome,"Porao")==0)
        *bst = inserirBST(*bst, "Cofre danificado");
    else if(strcmp(s->nome,"Sotao")==0)
        *bst = inserirBST(*bst, "Carta antiga");
}

/* navegação interativa; coleta automática ao entrar na sala */
static void explorarSalas(Sala *raiz, NoPista **bst){
    if(!raiz) return;
    Sala *atual = raiz;
    printf("Exploracao iniciada em: %s\n", atual->nome);
    coletarPistasEm(atual, bst);
    while(true){
        printf("\nVoce esta em: %s\n", atual->nome);
        bool folha = (!atual->esq && !atual->dir);
        if(folha){ printf("Fim do caminho.\n"); break; }

        printf("Opcoes: [e] esquerda%s  [d] direita%s  [s] sair\n",
               atual->esq? "" : " (ind.)", atual->dir? "" : " (ind.)");
        printf("Escolha: ");
        int c = getchar(); int ch; while((ch=getchar())!='\n' && ch!=EOF){}

        if(c=='s'||c=='S'){ printf("Exploracao encerrada.\n"); break; }
        else if((c=='e'||c=='E') && atual->esq){ atual = atual->esq; coletarPistasEm(atual, bst); }
        else if((c=='d'||c=='D') && atual->dir){ atual = atual->dir; coletarPistasEm(atual, bst); }
        else { printf("Movimento invalido.\n"); }
    }
}

/* ---------- montagem e menu ---------- */
static void montarMansao(Sala **hallOut){
    Sala *hall       = criarSala("Hall de Entrada");
    Sala *biblioteca = criarSala("Biblioteca");
    Sala *salaEstar  = criarSala("Sala de Estar");
    Sala *cozinha    = criarSala("Cozinha");
    Sala *jardim     = criarSala("Jardim");
    Sala *sotao      = criarSala("Sotao");
    Sala *porao      = criarSala("Porao");

    conectar(hall, biblioteca, salaEstar);
    conectar(biblioteca, sotao, cozinha);
    conectar(salaEstar,  jardim,  porao);

    *hallOut = hall;
}

static void liberarArvore(Sala *n){
    if(!n) return; liberarArvore(n->esq); liberarArvore(n->dir); free(n);
}

static void menu(void){
    printf("\n=== DETECTIVE QUEST — NIVEL AVENTUREIRO ===\n");
    printf("1 - Explorar mansao (coleta automatica de pistas)\n");
    printf("2 - Listar pistas em ordem alfabetica\n");
    printf("3 - Buscar pista pelo texto\n");
    printf("0 - Sair\n");
    printf("Escolha: ");
}

int main(void){
    Sala *hall = NULL; montarMansao(&hall);
    NoPista *bst = NULL;

    int op;
    do{
        menu();
        if(scanf("%d",&op)!=1){ op=-1; }
        int ch; while((ch=getchar())!='\n' && ch!=EOF){}

        switch(op){
            case 1: explorarSalas(hall, &bst); break;
            case 2:
                if(!bst) printf("Nenhuma pista coletada.\n");
                else { printf("\nPistas coletadas (ordem alfabetica):\n"); emOrdem(bst); }
                break;
            case 3: {
                char buf[64];
                printf("Texto da pista: ");
                if(!fgets(buf,sizeof(buf),stdin)) break;
                size_t n=strlen(buf); if(n&&buf[n-1]=='\n') buf[n-1]='\0';
                printf("%s\n", buscaBST(bst, buf)? "Encontrada." : "Nao encontrada.");
            } break;
            case 0: printf("Saindo.\n"); break;
            default: printf("Opcao invalida.\n");
        }
    }while(op!=0);

    liberarArvore(hall);
    liberarBST(bst);
    return 0;
}