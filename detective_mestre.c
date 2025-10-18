#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/*
  Detective Quest — Nível Mestre
  - Mansão fixa (árvore binária) para exploração.
  - BST de pistas (strings) para listagem/busca em ordem.
  - Hash de suspeitos: cada suspeito aponta para lista encadeada de pistas.
  - Menu:
      1 Explorar mansão (coleta automática de pistas + associação a suspeitos)
      2 Listar pistas em ordem alfabética (BST)
      3 Buscar pista pelo texto (BST)
      4 Listar suspeitos -> pistas (Hash)
      5 Mostrar suspeito mais citado (Hash)
      0 Sair
*/

/* =================== ÁRVORE DE CÔMODOS =================== */
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

    conectar(hall, biblioteca, salaEstar);
    conectar(biblioteca, sotao, cozinha);
    conectar(salaEstar,  jardim,  porao);

    *hallOut = hall;
}
static void liberarArvore(Sala *n){
    if(!n) return; liberarArvore(n->esq); liberarArvore(n->dir); free(n);
}

/* =================== BST DE PISTAS =================== */
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
    return r; /* ignora duplicata */
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
    emOrdem(r->esq);
    printf("- %s\n", r->texto);
    emOrdem(r->dir);
}
static void liberarBST(NoPista *r){
    if(!r) return;
    liberarBST(r->esq); liberarBST(r->dir); free(r);
}

/* =================== HASH DE SUSPEITOS =================== */
typedef struct PistaNode {
    char texto[64];
    struct PistaNode *next;
} PistaNode;

typedef struct SusNode {
    char nome[64];
    int total;             /* quantidade de pistas associadas */
    PistaNode *pistas;     /* lista de pistas desse suspeito */
    struct SusNode *next;  /* próximo na cadeia do bucket */
} SusNode;

#define HSIZE 23
typedef struct {
    SusNode *bucket[HSIZE];
} HashSus;

static unsigned hashStr(const char *s){
    unsigned sum = 0;
    for(; *s; ++s) sum += (unsigned)(unsigned char)*s;
    return sum % HSIZE;
}

static void hashInit(HashSus *h){ for(int i=0;i<HSIZE;i++) h->bucket[i]=NULL; }

/* procura suspeito no bucket; cria se não existir */
static SusNode* getOrCreateSus(HashSus *h, const char *nome){
    unsigned k = hashStr(nome);
    SusNode *p = h->bucket[k];
    while(p){ if(strcmp(p->nome,nome)==0) return p; p = p->next; }
    SusNode *novo = (SusNode*)malloc(sizeof(SusNode));
    if(!novo){ perror("malloc"); exit(EXIT_FAILURE); }
    strncpy(novo->nome,nome,sizeof(novo->nome)-1);
    novo->nome[sizeof(novo->nome)-1]='\0';
    novo->total = 0;
    novo->pistas = NULL;
    novo->next = h->bucket[k];
    h->bucket[k] = novo;
    return novo;
}

/* evita duplicar a mesma pista para o mesmo suspeito */
static bool pistaJaExiste(PistaNode *lst, const char *t){
    for(; lst; lst=lst->next) if(strcmp(lst->texto,t)==0) return true;
    return false;
}

/* API pedida: relaciona pista -> suspeito */
static void inserirNaHash(HashSus *h, const char *pista, const char *suspeito){
    SusNode *s = getOrCreateSus(h, suspeito);
    if(!pistaJaExiste(s->pistas, pista)){
        PistaNode *n = (PistaNode*)malloc(sizeof(PistaNode));
        if(!n){ perror("malloc"); exit(EXIT_FAILURE); }
        strncpy(n->texto,pista,sizeof(n->texto)-1);
        n->texto[sizeof(n->texto)-1]='\0';
        n->next = s->pistas;
        s->pistas = n;
        s->total++;
    }
}

static void listarAssociacoes(const HashSus *h){
    printf("\nSuspeitos e pistas associadas:\n");
    for(int i=0;i<HSIZE;i++){
        for(SusNode *s=h->bucket[i]; s; s=s->next){
            printf("Suspeito: %s  (total=%d)\n", s->nome, s->total);
            if(!s->pistas){ printf("  - nenhuma pista\n"); continue; }
            for(PistaNode *p=s->pistas; p; p=p->next)
                printf("  - %s\n", p->texto);
        }
    }
}

static void suspeitoMaisCitado(const HashSus *h){
    const SusNode *best = NULL;
    for(int i=0;i<HSIZE;i++)
        for(SusNode *s=h->bucket[i]; s; s=s->next)
            if(!best || s->total > best->total) best = s;
    if(best) printf("\nSuspeito mais citado: %s (pistas=%d)\n", best->nome, best->total);
    else     printf("\nNenhuma associação registrada.\n");
}

static void hashFree(HashSus *h){
    for(int i=0;i<HSIZE;i++){
        SusNode *s=h->bucket[i];
        while(s){
            PistaNode *p=s->pistas;
            while(p){ PistaNode *tmp=p->next; free(p); p=tmp; }
            SusNode *nx=s->next; free(s); s=nx;
        }
        h->bucket[i]=NULL;
    }
}

/* =================== COLETA: SALA -> PISTA + SUSPEITO =================== */
static void coletarEm(Sala *s, NoPista **bst, HashSus *hash){
    if(!s) return;
    if(strcmp(s->nome,"Biblioteca")==0){
        *bst = inserirBST(*bst, "Livro rasgado");
        inserirNaHash(hash, "Livro rasgado", "Sr. Silva");
    } else if(strcmp(s->nome,"Cozinha")==0){
        *bst = inserirBST(*bst, "Faca com marcas");
        inserirNaHash(hash, "Faca com marcas", "Cozinheiro");
    } else if(strcmp(s->nome,"Jardim")==0){
        *bst = inserirBST(*bst, "Pegadas na terra");
        inserirNaHash(hash, "Pegadas na terra", "Jardineiro");
    } else if(strcmp(s->nome,"Porao")==0){
        *bst = inserirBST(*bst, "Cofre danificado");
        inserirNaHash(hash, "Cofre danificado", "Sra. Costa");
    } else if(strcmp(s->nome,"Sotao")==0){
        *bst = inserirBST(*bst, "Carta antiga");
        inserirNaHash(hash, "Carta antiga", "Sr. Silva");
    }
}

/* =================== NAVEGAÇÃO =================== */
static void explorar(Sala *raiz, NoPista **bst, HashSus *hash){
    if(!raiz) return;
    Sala *atual = raiz;
    printf("Exploracao iniciada em: %s\n", atual->nome);
    coletarEm(atual, bst, hash);
    while(true){
        printf("\nVoce esta em: %s\n", atual->nome);
        bool folha = (!atual->esq && !atual->dir);
        if(folha){ printf("Fim do caminho.\n"); break; }

        printf("Opcoes: [e] esquerda%s  [d] direita%s  [s] sair\n",
               atual->esq? "" : " (ind.)", atual->dir? "" : " (ind.)");
        printf("Escolha: ");
        int c=getchar(); int ch; while((ch=getchar())!='\n' && ch!=EOF){}
        if(c=='s'||c=='S'){ printf("Exploracao encerrada.\n"); break; }
        else if((c=='e'||c=='E') && atual->esq){ atual=atual->esq; coletarEm(atual, bst, hash); }
        else if((c=='d'||c=='D') && atual->dir){ atual=atual->dir; coletarEm(atual, bst, hash); }
        else printf("Movimento invalido.\n");
    }
}

/* =================== MENU =================== */
static void menu(void){
    printf("\n=== DETECTIVE QUEST — NIVEL MESTRE ===\n");
    printf("1 - Explorar mansao (coleta pistas + associa suspeitos)\n");
    printf("2 - Listar pistas em ordem alfabetica\n");
    printf("3 - Buscar pista pelo texto\n");
    printf("4 - Listar suspeitos e pistas relacionadas\n");
    printf("5 - Mostrar suspeito mais citado\n");
    printf("0 - Sair\n");
    printf("Escolha: ");
}

/* =================== MAIN =================== */
int main(void){
    Sala *hall=NULL; montarMansao(&hall);
    NoPista *bst=NULL;
    HashSus hash; hashInit(&hash);

    int op;
    do{
        menu();
        if(scanf("%d",&op)!=1){ op=-1; }
        int ch; while((ch=getchar())!='\n' && ch!=EOF){}

        switch(op){
            case 1: explorar(hall, &bst, &hash); break;
            case 2:
                if(!bst) printf("Nenhuma pista coletada.\n");
                else { printf("\nPistas (ordem alfabetica):\n"); emOrdem(bst); }
                break;
            case 3: {
                char buf[64];
                printf("Texto da pista: ");
                if(!fgets(buf,sizeof(buf),stdin)) break;
                size_t n=strlen(buf); if(n&&buf[n-1]=='\n') buf[n-1]='\0';
                printf("%s\n", buscaBST(bst, buf)? "Encontrada." : "Nao encontrada.");
            } break;
            case 4: listarAssociacoes(&hash); break;
            case 5: suspeitoMaisCitado(&hash); break;
            case 0: printf("Saindo.\n"); break;
            default: printf("Opcao invalida.\n");
        }
    }while(op!=0);

    liberarArvore(hall);
    liberarBST(bst);
    hashFree(&hash);
    return 0;
}
