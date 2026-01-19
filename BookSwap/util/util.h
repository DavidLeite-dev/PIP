#ifndef ESSENCIAIS_H
#define ESSENCIAIS_H

#include <stddef.h>
// Constantes
#define MAX_NOME 50
#define MAX_EMAIL 50
#define MAX_PASSWORD 20
#define MAX_TITULO 100
#define MAX_AUTOR 100
#define MAX_OWNER 100
#define MAX_CATEGORIA 50
#define MAX_CONDICAO 20
#define MAX_DISPONIVEL 2
#define CONDICAO_NOVO 1
#define CONDICAO_BOM 2
#define CONDICAO_RAZOAVEL 3
#define ARQUIVO_AUTORES "autores.txt"
#define ARQUIVO_CATEGORIAS "categorias.txt"

// Funções utilitárias
void limparBuffer(void);
void limparEcra(void);
void pausar(void);
void opcaoInvalida(void);
void linha_h_topo(void);
void linha_h_meio(void);
void linha_h_fim(void);
void linha_vazia(void);
void texto_centrado(const char *formato, ...);
void texto_esquerda(const char *formato, ...);
void garantir_utf8(char *texto, size_t tamanho);

// Validação de input
int contem_ponto_virgula(const char *texto);
void aviso_ponto_virgula_nao_permitido(void);
int ler_linha_sem_ponto_virgula(char *destino, size_t destinoTamanho);
int ler_char_sem_ponto_virgula(char *destino);

// Leitura segura de números (evita problemas de scanf)
int ler_int_intervalo(int *destino, int min, int max);
int ler_float_intervalo(float *destino, float min, float max);
typedef int (*MenuItemFormatter)(char *buffer, size_t bufferSize, int index, void *item, void *context);
typedef void (*MenuPageHeader)(int paginaAtual, int totalPaginas, void *context);
int menu_paginar_itens(void *itens, int total, int porPagina, size_t itemSize, MenuItemFormatter formatter, MenuPageHeader header, void *context);
void fpopeec(int linhas);
const char *traduzir_condicao(int condicao);
int obter_id_autor(const char *autor);
const char *traduzir_autor(int id);
int obter_id_categoria(const char *categoria);
const char *traduzir_categoria(int id);

// Verificação mínima de requisitos para correr o programa.
// Retorna 1 se estiver OK; se não estiver OK, mostra uma mensagem de erro e retorna 0.
int verificarRequisitosAmbiente(void);

#endif /* ESSENCIAIS_H */


