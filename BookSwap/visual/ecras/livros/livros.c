#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "util.h"
#include "livros.h"
#include "transacoes.h"
#include "../user/users.h"

#define MAX_SELECAO_OPCOES 200

typedef struct {
    int id;
    char nome[128];
} ItemSelecao;

typedef int (*ObterIdFunc)(const char *);

typedef struct {
    const char *cabecalho;
} ContextoSelecao;

static void truncar_titulo(const char *titulo, char *dest, size_t destSize) {
    if (!dest || destSize == 0) return;
    if (!titulo) {
        dest[0] = '\0';
        return;
    }

    size_t len = strlen(titulo);
    if (len < destSize) {
        strncpy(dest, titulo, destSize - 1);
        dest[destSize - 1] = '\0';
        return;
    }

    if (destSize > 4) {
        strncpy(dest, titulo, destSize - 4);
        dest[destSize - 4] = '\0';
        strcat(dest, "...");
    } else {
        strncpy(dest, titulo, destSize - 1);
        dest[destSize - 1] = '\0';
    }
}

static int formatar_livro_catalogo(char *buffer, size_t bufferSize, int index, void *item, void *context) {
    (void)index;
    (void)context;
    if (!item) {
        if (buffer && bufferSize > 0) buffer[0] = '\0';
        return 0;
    }

    Livro *livro = (Livro *)item;
    char titulo_truncado[45];
    truncar_titulo(livro->titulo, titulo_truncado, sizeof(titulo_truncado));
    snprintf(buffer, bufferSize, "%s", titulo_truncado);
    return 0;
}

static int formatar_livro_usuario(char *buffer, size_t bufferSize, int index, void *item, void *context) {
    (void)index;
    (void)context;
    if (!item) {
        if (buffer && bufferSize > 0) buffer[0] = '\0';
        return 0;
    }

    Livro *livro = (Livro *)item;
    char titulo_truncado[45];
    truncar_titulo(livro->titulo, titulo_truncado, sizeof(titulo_truncado));
    snprintf(buffer, bufferSize, "%s [%s]", titulo_truncado, livro->disponivel ? "Disponível" : "Indisponível");
    return 0;
}

static int formatar_livro_admin(char *buffer, size_t bufferSize, int index, void *item, void *context) {
    (void)index;
    (void)context;
    if (!item) {
        if (buffer && bufferSize > 0) buffer[0] = '\0';
        return 0;
    }

    Livro *livro = (Livro *)item;
    char titulo_truncado[45];
    truncar_titulo(livro->titulo, titulo_truncado, sizeof(titulo_truncado));
    snprintf(buffer, bufferSize, "%s [%s]", titulo_truncado, livro->disponivel ? "Disponível" : "Indisponível");
    return 0;
}

static int removerLivroDoFicheiro(const char *titulo, const char *autor, const char *owner);
static int atualizarLivroNoFicheiro(const Livro *antigo, const Livro *novo);

static void header_catalogo_livros(int paginaAtual, int totalPaginas, void *context) {
    (void)context;
    linha_h_topo();
    texto_centrado("CATÁLOGO DE LIVROS");
    linha_h_meio();
    texto_centrado("PÁGINA %d de %d", paginaAtual, totalPaginas);
    linha_h_meio();
    texto_esquerda("Selecione um livro para ver detalhes.");
    linha_h_meio();
}

static void header_meus_livros(int paginaAtual, int totalPaginas, void *context) {
    (void)context;
    linha_h_topo();
    texto_centrado("MEUS LIVROS (VISUALIZAÇÃO)");
    linha_h_meio();
    texto_centrado("PÁGINA %d de %d", paginaAtual, totalPaginas);
    linha_h_meio();
    texto_esquerda("Escolha um livro para consultar os detalhes.");
    linha_h_meio();
}

static void header_admin_livros(int paginaAtual, int totalPaginas, void *context) {
    (void)context;
    linha_h_topo();
    texto_centrado("ADMINISTRAÇÃO DE LIVROS");
    linha_h_meio();
    texto_centrado("PÁGINA %d de %d", paginaAtual, totalPaginas);
    linha_h_meio();
    texto_esquerda("Selecione um livro para gerir.");
    linha_h_meio();
}

static int carregar_lista_opcoes(const char *arquivo, ItemSelecao itens[], int capacidade) {
    if (!arquivo || !itens || capacidade <= 0) return 0;

    FILE *fp = fopen(arquivo, "r");
    if (!fp) return 0;

    char linha[256];
    int total = 0;
    while (fgets(linha, sizeof(linha), fp) && total < capacidade) {
        char *sep = strchr(linha, ';');
        if (!sep) continue;
        *sep = '\0';
        int id = atoi(linha);
        char *nome = sep + 1;
        nome[strcspn(nome, "\r\n")] = '\0';
        garantir_utf8(nome, strlen(nome) + 1);

        itens[total].id = id;
        strncpy(itens[total].nome, nome, sizeof(itens[total].nome) - 1);
        itens[total].nome[sizeof(itens[total].nome) - 1] = '\0';
        total++;
    }
    fclose(fp);
    return total;
}

static void header_selecionar_item(int paginaAtual, int totalPaginas, void *context) {
    ContextoSelecao *ctx = (ContextoSelecao *)context;
    linha_h_topo();
    texto_centrado(ctx->cabecalho);
    linha_h_meio();
    texto_centrado("PÁGINA %d de %d", paginaAtual, totalPaginas);
    linha_h_meio();
}

static int formatar_item_selecao(char *buffer, size_t bufferSize, int index, void *item, void *context) {
    (void)index;
    (void)context;
    if (!buffer || bufferSize == 0 || !item) return 0;
    ItemSelecao *it = (ItemSelecao *)item;
    snprintf(buffer, bufferSize, "%s", it->nome);
    return 0;
}

static int selecionar_item_por_ficheiro(const char *arquivo, const char *cabecalho, const char *descricao, ObterIdFunc obterId, char *destino, size_t destinoTamanho, int *destId, int permitirAdicionarNovo) {
    if (!arquivo || !cabecalho || !descricao || !obterId || !destino || destinoTamanho == 0 || !destId) return 0;

    ItemSelecao itens[MAX_SELECAO_OPCOES];
    int total = carregar_lista_opcoes(arquivo, itens, MAX_SELECAO_OPCOES);

    if (total == 0 && !permitirAdicionarNovo) {
        limparEcra();
        linha_h_topo();
        texto_centrado(cabecalho);
        linha_h_meio();
        texto_esquerda(" Ainda não há %s cadastrados.", descricao);
        linha_h_fim();
        pausar();
        return 0;
    }

    while (1) {
        int escolha = 0;
        
        if (total > 0) {
            ContextoSelecao ctx = {cabecalho};
            escolha = menu_paginar_itens(itens, total, 10, sizeof(ItemSelecao), formatar_item_selecao, header_selecionar_item, &ctx);
        }

        if (escolha == 0) {
            if (!permitirAdicionarNovo) {
                return 0;
            }
            
            limparEcra();
            linha_h_topo();
            texto_centrado(cabecalho);
            linha_h_meio();
            texto_esquerda("Adicionar novo %s", descricao);
            linha_h_meio();
            
            char entrada[128];
            printf("║ Digite o nome: ");
            if (!ler_linha_sem_ponto_virgula(entrada, sizeof(entrada))) {
                aviso_ponto_virgula_nao_permitido();
                pausar();
                continue;
            }
            
            if (entrada[0] == '\0') {
                opcaoInvalida();
                pausar();
                continue;
            }
            
            garantir_utf8(entrada, sizeof(entrada));
            int novo_id = obterId(entrada);
            if (novo_id <= 0) {
                limparEcra();
                texto_esquerda("Erro ao guardar %s.", descricao);
                pausar();
                return 0;
            }
            *destId = novo_id;
            strncpy(destino, entrada, destinoTamanho - 1);
            destino[destinoTamanho - 1] = '\0';
            return 1;
        }

        if (escolha >= 1 && escolha <= total) {
            *destId = itens[escolha - 1].id;
            strncpy(destino, itens[escolha - 1].nome, destinoTamanho - 1);
            destino[destinoTamanho - 1] = '\0';
            return 1;
        }

        opcaoInvalida();
        pausar();
    }
}

void adicionarLivro(const char *nome_user) {
    Livro livro;
    FILE *fp = fopen("livros.txt", "a");
    if (!fp) {
        printf("Erro ao abrir o ficheiro livros.txt\n");
        pausar();
        return;
    }
    linha_h_topo();
    texto_centrado("ADICIONAR NOVO LIVRO");
    linha_h_meio();
    while (1) {
        printf("║ Título: ");
        if (!ler_linha_sem_ponto_virgula(livro.titulo, MAX_TITULO)) {
            aviso_ponto_virgula_nao_permitido();
            pausar();
            limparEcra();
            linha_h_topo();
            texto_centrado("ADICIONAR NOVO LIVRO");
            linha_h_meio();
            continue;
        }
        break;
    }

    if (!selecionar_item_por_ficheiro(ARQUIVO_AUTORES, "SELECIONE OU ADICIONE O AUTOR", "autor", obter_id_autor, livro.autor, sizeof(livro.autor), &livro.autor_id, 1)) {
        fclose(fp);
        return;
    }

    if (!selecionar_item_por_ficheiro(ARQUIVO_CATEGORIAS, "SELECIONE A CATEGORIA", "categoria", obter_id_categoria, livro.categoria, sizeof(livro.categoria), &livro.categoria_id, 0)) {
        fclose(fp);
        return;
    }

    strcpy(livro.owner, nome_user);

    limparEcra();
    linha_h_topo();
    texto_centrado("ADICIONAR NOVO LIVRO");
    linha_h_meio();
    texto_esquerda(" Título: %s", livro.titulo);
    texto_esquerda(" Autor: %s", livro.autor);
    texto_esquerda(" Categoria: %s", livro.categoria);
    linha_h_meio();

    texto_esquerda(" Condição do livro:");
    linha_h_meio();
    texto_esquerda("1 - Novo");
    texto_esquerda("2 - Bom");
    texto_esquerda("3 - Razoável");
    linha_h_meio();
    printf("║ Escolha uma opção: ");
    int opcao_condicao;
    if (!ler_int_intervalo(&opcao_condicao, 1, 3)) {
        opcao_condicao = 2;
    }
    fpopeec(8);
    switch(opcao_condicao) {
        case 1:
            livro.condicao_id = CONDICAO_NOVO;
            break;
        case 2:
            livro.condicao_id = CONDICAO_BOM;
            break;
        case 3:
            livro.condicao_id = CONDICAO_RAZOAVEL;
            break;
        default:
            livro.condicao_id = CONDICAO_BOM;
            break;
    }

    const char *nome_condicao = traduzir_condicao(livro.condicao_id);
    strncpy(livro.condicao, nome_condicao, MAX_CONDICAO - 1);
    livro.condicao[MAX_CONDICAO - 1] = '\0';
    texto_esquerda(" Condição: %s", livro.condicao);
    linha_h_meio();
    livro.disponivel = 1;  // Livro disponível por padrão ao adicionar
    
    if (confirmar_sn("║ Confirmar adição do livro?")) {
        fprintf(fp, "%s;%d;%s;%d;%d;%d\n", livro.titulo, livro.autor_id, livro.owner, livro.categoria_id, livro.condicao_id, livro.disponivel);
        fclose(fp);
        linha_h_meio();
        texto_centrado("LIVRO ADICIONADO COM SUCESSO");
        linha_h_fim();
        pausar();
        return;
    } else {
        fclose(fp);
        linha_h_meio();
        texto_centrado("ADICÃO DE LIVRO CANCELADA");
        linha_h_fim();
        pausar();
        return;
    }
}
void consultarLivros(const char *nome_user) {
    char opcao[10];
    linha_h_topo();
    texto_centrado("CONSULTAR CATÁLOGO DE LIVROS");
    linha_h_meio();
    texto_esquerda("  1. Pesquisar livros");
    texto_esquerda("  2. Ver todos os livros");
    linha_h_meio();
    texto_esquerda("Escolha uma opção: ");
    linha_h_fim();
    fgets(opcao, sizeof(opcao), stdin);
    limparEcra();

    if (opcao[0] == '1') {
        pesquisarLivros(nome_user);
        return;
    } else if (opcao[0] == '2') {
        // Continua para listar todos os livros
    } else {
        opcaoInvalida();
        return;
    }

    FILE *fp = fopen("livros.txt", "r");
    if (!fp) {
        printf("Erro ao abrir o ficheiro livros.txt\n");
        pausar();
        return;
    }

    Livro *livros = NULL;
    int total = 0, capacidade = 10;
    char linha[4 * MAX_TITULO];

    livros = malloc((size_t)capacidade * sizeof(Livro));
    if (!livros) {
        printf("Erro de memoria.\n");
        fclose(fp);
        return;
    }

    // Ler todos os livros do ficheiro (exceto os do próprio utilizador)
    while (fgets(linha, sizeof(linha), fp)) {
        if (total == capacidade) {
            capacidade *= 2;
            livros = realloc(livros, (size_t)capacidade * sizeof(Livro));
            if (!livros) {
                printf("Erro de memoria.\n");
                fclose(fp);
                return;
            }
        }
        Livro livro_temp;
        if (carregar_livro_de_linha(linha, &livro_temp)) {
            // Não mostrar livros do próprio utilizador
            if (strcmp(livro_temp.owner, nome_user) != 0) {
                livros[total] = livro_temp;
                total++;
            }
        }
    }
    fclose(fp);

    if (total == 0) {
        linha_h_topo();
        texto_centrado("NENHUM LIVRO ENCONTRADO");
        linha_h_fim();
        free(livros);
        return;
    }

    mostrarLivros(nome_user, livros, total);
    free(livros);
}

void pesquisarLivros(const char *nome_user) {
    FILE *fp = fopen("livros.txt", "r");
    if (!fp) {
        printf("Erro ao abrir o ficheiro livros.txt\n");
        pausar();
        return;
    }

    Livro *livros = NULL;
    int total = 0, capacidade = 10;
    char linha[4 * MAX_TITULO];

    livros = malloc((size_t)capacidade * sizeof(Livro));
    if (!livros) {
        printf("Erro de memoria.\n");
        fclose(fp);
        return;
    }

    // Ler todos os livros do ficheiro
    while (fgets(linha, sizeof(linha), fp)) {
        if (total == capacidade) {
            capacidade *= 2;
            livros = realloc(livros, (size_t)capacidade * sizeof(Livro));
            if (!livros) {
                printf("Erro de memoria.\n");
                fclose(fp);
                return;
            }
        }
        if (carregar_livro_de_linha(linha, &livros[total])) {
            total++;
        }
    }
    fclose(fp);

    char tipo[20];
    linha_h_topo();
    texto_centrado("PESQUISAR LIVROS");
    linha_h_meio();
    texto_esquerda("  1. Por categoria");
    texto_esquerda("  2. Por nome");
    texto_esquerda("  3. Por autor");
    texto_esquerda("  4. Por condição");
    linha_h_fim();
    printf("\nEscolha uma opção: ");
    fgets(tipo, sizeof(tipo), stdin);
    tipo[strcspn(tipo, "\n")] = 0;
    limparEcra();

    char query[MAX_TITULO];
    int escolha_numero = 0;

    if (tipo[0] == '1') {
        char categorias[100][MAX_CATEGORIA];
        int num_cat = 0;
        for (int i = 0; i < total; i++) {
            int found = 0;
            for (int j = 0; j < num_cat; j++) {
                if (strcmp(categorias[j], livros[i].categoria) == 0) {
                    found = 1;
                    break;
                }
            }
            if (!found && num_cat < 100) {
                strcpy(categorias[num_cat++], livros[i].categoria);
            }
        }
        linha_h_topo();
        texto_centrado("CATEGORIAS EXISTENTES");
        linha_h_meio();
        for (int i = 0; i < num_cat; i++) {
            texto_esquerda("  %d. %s", i + 1, categorias[i]);
        }
        linha_h_fim();
        printf("\nSelecione o número da categoria: ");
        if (!ler_int_intervalo(&escolha_numero, 1, num_cat)) {
            escolha_numero = 0;
        }
        
        if (escolha_numero >= 1 && escolha_numero <= num_cat) {
            strcpy(query, categorias[escolha_numero - 1]);
        } else {
            limparEcra();
            opcaoInvalida();
            free(livros);
            pausar();
            return;
        }
    } else if (tipo[0] == '3') {
        char autores[100][MAX_AUTOR];
        int num_aut = 0;
        for (int i = 0; i < total; i++) {
            int found = 0;
            for (int j = 0; j < num_aut; j++) {
                if (strcmp(autores[j], livros[i].autor) == 0) {
                    found = 1;
                    break;
                }
            }
            if (!found && num_aut < 100) {
                strcpy(autores[num_aut++], livros[i].autor);
            }
        }
        linha_h_topo();
        texto_centrado("AUTORES EXISTENTES");
        linha_h_meio();
        for (int i = 0; i < num_aut; i++) {
            texto_esquerda("  %d. %s", i + 1, autores[i]);
        }
        linha_h_fim();
        printf("\nSelecione o número do autor: ");
        if (!ler_int_intervalo(&escolha_numero, 1, num_aut)) {
            escolha_numero = 0;
        }
        
        if (escolha_numero >= 1 && escolha_numero <= num_aut) {
            strcpy(query, autores[escolha_numero - 1]);
        } else {
            limparEcra();
            opcaoInvalida();
            free(livros);
            pausar();
            return;
        }
    } else if (tipo[0] == '4') {
        char condicoes[10][MAX_CONDICAO];
        int num_cond = 0;
        for (int i = 0; i < total; i++) {
            int found = 0;
            for (int j = 0; j < num_cond; j++) {
                if (strcmp(condicoes[j], livros[i].condicao) == 0) {
                    found = 1;
                    break;
                }
            }
            if (!found && num_cond < 10) {
                strcpy(condicoes[num_cond++], livros[i].condicao);
            }
        }
        linha_h_topo();
        texto_centrado("CONDIÇÕES EXISTENTES");
        linha_h_meio();
        for (int i = 0; i < num_cond; i++) {
            texto_esquerda("  %d. %s", i + 1, condicoes[i]);
        }
        linha_h_fim();
        printf("\nSelecione o número da condição: ");
        if (!ler_int_intervalo(&escolha_numero, 1, num_cond)) {
            escolha_numero = 0;
        }
        
        if (escolha_numero >= 1 && escolha_numero <= num_cond) {
            strcpy(query, condicoes[escolha_numero - 1]);
        } else {
            limparEcra();
            opcaoInvalida();
            free(livros);
            pausar();
            return;
        }
    } else if (tipo[0] == '2') {
        printf("Digite o nome do livro: ");
        if (!ler_linha_sem_ponto_virgula(query, sizeof(query))) {
            aviso_ponto_virgula_nao_permitido();
            free(livros);
            pausar();
            return;
        }
    } else {
        limparEcra();
        opcaoInvalida();
        free(livros);
        pausar();
        return;
    }

    // Filtrar
    Livro *filtrados = NULL;
    int total_filtrados = 0, cap_filtrados = 10;
    filtrados = malloc((size_t)cap_filtrados * sizeof(Livro));
    if (!filtrados) {
        printf("Erro de memoria.\n");
        free(livros);
        return;
    }

    for (int i = 0; i < total; i++) {
        char *campo = NULL;
        if (tipo[0] == '1') campo = livros[i].categoria;
        else if (tipo[0] == '2') campo = livros[i].titulo;
        else if (tipo[0] == '3') campo = livros[i].autor;
        else if (tipo[0] == '4') campo = livros[i].condicao;
        else {
            printf("Tipo invalido.\n");
            free(livros);
            free(filtrados);
            return;
        }

        // Pesquisa por substring (ignorando maiúsculas/minúsculas)
        char campo_lower[MAX_TITULO], query_lower[MAX_TITULO];
        strcpy(campo_lower, campo);
        strcpy(query_lower, query);
        for (int j = 0; campo_lower[j]; j++) campo_lower[j] = (char)tolower((unsigned char)campo_lower[j]);
        for (int j = 0; query_lower[j]; j++) query_lower[j] = (char)tolower((unsigned char)query_lower[j]);

        if (strstr(campo_lower, query_lower)) {
            // Não mostrar livros do próprio utilizador
            if (strcmp(livros[i].owner, nome_user) != 0) {
                if (total_filtrados == cap_filtrados) {
                    cap_filtrados *= 2;
                    filtrados = realloc(filtrados, (size_t)cap_filtrados * sizeof(Livro));
                    if (!filtrados) {
                        printf("Erro de memoria.\n");
                        free(livros);
                        return;
                    }
                }
                filtrados[total_filtrados++] = livros[i];
            }
        }
    }
    free(livros);

    if (total_filtrados == 0) {
        limparEcra();
        linha_h_topo();
        texto_centrado("NENHUM LIVRO ENCONTRADO");
        linha_h_fim();
        free(filtrados);
        pausar();
        return;
    }

    limparEcra();
    mostrarLivros(nome_user, filtrados, total_filtrados);
    free(filtrados);
}

void listarMeusLivros(const char *nome_user) {
    FILE *fp = fopen("livros.txt", "r");
    if (!fp) {
        printf("Erro ao abrir o ficheiro livros.txt\n");
        pausar();
        return;
    }

    Livro *livros = NULL;
    int total = 0, capacidade = 10;
    char linha[4 * MAX_TITULO];

    livros = malloc((size_t)capacidade * sizeof(Livro));
    if (!livros) {
        printf("Erro de memoria.\n");
        fclose(fp);
        return;
    }

    while (fgets(linha, sizeof(linha), fp)) {
        if (total == capacidade) {
            capacidade *= 2;
            livros = realloc(livros, (size_t)capacidade * sizeof(Livro));
            if (!livros) {
                printf("Erro de memoria.\n");
                fclose(fp);
                return;
            }
        }
        if (carregar_livro_de_linha(linha, &livros[total])) {
            total++;
        }
    }
    fclose(fp);

    if (total == 0) {
        linha_h_topo();
        texto_centrado("NENHUM LIVRO ENCONTRADO");
        linha_h_fim();
        free(livros);
        return;
    }

    Livro *meus = malloc(10 * sizeof(Livro));
    if (!meus) {
        printf("Erro de memoria.\n");
        free(livros);
        return;
    }
    int total_meus = 0, cap_meus = 10;

    for (int i = 0; i < total; i++) {
        if (strcmp(livros[i].owner, nome_user) == 0) {
            if (total_meus == cap_meus) {
                cap_meus *= 2;
                Livro *tmp = realloc(meus, (size_t)cap_meus * sizeof(Livro));
                if (!tmp) {
                    printf("Erro de memoria.\n");
                    free(livros);
                    free(meus);
                    return;
                }
                meus = tmp;
            }
            meus[total_meus++] = livros[i];
        }
    }
    free(livros);

    if (total_meus == 0) {
        limparEcra();
        linha_h_topo();
        texto_centrado("NÃO EXISTEM LIVROS ASSOCIADOS A SI");
        linha_h_fim();
        free(meus);
        pausar();
        return;
    }

    /* Carregar transações para mostrar condição e pendências */
    Requisicao *reqs = NULL; int total_req = 0, cap_req = 10; char linha_req[500];
    FILE *fr = fopen("requisicoes.txt", "r");
    if (fr) {
        reqs = malloc((size_t)cap_req * sizeof(Requisicao));
        if (reqs) {
            while (fgets(linha_req, sizeof(linha_req), fr)) {
                if (total_req == cap_req) {
                    cap_req *= 2;
                    Requisicao *tmp = realloc(reqs, (size_t)cap_req * sizeof(Requisicao));
                    if (!tmp) { free(reqs); reqs = NULL; break; }
                    reqs = tmp;
                }
                sscanf(linha_req, "%d;%[^;];%[^;];%[^;];%[^;];%[^;];%[^\n]", 
                       &reqs[total_req].id, reqs[total_req].titulo, reqs[total_req].autor,
                       reqs[total_req].requester, reqs[total_req].owner, reqs[total_req].status, reqs[total_req].data);
                total_req++;
            }
        }
        fclose(fr);
    }

    Compra *comps = NULL; int total_comp = 0, cap_comp = 10; char linha_comp[500];
    FILE *fc = fopen("compras.txt", "r");
    if (fc) {
        comps = malloc((size_t)cap_comp * sizeof(Compra));
        if (comps) {
            while (fgets(linha_comp, sizeof(linha_comp), fc)) {
                if (total_comp == cap_comp) {
                    cap_comp *= 2;
                    Compra *tmp = realloc(comps, (size_t)cap_comp * sizeof(Compra));
                    if (!tmp) { free(comps); comps = NULL; break; }
                    comps = tmp;
                }
                sscanf(linha_comp, "%d;%[^;];%[^;];%[^;];%[^;];%f;%[^;];%[^\n]", 
                       &comps[total_comp].id, comps[total_comp].titulo, comps[total_comp].autor,
                       comps[total_comp].comprador, comps[total_comp].vendedor, &comps[total_comp].preco,
                       comps[total_comp].data, comps[total_comp].status);
                total_comp++;
            }
        }
        fclose(fc);
    }

    Troca *trocas = NULL; int total_trocas = 0, cap_trocas = 10; char linha_troca[500];
    FILE *ft = fopen("trocas.txt", "r");
    if (ft) {
        trocas = malloc((size_t)cap_trocas * sizeof(Troca));
        if (trocas) {
            while (fgets(linha_troca, sizeof(linha_troca), ft)) {
                if (total_trocas == cap_trocas) {
                    cap_trocas *= 2;
                    Troca *tmp = realloc(trocas, (size_t)cap_trocas * sizeof(Troca));
                    if (!tmp) { free(trocas); trocas = NULL; break; }
                    trocas = tmp;
                }
                sscanf(linha_troca, "%d;%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^\n]", 
                       &trocas[total_trocas].id, trocas[total_trocas].titulo1, trocas[total_trocas].autor1,
                       trocas[total_trocas].titulo2, trocas[total_trocas].autor2, trocas[total_trocas].user1,
                       trocas[total_trocas].user2, trocas[total_trocas].status, trocas[total_trocas].data);
                total_trocas++;
            }
        }
        fclose(ft);
    }

    while (1) {
        int escolha = menu_paginar_itens(meus, total_meus, 5, sizeof(Livro), formatar_livro_usuario, header_meus_livros, NULL);
        if (escolha == 0) {
            break;
        }

        int idx = escolha - 1;
        limparEcra();

        char condicao_final[128];
        char status_disponibilidade[50];
        snprintf(condicao_final, sizeof(condicao_final), "%s", meus[idx].condicao);
        snprintf(status_disponibilidade, sizeof(status_disponibilidade), "%s", meus[idx].disponivel ? "Disponível" : "Indisponível");

        linha_h_topo();
        texto_centrado("DETALHES DO MEU LIVRO");
        linha_h_meio();
        texto_esquerda("  Titulo: %s", meus[idx].titulo);
        texto_esquerda("  Autor: %s", meus[idx].autor);
        texto_esquerda("  Categoria: %s", meus[idx].categoria);
        texto_esquerda(" Disponibilidade: %s", status_disponibilidade);

        int pendencias = 0;
        if (reqs) {
            for (int r = 0; r < total_req; r++) {
                if (strcmp(reqs[r].owner, nome_user) == 0 && strcmp(reqs[r].titulo, meus[idx].titulo) == 0) {
                    if (strcmp(reqs[r].status, "Pendente") == 0) {
                        texto_esquerda(" Pendente: Requisição de %s", reqs[r].requester);
                        pendencias++;
                    } else if (strcmp(reqs[r].status, "Aceita") == 0) {
                        snprintf(condicao_final, sizeof(condicao_final), "Emprestado para %s", reqs[r].requester);
                    }
                }
            }
        }
        if (comps) {
            for (int c = 0; c < total_comp; c++) {
                if (strcmp(comps[c].vendedor, nome_user) == 0 && strcmp(comps[c].titulo, meus[idx].titulo) == 0) {
                    if (strcmp(comps[c].status, "Pendente") == 0) {
                        texto_esquerda(" Pendente: Compra de %s", comps[c].comprador);
                        pendencias++;
                    } else if (strcmp(comps[c].status, "Confirmada") == 0) {
                        snprintf(condicao_final, sizeof(condicao_final), "Vendido para %s", comps[c].comprador);
                    }
                }
            }
        }
        if (trocas) {
            for (int t = 0; t < total_trocas; t++) {
                int isMeu = 0; int souUser1 = 0;
                if (strcmp(trocas[t].user1, nome_user) == 0 && strcmp(trocas[t].titulo1, meus[idx].titulo) == 0) {
                    isMeu = 1; souUser1 = 1;
                } else if (strcmp(trocas[t].user2, nome_user) == 0 && strcmp(trocas[t].titulo2, meus[idx].titulo) == 0) {
                    isMeu = 1; souUser1 = 0;
                }
                if (isMeu) {
                    if (strcmp(trocas[t].status, "Pendente") == 0) {
                        if (souUser1)
                            texto_esquerda(" Pendente: Troca aguardando %s", trocas[t].user2);
                        else
                            texto_esquerda(" Pendente: Troca proposta por %s", trocas[t].user1);
                        pendencias++;
                    } else if (strcmp(trocas[t].status, "Aceita") == 0) {
                        if (souUser1)
                            snprintf(condicao_final, sizeof(condicao_final), "Trocado por %s (%s)", trocas[t].titulo2, trocas[t].user2);
                        else
                            snprintf(condicao_final, sizeof(condicao_final), "Trocado por %s (%s)", trocas[t].titulo1, trocas[t].user1);
                    }
                }
            }
        }

        if (pendencias == 0) {
            texto_esquerda(" Pendências: Nenhuma");
        }

        linha_h_meio();
        texto_esquerda(" Condição atual: %s", condicao_final);
        linha_h_meio();
        texto_esquerda("  1. Alternar disponibilidade");
        texto_esquerda("  2. Eliminar livro");
        texto_esquerda("  0. Voltar");
        linha_h_fim();

        char opcao[16];
        printf("\nEscolha uma opção: ");
        if (!fgets(opcao, sizeof(opcao), stdin)) {
            continue;
        }

        if (opcao[0] == '0') {
            limparEcra();
            continue;
        }

        if (opcao[0] == '1') {
            int nova = !meus[idx].disponivel;
            atualizarDisponibilidadeLivro(meus[idx].titulo, meus[idx].autor, meus[idx].owner, nova);
            meus[idx].disponivel = nova;

            limparEcra();
            linha_h_topo();
            texto_centrado("DISPONIBILIDADE ATUALIZADA");
            linha_h_meio();
            texto_esquerda("  Livro: %s", meus[idx].titulo);
            texto_esquerda("  Disponibilidade: %s", nova ? "Disponível" : "Indisponível");
            linha_h_fim();
            pausar();
            limparEcra();
            continue;
        }

        if (opcao[0] == '2') {
            if (!confirmar_sn("\nTem a certeza que quer eliminar este livro?")) {
                limparEcra();
                continue;
            }

            int ok = removerLivroDoFicheiro(meus[idx].titulo, meus[idx].autor, meus[idx].owner);
            limparEcra();
            linha_h_topo();
            if (ok) {
                texto_centrado("LIVRO ELIMINADO");
                linha_h_meio();
                texto_esquerda("  Livro: %s", meus[idx].titulo);
                linha_h_fim();

                for (int j = idx; j < total_meus - 1; j++) {
                    meus[j] = meus[j + 1];
                }
                total_meus--;

                if (total_meus == 0) {
                    pausar();
                    break;
                }
            } else {
                texto_centrado("ERRO AO ELIMINAR LIVRO");
                linha_h_meio();
                texto_esquerda("  Tente novamente.");
                linha_h_fim();
            }

            pausar();
            limparEcra();
            continue;
        }

        limparEcra();
        opcaoInvalida();
    }

    free(meus);
    free(reqs);
    free(comps);
    free(trocas);
}

void mostrarLivros(const char *nome_user, Livro *livros, int total) {
    char opcao[10];
    while (1) {
        int escolha = menu_paginar_itens(livros, total, 10, sizeof(Livro), formatar_livro_catalogo, header_catalogo_livros, NULL);
        if (escolha == 0) {
            limparEcra();
            break;
        }

        limparEcra();
        int idx = escolha - 1;
        linha_h_topo();
        texto_centrado("DETALHES DO LIVRO");
        linha_h_meio();
        texto_esquerda(" Titulo: %s", livros[idx].titulo);
        texto_esquerda(" Autor: %s", livros[idx].autor);
        texto_esquerda(" Dono: %s", livros[idx].owner);
        texto_esquerda(" Categoria: %s", livros[idx].categoria);
        texto_esquerda(" Condição: %s", livros[idx].condicao);
        texto_esquerda(" Disponibilidade: %s", livros[idx].disponivel ? "Disponível" : "Indisponível");
        linha_h_meio();
        if (livros[idx].disponivel) {
            texto_esquerda("  1. Requisitar livro");
            texto_esquerda("  2. Comprar livro");
            texto_esquerda("  3. Trocar livro");
        } else {
            texto_centrado("Livro não disponível para transações");
        }
        texto_esquerda("  0. Voltar");
        linha_h_fim();
        printf("\nEscolha uma opção: ");
        fgets(opcao, sizeof(opcao), stdin);

        if (livros[idx].disponivel) {
            if (opcao[0] == '1') {
                limparEcra();
                requisitarLivroEspecifico(nome_user, livros[idx].titulo, livros[idx].autor, livros[idx].owner);
            } else if (opcao[0] == '2') {
                limparEcra();
                comprarLivroEspecifico(nome_user, livros[idx].titulo, livros[idx].autor, livros[idx].owner);
            } else if (opcao[0] == '3') {
                limparEcra();
                trocarLivroEspecifico(nome_user, livros[idx].titulo, livros[idx].autor, livros[idx].owner);
            }
        }
        limparEcra();
    }
}
// Função para trocar a posse de um livro
void trocarPosseLivro(char *titulo, char *autor, char *antigoOwner, char *novoOwner) {
    FILE *fp = fopen("livros.txt", "r");
    FILE *tmp = fopen("livros_tmp.txt", "w");
    
    if (!fp || !tmp) {
        if (fp) fclose(fp);
        if (tmp) fclose(tmp);
        return;
    }

    char linha[500];
    while (fgets(linha, sizeof(linha), fp)) {
        Livro livro;
        if (!carregar_livro_de_linha(linha, &livro)) {
            continue;
        }
        
        if (strcmp(livro.titulo, titulo) == 0 && strcmp(livro.autor, autor) == 0 && strcmp(livro.owner, antigoOwner) == 0) {
            // Trocar o owner
            fprintf(tmp, "%s;%d;%s;%d;%d;%d\n", livro.titulo, livro.autor_id, novoOwner, livro.categoria_id, livro.condicao_id, livro.disponivel);
        } else {
            fprintf(tmp, "%s;%d;%s;%d;%d;%d\n", livro.titulo, livro.autor_id, livro.owner, livro.categoria_id, livro.condicao_id, livro.disponivel);
        }
    }
    
    fclose(fp);
    fclose(tmp);
    remove("livros.txt");
    rename("livros_tmp.txt", "livros.txt");
}

// Função para atualizar a disponibilidade de um livro
void atualizarDisponibilidadeLivro(char *titulo, char *autor, char *owner, int disponivel) {
    FILE *fp = fopen("livros.txt", "r");
    FILE *tmp = fopen("livros_tmp.txt", "w");
    
    if (!fp || !tmp) {
        if (fp) fclose(fp);
        if (tmp) fclose(tmp);
        return;
    }

    char linha[500];
    while (fgets(linha, sizeof(linha), fp)) {
        Livro livro;
        if (!carregar_livro_de_linha(linha, &livro)) {
            continue;
        }

        if (strcmp(livro.titulo, titulo) == 0 && strcmp(livro.autor, autor) == 0 && strcmp(livro.owner, owner) == 0) {
            // Atualizar disponibilidade
            fprintf(tmp, "%s;%d;%s;%d;%d;%d\n", livro.titulo, livro.autor_id, livro.owner, livro.categoria_id, livro.condicao_id, disponivel);
        } else {
            fprintf(tmp, "%s;%d;%s;%d;%d;%d\n", livro.titulo, livro.autor_id, livro.owner, livro.categoria_id, livro.condicao_id, livro.disponivel);
        }
    }
    
    fclose(fp);
    fclose(tmp);
    remove("livros.txt");
    rename("livros_tmp.txt", "livros.txt");
}

static int removerLivroDoFicheiro(const char *titulo, const char *autor, const char *owner) {
    FILE *fp = fopen("livros.txt", "r");
    FILE *tmp = fopen("livros_tmp.txt", "w");

    if (!fp || !tmp) {
        if (fp) fclose(fp);
        if (tmp) fclose(tmp);
        return 0;
    }

    int removido = 0;
    char linha[500];
    while (fgets(linha, sizeof(linha), fp)) {
        Livro livro;
        if (!carregar_livro_de_linha(linha, &livro)) {
            continue;
        }

        if (strcmp(livro.titulo, titulo) == 0 && strcmp(livro.autor, autor) == 0 && strcmp(livro.owner, owner) == 0) {
            removido = 1;
            continue;
        }

        fprintf(tmp, "%s;%d;%s;%d;%d;%d\n", livro.titulo, livro.autor_id, livro.owner, livro.categoria_id, livro.condicao_id, livro.disponivel);
    }

    fclose(fp);
    fclose(tmp);

    if (!removido) {
        remove("livros_tmp.txt");
        return 0;
    }

    remove("livros.txt");
    rename("livros_tmp.txt", "livros.txt");
    return 1;
}

// Função para verificar se um livro está disponível
int verificarDisponibilidadeLivro(char *titulo, char *autor, char *owner) {
    FILE *fp = fopen("livros.txt", "r");
    if (!fp) return 0;

    char linha[500];
    while (fgets(linha, sizeof(linha), fp)) {
        Livro livro;
        if (!carregar_livro_de_linha(linha, &livro)) {
            continue;
        }

        if (strcmp(livro.titulo, titulo) == 0 && strcmp(livro.autor, autor) == 0 && strcmp(livro.owner, owner) == 0) {
            fclose(fp);
            return livro.disponivel;
        }
    }
    
    fclose(fp);
    return 0;
}

// Menu de administração de livros
void menuAdminLivros(void) {
    FILE *fp = fopen("livros.txt", "r");
    if (!fp) {
        printf("Erro ao abrir o ficheiro livros.txt\n");
        pausar();
        return;
    }

    int total = 0;
    int capacidade = 10;
    Livro *livros = malloc((size_t)capacidade * sizeof(Livro));
    if (!livros) {
        fclose(fp);
        return;
    }

    char linha[500];
    while (fgets(linha, sizeof(linha), fp)) {
        Livro livro;
        if (!carregar_livro_de_linha(linha, &livro)) {
            continue;
        }

        if (total == capacidade) {
            capacidade *= 2;
            Livro *novo = realloc(livros, (size_t)capacidade * sizeof(Livro));
            if (!novo) {
                fclose(fp);
                free(livros);
                return;
            }
            livros = novo;
        }

        livros[total++] = livro;
    }
    fclose(fp);

    if (total == 0) {
        limparEcra();
        linha_h_topo();
        texto_centrado("NENHUM LIVRO PARA ADMINISTRAR");
        linha_h_fim();
        free(livros);
        pausar();
        return;
    }

    while (1) {
        int escolha = menu_paginar_itens(livros, total, 10, sizeof(Livro), formatar_livro_admin, header_admin_livros, NULL);
        if (escolha == 0) {
            break;
        }

        int idx = escolha - 1;
        while (1) {
            limparEcra();
            linha_h_topo();
            texto_centrado("GERIR LIVRO");
            linha_h_meio();
            texto_esquerda("Título: %s", livros[idx].titulo);
            texto_esquerda("Autor: %s", livros[idx].autor);
            texto_esquerda("Dono: %s", livros[idx].owner);
            texto_esquerda("Categoria: %s", livros[idx].categoria);
            texto_esquerda("Condição: %s", livros[idx].condicao);
            texto_esquerda("Disponibilidade: %s", livros[idx].disponivel ? "Disponível" : "Indisponível");
            linha_h_meio();
            texto_esquerda("1. Alterar título");
            texto_esquerda("2. Alterar autor");
            texto_esquerda("3. Alterar categoria");
            texto_esquerda("4. Alterar condição");
            texto_esquerda("5. Alternar disponibilidade");
            texto_esquerda("6. Eliminar livro");
            texto_esquerda("0. Voltar");
            linha_h_fim();
            printf("║ Escolha uma opção: ");

            int opcao;
            if (!ler_int_intervalo(&opcao, 0, 6)) {
                opcaoInvalida();
                pausar();
                continue;
            }

            if (opcao == 0) {
                break;
            }

            if (opcao == 5) {
                int novaDisponibilidade = !livros[idx].disponivel;
                atualizarDisponibilidadeLivro(livros[idx].titulo, livros[idx].autor, livros[idx].owner, novaDisponibilidade);
                livros[idx].disponivel = novaDisponibilidade;

                limparEcra();
                linha_h_topo();
                texto_centrado("ATUALIZAÇÃO DE DISPONIBILIDADE");
                linha_h_meio();
                texto_esquerda("Livro: %s", livros[idx].titulo);
                texto_esquerda("Status: %s", novaDisponibilidade ? "Disponível" : "Indisponível");
                linha_h_fim();
                pausar();
                continue;
            }

            if (opcao == 6) {
                limparEcra();
                linha_h_topo();
                texto_centrado("ELIMINAR LIVRO");
                linha_h_meio();
                texto_esquerda("Título: %s", livros[idx].titulo);
                texto_esquerda("Autor: %s", livros[idx].autor);
                texto_esquerda("Dono: %s", livros[idx].owner);
                linha_h_meio();
                linha_h_fim();
                
                if (confirmar_sn("║ Tem a certeza?")) {
                    int ok = removerLivroDoFicheiro(livros[idx].titulo, livros[idx].autor, livros[idx].owner);
                    limparEcra();
                    linha_h_topo();
                    if (ok) {
                        texto_centrado("SUCESSO");
                        linha_h_meio();
                        texto_esquerda("Livro eliminado.");
                        linha_h_fim();
                        pausar();

                        for (int i = idx; i < total - 1; i++) {
                            livros[i] = livros[i + 1];
                        }
                        total--;

                        if (total == 0) {
                            free(livros);
                            return;
                        }

                        if (idx >= total) idx = total - 1;
                        break;
                    }

                    texto_centrado("ERRO");
                    linha_h_meio();
                    texto_esquerda("Não foi possível eliminar o livro.");
                    linha_h_fim();
                    pausar();
                } else {
                    limparEcra();
                    linha_h_topo();
                    texto_centrado("CANCELADO");
                    linha_h_meio();
                    texto_esquerda("Operação cancelada.");
                    linha_h_fim();
                    pausar();
                }
                continue;
            }

            Livro original = livros[idx];
            Livro editado = livros[idx];

            if (opcao == 1) {
                limparEcra();
                linha_h_topo();
                texto_centrado("ALTERAR TÍTULO");
                linha_h_meio();
                linha_h_fim();
                printf("║ Novo título: ");

                if (!ler_linha_sem_ponto_virgula(editado.titulo, sizeof(editado.titulo))) {
                    aviso_ponto_virgula_nao_permitido();
                    pausar();
                    continue;
                }
            } else if (opcao == 2) {
                if (!selecionar_item_por_ficheiro(ARQUIVO_AUTORES, "SELECIONE OU ADICIONE O AUTOR", "autor", obter_id_autor, editado.autor, sizeof(editado.autor), &editado.autor_id, 1)) {
                    continue;
                }
            } else if (opcao == 3) {
                if (!selecionar_item_por_ficheiro(ARQUIVO_CATEGORIAS, "SELECIONE A CATEGORIA", "categoria", obter_id_categoria, editado.categoria, sizeof(editado.categoria), &editado.categoria_id, 0)) {
                    continue;
                }
            } else if (opcao == 4) {
                limparEcra();
                linha_h_topo();
                texto_centrado("ALTERAR CONDIÇÃO");
                linha_h_meio();
                texto_esquerda("1 - Novo");
                texto_esquerda("2 - Bom");
                texto_esquerda("3 - Razoável");
                linha_h_meio();
                linha_h_fim();
                printf("║ Escolha uma opção: ");

                int opcao_condicao;
                if (!ler_int_intervalo(&opcao_condicao, 1, 3)) {
                    opcaoInvalida();
                    pausar();
                    continue;
                }

                switch (opcao_condicao) {
                    case 1:
                        editado.condicao_id = CONDICAO_NOVO;
                        break;
                    case 2:
                        editado.condicao_id = CONDICAO_BOM;
                        break;
                    case 3:
                        editado.condicao_id = CONDICAO_RAZOAVEL;
                        break;
                    default:
                        opcaoInvalida();
                        pausar();
                        continue;
                }

                const char *nome_condicao = traduzir_condicao(editado.condicao_id);
                strncpy(editado.condicao, nome_condicao, sizeof(editado.condicao) - 1);
                editado.condicao[sizeof(editado.condicao) - 1] = '\0';
            } else {
                opcaoInvalida();
                pausar();
                continue;
            }

            int ok = atualizarLivroNoFicheiro(&original, &editado);
            limparEcra();
            linha_h_topo();
            if (ok) {
                texto_centrado("SUCESSO");
                linha_h_meio();
                texto_esquerda("Livro atualizado.");
                livros[idx] = editado;
            } else {
                texto_centrado("ERRO");
                linha_h_meio();
                texto_esquerda("Não foi possível atualizar o livro.");
            }
            linha_h_fim();
            pausar();
        }
    }

    free(livros);
}

static int atualizarLivroNoFicheiro(const Livro *antigo, const Livro *novo) {
    if (!antigo || !novo) return 0;

    FILE *fp = fopen("livros.txt", "r");
    if (!fp) return 0;

    FILE *tmp = fopen("livros_tmp.txt", "w");
    if (!tmp) {
        fclose(fp);
        return 0;
    }

    int atualizado = 0;
    char linha[500];
    while (fgets(linha, sizeof(linha), fp)) {
        Livro livro;
        if (!carregar_livro_de_linha(linha, &livro)) {
            continue;
        }

        if (!atualizado && strcmp(livro.titulo, antigo->titulo) == 0 && strcmp(livro.autor, antigo->autor) == 0 && strcmp(livro.owner, antigo->owner) == 0) {
            fprintf(tmp, "%s;%d;%s;%d;%d;%d\n", novo->titulo, novo->autor_id, novo->owner, novo->categoria_id, novo->condicao_id, novo->disponivel);
            atualizado = 1;
        } else {
            fprintf(tmp, "%s;%d;%s;%d;%d;%d\n", livro.titulo, livro.autor_id, livro.owner, livro.categoria_id, livro.condicao_id, livro.disponivel);
        }
    }

    fclose(fp);
    fclose(tmp);

    if (!atualizado) {
        remove("livros_tmp.txt");
        return 0;
    }

    remove("livros.txt");
    rename("livros_tmp.txt", "livros.txt");
    return 1;
}

int carregar_livro_de_linha(const char *linha, Livro *livro) {
    if (!linha || !livro) return 0;

    char titulo[MAX_TITULO];
    char owner[MAX_OWNER];
    int autor_id = 0;
    int categoria_id = 0;
    int condicao_id = 0;
    int disponivel = 0;

    int matched = sscanf(linha, " %[^;];%d;%[^;];%d;%d;%d", titulo, &autor_id, owner, &categoria_id, &condicao_id, &disponivel);
    if (matched < 6) return 0;

    /* Remove BOM UTF-8 (EF BB BF) caso exista no início do ficheiro.
       Isto acontecia no 1º livro do catálogo e fazia a borda direita ficar 1 carácter desalinhada,
       porque o BOM é “invisível” mas contava no cálculo de espaços. */
    if ((unsigned char)titulo[0] == 0xEF && (unsigned char)titulo[1] == 0xBB && (unsigned char)titulo[2] == 0xBF) {
        memmove(titulo, titulo + 3, strlen(titulo + 3) + 1);
    }

    strncpy(livro->titulo, titulo, MAX_TITULO - 1);
    livro->titulo[MAX_TITULO - 1] = '\0';
    garantir_utf8(livro->titulo, MAX_TITULO);

    strncpy(livro->owner, owner, MAX_OWNER - 1);
    livro->owner[MAX_OWNER - 1] = '\0';
    garantir_utf8(livro->owner, MAX_OWNER);

    livro->autor_id = autor_id;
    livro->categoria_id = categoria_id;
    livro->condicao_id = condicao_id;
    livro->disponivel = disponivel;

    const char *nome_autor = traduzir_autor(autor_id);
    strncpy(livro->autor, nome_autor, MAX_AUTOR - 1);
    livro->autor[MAX_AUTOR - 1] = '\0';

    const char *nome_categoria = traduzir_categoria(categoria_id);
    strncpy(livro->categoria, nome_categoria, MAX_CATEGORIA - 1);
    livro->categoria[MAX_CATEGORIA - 1] = '\0';

    const char *nome_condicao = traduzir_condicao(condicao_id);
    strncpy(livro->condicao, nome_condicao, MAX_CONDICAO - 1);
    livro->condicao[MAX_CONDICAO - 1] = '\0';

    return 1;
}
