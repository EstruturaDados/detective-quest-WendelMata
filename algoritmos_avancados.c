#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/*
  Detective Quest — arquivo único (algoritmos-avançados.c)

  BLOCO 1 — NÍVEL NOVATO (Mapa da mansão com árvore binária fixa)
    - struct Sala { nome, esq, dir }
    - montar mansão de forma estática
    - explorar interativamente: esquerda (e), direita (d), sair (s)

  BLOCO 2 — NÍVEL AVENTUREIRO (BST de pistas)
    - BST de strings para armazenar pistas coletadas ao entrar em certas salas
    - inserir, buscar e listar em ordem alfabética

  BLOCO 3 — NÍVEL MESTRE (Tabela hash de suspeitos)
    - hash que mapeia suspeito -> lista encadeada de pistas
    - registrar associações pista->suspeito durante a exploração
    - listar associações e mostrar o suspeito mais citado
*/

/* ======================== ÁRVORE DE CÔMODOS ======================== */
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

static void montarMansao(Sala **hallOut){
    Sala *hall       = criarSala("Hall de Entrada");
    Sala *biblioteca = criarSala("Biblioteca");
    Sala *salaEstar  = criarSala("Sala de Estar");
    Sala *cozinha    = criarSala("Cozinha");
    Sala *jardim     = criarSala("Jardim");
    Sala *sotao      = criarSala("Sotao");
    Sala *porao      = criarSala("Porao");

    //            Hall
    //      /                \
    // Biblioteca         Sala de Estar
    //  /     \            /          \
    // Sotao  Cozinha   Jardim        Porao
    conectar(hall, biblioteca, salaEstar);
    conectar(biblioteca, sotao, cozinha);
    conectar(salaEstar,  jardim,  porao);

    *hallOut = hall;
}

static void liberarArvore(Sala *n){
    if(!n) return; liberarArvore(n->esq); liberarArvore(n->dir); free(n);
}

/* ======================== BST DE PISTAS (strings) ======================== */
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
static NoPista* inserirBST(NoPista *r, const char *t){
    if(!r) return novoNo(t);
    int c = strcmp(t, r->texto);
    if(c < 0) r->esq = inserirBST(r->esq, t);
    else if(c > 0) r->dir = inserirBST(r->dir, t);
    return r; // ignora duplicatas
}
static bool buscaBST(NoPista *r, const char *t){
    while(r){
        int c = strcmp(t, r->texto);
        if(c==0) return true;
        r = (c<0)? r->esq : r->dir;
    }
    return false;
}
static void emOrdem(NoPista *r){
    if(!r) return;
    emOrdem(r->esq); printf("- %s\n", r->texto); emOrdem(r->dir);
}
static void liberarBST(NoPista *r){
    if(!r) return; liberarBST(r->esq); liberarBST(r->dir); free(r);
}

/* ======================== HASH DE SUSPEITOS ======================== */
typedef struct PistaNode {
    char texto[64];
    struct PistaNode *next;
} PistaNode;

typedef struct SusNode {
    char nome[64];
    int total;             // número de pistas associadas
    PistaNode *pistas;     // lista de pistas desse suspeito
    struct SusNode *next;  // encadeamento do bucket
} SusNode;

#define HSIZE 23
typedef struct { SusNode *bucket[HSIZE]; } HashSus;

static unsigned hashStr(const char *s){ unsigned sum=0; for(;*s;++s) sum+=(unsigned char)*s; return sum%HSIZE; }
static void hashInit(HashSus *h){ for(int i=0;i<HSIZE;i++) h->bucket[i]=NULL; }

static SusNode* getOrCreateSus(HashSus *h, const char *nome){
    unsigned k=hashStr(nome); SusNode *p=h->bucket[k];
    while(p){ if(strcmp(p->nome,nome)==0) return p; p=p->next; }
    SusNode *n=(SusNode*)malloc(sizeof(SusNode)); if(!n){ perror("malloc"); exit(EXIT_FAILURE); }
    strncpy(n->nome,nome,sizeof(n->nome)-1); n->nome[sizeof(n->nome)-1]='\0';
    n->total=0; n->pistas=NULL; n->next=h->bucket[k]; h->bucket[k]=n; return n;
}
static bool pistaJaExiste(PistaNode *lst, const char *t){ for(;lst;lst=lst->next) if(strcmp(lst->texto,t)==0) return true; return false; }
static void inserirNaHash(HashSus *h, const char *pista, const char *suspeito){
    SusNode *s=getOrCreateSus(h,suspeito);
    if(!pistaJaExiste(s->pistas,pista)){
        PistaNode *n=(PistaNode*)malloc(sizeof(PistaNode)); if(!n){ perror("malloc"); exit(EXIT_FAILURE); }
        strncpy(n->texto,pista,sizeof(n->texto)-1); n->texto[sizeof(n->texto)-1]='\0';
        n->next=s->pistas; s->pistas=n; s->total++;
    }
}
static void listarAssociacoes(const HashSus *h){
    printf("\nSuspeitos e pistas associadas:\n");
    for(int i=0;i<HSIZE;i++)
        for(SusNode *s=h->bucket[i]; s; s=s->next){
            printf("Suspeito: %s (total=%d)\n", s->nome, s->total);
            if(!s->pistas) { printf("  - nenhuma pista\n"); continue; }
            for(PistaNode *p=s->pistas; p; p=p->next) printf("  - %s\n", p->texto);
        }
}
static void suspeitoMaisCitado(const HashSus *h){
    const SusNode *best=NULL;
    for(int i=0;i<HSIZE;i++) for(SusNode *s=h->bucket[i]; s; s=s->next)
        if(!best || s->total>best->total) best=s;
    if(best) printf("\nSuspeito mais citado: %s (pistas=%d)\n", best->nome, best->total);
    else     printf("\nNenhuma associacao registrada.\n");
}
static void hashFree(HashSus *h){
    for(int i=0;i<HSIZE;i++){
        SusNode *s=h->bucket[i];
        while(s){ PistaNode *p=s->pistas; while(p){ PistaNode *nx=p->next; free(p); p=nx; }
                  SusNode *sn=s->next; free(s); s=sn; }
        h->bucket[i]=NULL;
    }
}

/* ======================== COLETA POR SALA ======================== */
static void coletarPistasPorSala(Sala *s, NoPista **bst, HashSus *hash, bool mestre){
    if(!s) return;
    if(strcmp(s->nome,"Biblioteca")==0){
        *bst = inserirBST(*bst, "Livro rasgado");
        if(mestre) inserirNaHash(hash,"Livro rasgado","Sr. Silva");
    } else if(strcmp(s->nome,"Cozinha")==0){
        *bst = inserirBST(*bst, "Faca com marcas");
        if(mestre) inserirNaHash(hash,"Faca com marcas","Cozinheiro");
    } else if(strcmp(s->nome,"Jardim")==0){
        *bst = inserirBST(*bst, "Pegadas na terra");
        if(mestre) inserirNaHash(hash,"Pegadas na terra","Jardineiro");
    } else if(strcmp(s->nome,"Porao")==0){
        *bst = inserirBST(*bst, "Cofre danificado");
        if(mestre) inserirNaHash(hash,"Cofre danificado","Sra. Costa");
    } else if(strcmp(s->nome,"Sotao")==0){
        *bst = inserirBST(*bst, "Carta antiga");
        if(mestre) inserirNaHash(hash,"Carta antiga","Sr. Silva");
    }
}

/* ======================== EXPLORAÇÃO ======================== */
static void explorarNovato(Sala *raiz){
    if(!raiz) return;
    Sala *atual=raiz; printf("Exploracao iniciada em: %s\n", atual->nome);
    while(true){
        printf("\nVoce esta em: %s\n", atual->nome);
        bool folha = (!atual->esq && !atual->dir);
        if(folha){ printf("Fim do caminho.\n"); break; }
        printf("Opcoes: [e] esquerda%s  [d] direita%s  [s] sair\n",
               atual->esq? "" : " (ind.)", atual->dir? "" : " (ind.)");
        printf("Escolha: ");
        int c=getchar(); int ch; while((ch=getchar())!='\n' && ch!=EOF){}
        if(c=='s'||c=='S'){ printf("Exploracao encerrada.\n"); break; }
        else if((c=='e'||c=='E') && atual->esq) atual=atual->esq;
        else if((c=='d'||c=='D') && atual->dir) atual=atual->dir;
        else printf("Movimento invalido.\n");
    }
}
static void explorarAventureiro(Sala *raiz, NoPista **bst){
    if(!raiz) return;
    Sala *atual=raiz; printf("Exploracao com coleta de pistas iniciada em: %s\n", atual->nome);
    coletarPistasPorSala(atual, bst, NULL, false);
    while(true){
        printf("\nVoce esta em: %s\n", atual->nome);
        bool folha = (!atual->esq && !atual->dir);
        if(folha){ printf("Fim do caminho.\n"); break; }
        printf("Opcoes: [e] esquerda%s  [d] direita%s  [s] sair\n",
               atual->esq? "" : " (ind.)", atual->dir? "" : " (ind.)");
        printf("Escolha: ");
        int c=getchar(); int ch; while((ch=getchar())!='\n' && ch!=EOF){}
        if(c=='s'||c=='S'){ printf("Exploracao encerrada.\n"); break; }
        else if((c=='e'||c=='E') && atual->esq){ atual=atual->esq; coletarPistasPorSala(atual, bst, NULL, false); }
        else if((c=='d'||c=='D') && atual->dir){ atual=atual->dir; coletarPistasPorSala(atual, bst, NULL, false); }
        else printf("Movimento invalido.\n");
    }
}
static void explorarMestre(Sala *raiz, NoPista **bst, HashSus *hash){
    if(!raiz) return;
    Sala *atual=raiz; printf("Exploracao com pistas e suspeitos iniciada em: %s\n", atual->nome);
    coletarPistasPorSala(atual, bst, hash, true);
    while(true){
        printf("\nVoce esta em: %s\n", atual->nome);
        bool folha = (!atual->esq && !atual->dir);
        if(folha){ printf("Fim do caminho.\n"); break; }
        printf("Opcoes: [e] esquerda%s  [d] direita%s  [s] sair\n",
               atual->esq? "" : " (ind.)", atual->dir? "" : " (ind.)");
        printf("Escolha: ");
        int c=getchar(); int ch; while((ch=getchar())!='\n' && ch!=EOF){}
        if(c=='s'||c=='S'){ printf("Exploracao encerrada.\n"); break; }
        else if((c=='e'||c=='E') && atual->esq){ atual=atual->esq; coletarPistasPorSala(atual, bst, hash, true); }
        else if((c=='d'||c=='D') && atual->dir){ atual=atual->dir; coletarPistasPorSala(atual, bst, hash, true); }
        else printf("Movimento invalido.\n");
    }
}

/* ======================== MENUS ======================== */
static void menuNovato(void){
    printf("\n=== DETECTIVE QUEST — NIVEL NOVATO ===\n");
    printf("1 - Explorar mansao\n");
    printf("0 - Sair\nEscolha: ");
}
static void menuAventureiro(void){
    printf("\n=== DETECTIVE QUEST — NIVEL AVENTUREIRO ===\n");
    printf("1 - Explorar mansao (coleta de pistas)\n");
    printf("2 - Listar pistas em ordem alfabetica\n");
    printf("3 - Buscar pista pelo texto\n");
    printf("0 - Sair\nEscolha: ");
}
static void menuMestre(void){
    printf("\n=== DETECTIVE QUEST — NIVEL MESTRE ===\n");
    printf("1 - Explorar mansao (pistas + suspeitos)\n");
    printf("2 - Listar pistas em ordem alfabetica\n");
    printf("3 - Buscar pista pelo texto\n");
    printf("4 - Listar suspeitos e pistas relacionadas\n");
    printf("5 - Mostrar suspeito mais citado\n");
    printf("0 - Sair\nEscolha: ");
}

/* ======================== MAIN ======================== */
int main(void){
    Sala *hall=NULL; montarMansao(&hall);
    NoPista *bst=NULL;
    HashSus hash; hashInit(&hash);

    printf("Selecione o nivel (1=Novato, 2=Aventureiro, 3=Mestre): ");
    int nivel=1; if(scanf("%d",&nivel)!=1 || nivel<1 || nivel>3) nivel=1;
    int ch; while((ch=getchar())!='\n' && ch!=EOF){} // limpar linha

    int op;
    do{
        if(nivel==1)      menuNovato();
        else if(nivel==2) menuAventureiro();
        else              menuMestre();

        if(scanf("%d",&op)!=1){ op=-1; }
        while((ch=getchar())!='\n' && ch!=EOF){}

        if(nivel==1){
            switch(op){
                case 1: explorarNovato(hall); break;
                case 0: printf("Saindo.\n");   break;
                default: printf("Opcao invalida.\n");
            }
        } else if(nivel==2){
            switch(op){
                case 1: explorarAventureiro(hall, &bst); break;
                case 2: if(!bst) printf("Nenhuma pista coletada.\n");
                        else { printf("\nPistas (ordem alfabetica):\n"); emOrdem(bst); }
                        break;
                case 3: {
                    char buf[64]; printf("Texto da pista: ");
                    if(!fgets(buf,sizeof(buf),stdin)) break;
                    size_t n=strlen(buf); if(n&&buf[n-1]=='\n') buf[n-1]='\0';
                    printf("%s\n", buscaBST(bst, buf)? "Encontrada." : "Nao encontrada.");
                } break;
                case 0: printf("Saindo.\n"); break;
                default: printf("Opcao invalida.\n");
            }
        } else { // Mestre
            switch(op){
                case 1: explorarMestre(hall, &bst, &hash); break;
                case 2: if(!bst) printf("Nenhuma pista coletada.\n");
                        else { printf("\nPistas (ordem alfabetica):\n"); emOrdem(bst); }
                        break;
                case 3: {
                    char buf[64]; printf("Texto da pista: ");
                    if(!fgets(buf,sizeof(buf),stdin)) break;
                    size_t n=strlen(buf); if(n&&buf[n-1]=='\n') buf[n-1]='\0';
                    printf("%s\n", buscaBST(bst, buf)? "Encontrada." : "Nao encontrada.");
                } break;
                case 4: listarAssociacoes(&hash); break;
                case 5: suspeitoMaisCitado(&hash); break;
                case 0: printf("Saindo.\n"); break;
                default: printf("Opcao invalida.\n");
            }
        }
    }while(op!=0);

    liberarArvore(hall);
    liberarBST(bst);
    hashFree(&hash);
    return 0;
}