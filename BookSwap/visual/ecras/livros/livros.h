#ifndef LIVROS_H
#define LIVROS_H

#include <util.h>

typedef struct {
    char titulo[MAX_TITULO];
    int autor_id;
    char autor[MAX_AUTOR];
    char owner[MAX_OWNER];
    int categoria_id;
    char categoria[MAX_CATEGORIA]; // deixar admin criar categorias
    int condicao_id;
    char condicao[MAX_CONDICAO];
    int disponivel;  // 1 = disponível, 0 = indisponível
} Livro;

void adicionarLivro(const char *nome_user);
void consultarLivros(const char *nome_user);
void pesquisarLivros(const char *nome_user);
void listarMeusLivros(const char *nome_user);
void mostrarLivros(const char *nome_user, Livro *livros, int total);
int carregar_livro_de_linha(const char *linha, Livro *livro);

// Funções auxiliares para gestão de livros
void trocarPosseLivro(char *titulo, char *autor, char *antigoOwner, char *novoOwner);
void atualizarDisponibilidadeLivro(char *titulo, char *autor, char *owner, int disponivel);
int verificarDisponibilidadeLivro(char *titulo, char *autor, char *owner);
void menuAdminLivros(void);

#endif /* LIVROS_H */