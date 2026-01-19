#ifndef TRANSACOES_H
#define TRANSACOES_H

#include <util.h>

#define MAX_STATUS 20
#define MAX_DATA 20

typedef struct {
    int id;
    char titulo[MAX_TITULO];
    char autor[MAX_AUTOR];
    char requester[MAX_NOME];
    char owner[MAX_NOME];
    char status[MAX_STATUS];
    char data[MAX_DATA];
} Requisicao;

typedef struct {
    int id;
    char titulo[MAX_TITULO];
    char autor[MAX_AUTOR];
    char comprador[MAX_NOME];
    char vendedor[MAX_NOME];
    float preco;
    char data[MAX_DATA];
    char status[MAX_STATUS];
} Compra;

typedef struct {
    int id;
    char titulo1[MAX_TITULO];
    char autor1[MAX_AUTOR];
    char titulo2[MAX_TITULO];
    char autor2[MAX_AUTOR];
    char user1[MAX_NOME];
    char user2[MAX_NOME];
    char status[MAX_STATUS];
    char data[MAX_DATA];
} Troca;

// Funções de requisição
void requisitarLivroEspecifico(char *titulo, char *autor, char *owner);
void consultarMeusPedidos(void);

// Funções de compra
void comprarLivroEspecifico(char *titulo, char *autor, char *vendedor);
void consultarCompras(void);

// Funções de troca
void trocarLivroEspecifico(char *titulo, char *autor, char *owner);

// Submenu "Transações" no menu principal
void menuTransacoesUsuario(void);

// Funções auxiliares
void obterDataAtual(char *data);
int obterProximoIdRequisicao(void);
int obterProximoIdCompra(void);
int obterProximoIdTroca(void);

#endif /* TRANSACOES_H */
