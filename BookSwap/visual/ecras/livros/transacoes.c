#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "util.h"
#include "transacoes.h"
#include "livros.h"
#include "../user/users.h"

extern char f_nome[MAX_NOME];

static void cancelar_pendentes_por_livro(const char *titulo, const char *autor, const char *owner,
                                        int compraAceiteId, int trocaAceiteId) {
    if (!titulo || !autor || !owner) return;

    // Requisições pendentes para o livro deste owner -> Rejeitada
    {
        FILE *f = fopen("requisicoes.txt", "r");
        FILE *tmp = fopen("requisicoes_tmp.txt", "w");
        if (f && tmp) {
            char linha[500];
            while (fgets(linha, sizeof linha, f)) {
                Requisicao r;
                if (sscanf(linha, "%d;%[^;];%[^;];%[^;];%[^;];%[^;];%[^\n]", &r.id, r.titulo, r.autor, r.requester, r.owner, r.status, r.data) == 7) {
                    if (strcmp(r.status, "Pendente") == 0 && strcmp(r.owner, owner) == 0 &&
                        strcmp(r.titulo, titulo) == 0 && strcmp(r.autor, autor) == 0) {
                        fprintf(tmp, "%d;%s;%s;%s;%s;Rejeitada;%s\n", r.id, r.titulo, r.autor, r.requester, r.owner, r.data);
                    } else {
                        fputs(linha, tmp);
                    }
                } else {
                    fputs(linha, tmp);
                }
            }
            fclose(f);
            fclose(tmp);
            remove("requisicoes.txt");
            rename("requisicoes_tmp.txt", "requisicoes.txt");
        } else {
            if (f) fclose(f);
            if (tmp) fclose(tmp);
        }
    }

    // Compras pendentes para o livro deste vendedor -> Cancelada (exceto a compra aceite)
    {
        FILE *f = fopen("compras.txt", "r");
        FILE *tmp = fopen("compras_tmp.txt", "w");
        if (f && tmp) {
            char linha[500];
            while (fgets(linha, sizeof linha, f)) {
                Compra c;
                if (sscanf(linha, "%d;%[^;];%[^;];%[^;];%[^;];%f;%[^;];%[^\n]", &c.id, c.titulo, c.autor, c.comprador, c.vendedor, &c.preco, c.data, c.status) == 8) {
                    if (c.id != compraAceiteId && strcmp(c.status, "Pendente") == 0 && strcmp(c.vendedor, owner) == 0 &&
                        strcmp(c.titulo, titulo) == 0 && strcmp(c.autor, autor) == 0) {
                        fprintf(tmp, "%d;%s;%s;%s;%s;%.2f;%s;Cancelada\n", c.id, c.titulo, c.autor, c.comprador, c.vendedor, c.preco, c.data);
                    } else {
                        fputs(linha, tmp);
                    }
                } else {
                    fputs(linha, tmp);
                }
            }
            fclose(f);
            fclose(tmp);
            remove("compras.txt");
            rename("compras_tmp.txt", "compras.txt");
        } else {
            if (f) fclose(f);
            if (tmp) fclose(tmp);
        }
    }

    // Trocas pendentes que envolvam o livro -> Rejeitada (exceto a troca aceite)
    {
        FILE *f = fopen("trocas.txt", "r");
        FILE *tmp = fopen("trocas_tmp.txt", "w");
        if (f && tmp) {
            char linha[500];
            while (fgets(linha, sizeof linha, f)) {
                Troca t;
                if (sscanf(linha, "%d;%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^\n]", &t.id, t.titulo1, t.autor1, t.titulo2, t.autor2, t.user1, t.user2, t.status, t.data) == 9) {
                    int envolve = 0;
                    if (strcmp(t.status, "Pendente") == 0) {
                        // Livro do owner como livro 1
                        if (strcmp(t.user1, owner) == 0 && strcmp(t.titulo1, titulo) == 0 && strcmp(t.autor1, autor) == 0) envolve = 1;
                        // Livro do owner como livro 2
                        if (strcmp(t.user2, owner) == 0 && strcmp(t.titulo2, titulo) == 0 && strcmp(t.autor2, autor) == 0) envolve = 1;
                    }

                    if (envolve && t.id != trocaAceiteId) {
                        fprintf(tmp, "%d;%s;%s;%s;%s;%s;%s;Rejeitada;%s\n", t.id, t.titulo1, t.autor1, t.titulo2, t.autor2, t.user1, t.user2, t.data);
                    } else {
                        fputs(linha, tmp);
                    }
                } else {
                    fputs(linha, tmp);
                }
            }
            fclose(f);
            fclose(tmp);
            remove("trocas.txt");
            rename("trocas_tmp.txt", "trocas.txt");
        } else {
            if (f) fclose(f);
            if (tmp) fclose(tmp);
        }
    }
}

void obterDataAtual(char *data) {
    time_t agora = time(NULL);
    struct tm *tempo_local = localtime(&agora);
    strftime(data, MAX_DATA, "%d/%m/%Y", tempo_local);
}

int obterProximoIdRequisicao(void) {
    FILE *fp = fopen("requisicoes.txt", "r");
    int id = 1;
    if (fp) {
        char linha[500];
        while (fgets(linha, sizeof(linha), fp)) {
            int temp_id;
            sscanf(linha, "%d;", &temp_id);
            if (temp_id >= id) id = temp_id + 1;
        }
        fclose(fp);
    }
    return id;
}

int obterProximoIdCompra(void) {
    FILE *fp = fopen("compras.txt", "r");
    int id = 1;
    if (fp) {
        char linha[500];
        while (fgets(linha, sizeof(linha), fp)) {
            int temp_id;
            sscanf(linha, "%d;", &temp_id);
            if (temp_id >= id) id = temp_id + 1;
        }
        fclose(fp);
    }
    return id;
}

int obterProximoIdTroca(void) {
    FILE *fp = fopen("trocas.txt", "r");
    int id = 1;
    if (fp) {
        char linha[500];
        while (fgets(linha, sizeof(linha), fp)) {
            int temp_id;
            sscanf(linha, "%d;", &temp_id);
            if (temp_id >= id) id = temp_id + 1;
        }
        fclose(fp);
    }
    return id;
}

void requisitarLivroEspecifico(char *titulo, char *autor, char *owner) {
    // Verificar se o livro está disponível
    if (!verificarDisponibilidadeLivro(titulo, autor, owner)) {
        limparEcra();
        linha_h_topo();
        texto_centrado("LIVRO INDISPONÍVEL PARA REQUISIÇÃO");
        texto_centrado("Este livro não está disponível no momento.");
        linha_h_fim();
        pausar();
        return;
    }
    
    FILE *fp = fopen("requisicoes.txt", "a");
    if (fp) {
        char data[MAX_DATA];
        obterDataAtual(data);
        int id = obterProximoIdRequisicao();
        fprintf(fp, "%d;%s;%s;%s;%s;Pendente;%s\n", id, titulo, autor, f_nome, owner, data);
        fclose(fp);
    }
}

typedef struct {
    int id;
    char tipo[20];
    char titulo[MAX_TITULO];
    char autor[MAX_AUTOR];
    char outro_user[MAX_NOME];
    char status[MAX_STATUS];
    char data[MAX_DATA];
    float preco;
    char titulo2[MAX_TITULO];
    char autor2[MAX_AUTOR];
} PedidoUnificado;

static int formatar_pedido_unificado(char *buffer, size_t bufferSize, int index, void *item, void *context) {
    (void)index;
    (void)context;
    if (!item || !buffer || bufferSize == 0) {
        if (buffer && bufferSize > 0) buffer[0] = '\0';
        return 0;
    }

    PedidoUnificado *pedido = (PedidoUnificado *)item;

    const char *nota = "";
    if (strcmp(pedido->status, "Cancelada") == 0 || strcmp(pedido->status, "Rejeitada") == 0) {
        // Não dá para distinguir 100% se foi manual ou automático, por isso deixamos uma indicação neutra.
        nota = " (possível: livro ficou indisponível)";
    }

    snprintf(buffer, bufferSize, "[%s] #%d: %s (%s) | %s%s",
             pedido->tipo, pedido->id, pedido->titulo, pedido->autor, pedido->status, nota);
    return 0;
}

static void header_consultar_meus_pedidos(int paginaAtual, int totalPaginas, void *context) {
    (void)context;
    linha_h_topo();
    texto_centrado("CONSULTAR OS MEUS PEDIDOS");
    linha_h_meio();
    texto_centrado("PÁGINA %d de %d", paginaAtual, totalPaginas);
    linha_h_meio();
    texto_esquerda("Selecione um pedido. Só os pedidos pendentes podem ser cancelados.");
    linha_h_meio();
}

void consultarMeusPedidos(void) {
    PedidoUnificado *pedidos = NULL;
    int total_pedidos = 0, capacidade = 10;
    pedidos = malloc((size_t)capacidade * sizeof(PedidoUnificado));
    if (!pedidos) {
        printf("Erro de memória.\n");
        pausar();
        return;
    }

    // Carregar requisições
    FILE *fr = fopen("requisicoes.txt", "r");
    if (fr) {
        char linha[500];
        while (fgets(linha, sizeof(linha), fr)) {
            Requisicao r;
            sscanf(linha, "%d;%[^;];%[^;];%[^;];%[^;];%[^;];%[^\n]", 
                   &r.id, r.titulo, r.autor, r.requester, r.owner, r.status, r.data);
            
            if (strcmp(r.requester, f_nome) == 0) {
                if (total_pedidos == capacidade) {
                    capacidade *= 2;
                    PedidoUnificado *tmp = realloc(pedidos, (size_t)capacidade * sizeof(PedidoUnificado));
                    if (!tmp) { free(pedidos); fclose(fr); return; }
                    pedidos = tmp;
                }
                
                pedidos[total_pedidos].id = r.id;
                strcpy(pedidos[total_pedidos].tipo, "Requisição");
                strcpy(pedidos[total_pedidos].titulo, r.titulo);
                strcpy(pedidos[total_pedidos].autor, r.autor);
                strcpy(pedidos[total_pedidos].outro_user, r.owner);
                strcpy(pedidos[total_pedidos].status, r.status);
                strcpy(pedidos[total_pedidos].data, r.data);
                pedidos[total_pedidos].preco = 0;
                total_pedidos++;
            }
        }
        fclose(fr);
    }

    // Carregar compras
    FILE *fc = fopen("compras.txt", "r");
    if (fc) {
        char linha[500];
        while (fgets(linha, sizeof(linha), fc)) {
            Compra c;
            sscanf(linha, "%d;%[^;];%[^;];%[^;];%[^;];%f;%[^;];%[^\n]", 
                   &c.id, c.titulo, c.autor, c.comprador, c.vendedor, &c.preco, c.data, c.status);
            
            if (strcmp(c.comprador, f_nome) == 0) {
                if (total_pedidos == capacidade) {
                    capacidade *= 2;
                    PedidoUnificado *tmp = realloc(pedidos, (size_t)capacidade * sizeof(PedidoUnificado));
                    if (!tmp) { free(pedidos); fclose(fc); return; }
                    pedidos = tmp;
                }
                
                pedidos[total_pedidos].id = c.id;
                strcpy(pedidos[total_pedidos].tipo, "Compra");
                strcpy(pedidos[total_pedidos].titulo, c.titulo);
                strcpy(pedidos[total_pedidos].autor, c.autor);
                strcpy(pedidos[total_pedidos].outro_user, c.vendedor);
                strcpy(pedidos[total_pedidos].status, c.status);
                strcpy(pedidos[total_pedidos].data, c.data);
                pedidos[total_pedidos].preco = c.preco;
                total_pedidos++;
            }
        }
        fclose(fc);
    }

    // Carregar trocas
    FILE *ft = fopen("trocas.txt", "r");
    if (ft) {
        char linha[500];
        while (fgets(linha, sizeof(linha), ft)) {
            Troca t;
            sscanf(linha, "%d;%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^\n]", 
                   &t.id, t.titulo1, t.autor1, t.titulo2, t.autor2, t.user1, t.user2, t.status, t.data);
            
            if (strcmp(t.user1, f_nome) == 0) {
                if (total_pedidos == capacidade) {
                    capacidade *= 2;
                    PedidoUnificado *tmp = realloc(pedidos, (size_t)capacidade * sizeof(PedidoUnificado));
                    if (!tmp) { free(pedidos); fclose(ft); return; }
                    pedidos = tmp;
                }
                
                pedidos[total_pedidos].id = t.id;
                strcpy(pedidos[total_pedidos].tipo, "Troca");
                strcpy(pedidos[total_pedidos].titulo, t.titulo1);
                strcpy(pedidos[total_pedidos].autor, t.autor1);
                strcpy(pedidos[total_pedidos].titulo2, t.titulo2);
                strcpy(pedidos[total_pedidos].autor2, t.autor2);
                strcpy(pedidos[total_pedidos].outro_user, t.user2);
                strcpy(pedidos[total_pedidos].status, t.status);
                strcpy(pedidos[total_pedidos].data, t.data);
                pedidos[total_pedidos].preco = 0;
                total_pedidos++;
            }
        }
        fclose(ft);
    }

    if (total_pedidos == 0) {
        limparEcra();
        linha_h_topo();
        texto_centrado("NENHUM PEDIDO ENCONTRADO");
        linha_h_fim();
        free(pedidos);
        pausar();
        return;
    }

    while (1) {
        int escolha = menu_paginar_itens(pedidos, total_pedidos, 3, sizeof(PedidoUnificado),
                                         formatar_pedido_unificado, header_consultar_meus_pedidos, NULL);
        if (escolha == 0) {
            break;
        }

        int idx = escolha - 1;
        if (strcmp(pedidos[idx].status, "Pendente") != 0) {
            limparEcra();
            linha_h_topo();
            texto_centrado("ESTE PEDIDO JÁ NÃO ESTÁ PENDENTE");
            linha_h_meio();
            texto_esquerda(" Tipo: %s", pedidos[idx].tipo);
            texto_esquerda(" ID: #%d", pedidos[idx].id);
            texto_esquerda(" Livro: %s", pedidos[idx].titulo);
            texto_esquerda(" Estado: %s", pedidos[idx].status);
            linha_h_meio();
            texto_esquerda("Nota: pedidos podem ficar Cancelados/Rejeitados se");
            texto_esquerda("o livro tiver sido atribuído a outra transação.");
            linha_h_fim();
            pausar();
            continue;
        }

        limparEcra();
        linha_h_topo();
        texto_centrado("CANCELAR PEDIDO");
        linha_h_meio();
        texto_esquerda(" Tipo: %s", pedidos[idx].tipo);
        texto_esquerda(" ID: #%d", pedidos[idx].id);
        texto_esquerda(" Livro: %s", pedidos[idx].titulo);
        linha_h_fim();
        printf(" Tem certeza que deseja cancelar? (S/N)");

        char confirma[10];
        fgets(confirma, sizeof(confirma), stdin);

        if (confirma[0] == 'S' || confirma[0] == 's') {
            int sucesso = 0;

            if (strcmp(pedidos[idx].tipo, "Requisição") == 0) {
                FILE *f = fopen("requisicoes.txt", "r");
                FILE *tmp = fopen("requisicoes_tmp.txt", "w");
                if (f && tmp) {
                    char linha[500];
                    while (fgets(linha, sizeof(linha), f)) {
                        int id_linha;
                        sscanf(linha, "%d;", &id_linha);
                        if (id_linha != pedidos[idx].id) {
                            fputs(linha, tmp);
                        }
                    }
                    fclose(f);
                    fclose(tmp);
                    remove("requisicoes.txt");
                    rename("requisicoes_tmp.txt", "requisicoes.txt");
                    sucesso = 1;
                }
            } else if (strcmp(pedidos[idx].tipo, "Compra") == 0) {
                FILE *f = fopen("compras.txt", "r");
                FILE *tmp = fopen("compras_tmp.txt", "w");
                if (f && tmp) {
                    char linha[500];
                    while (fgets(linha, sizeof(linha), f)) {
                        int id_linha;
                        sscanf(linha, "%d;", &id_linha);
                        if (id_linha != pedidos[idx].id) {
                            fputs(linha, tmp);
                        }
                    }
                    fclose(f);
                    fclose(tmp);
                    remove("compras.txt");
                    rename("compras_tmp.txt", "compras.txt");
                    sucesso = 1;
                }
            } else if (strcmp(pedidos[idx].tipo, "Troca") == 0) {
                FILE *f = fopen("trocas.txt", "r");
                FILE *tmp = fopen("trocas_tmp.txt", "w");
                if (f && tmp) {
                    char linha[500];
                    while (fgets(linha, sizeof(linha), f)) {
                        int id_linha;
                        sscanf(linha, "%d;", &id_linha);
                        if (id_linha != pedidos[idx].id) {
                            fputs(linha, tmp);
                        }
                    }
                    fclose(f);
                    fclose(tmp);
                    remove("trocas.txt");
                    rename("trocas_tmp.txt", "trocas.txt");
                    sucesso = 1;
                }
            }

            if (sucesso) {
                limparEcra();
                linha_h_topo();
                texto_esquerda("PEDIDO CANCELADO COM SUCESSO!");
                texto_esquerda("O pedido foi removido do histórico");
                linha_h_fim();
                pausar();

                free(pedidos);
                consultarMeusPedidos();
                return;
            }
        } else {
            limparEcra();
            linha_h_topo();
            texto_centrado("CANCELAMENTO ABORTADO");
            linha_h_fim();
            pausar();
        }
    }

    free(pedidos);
}

void comprarLivroEspecifico(char *titulo, char *autor, char *vendedor) {
    // Verificar se o livro está disponível
    if (!verificarDisponibilidadeLivro(titulo, autor, vendedor)) {
        limparEcra();
        linha_h_topo();
        texto_centrado("LIVRO INDISPONÍVEL PARA COMPRA");
        texto_centrado("Este livro não está disponível no momento.");
        linha_h_fim();
        pausar();
        return;
    }
    
    float preco;
    while (1) {
        printf("Preço (0 cancelar): ");
        if (!ler_float_intervalo(&preco, 0.0f, 1000000.0f)) {
            limparEcra();
            linha_h_topo();
            texto_centrado("PREÇO INVÁLIDO");
            texto_centrado("Tente novamente.");
            linha_h_fim();
            pausar();
            continue;
        }
        break;
    }

    if (preco <= 0.0f) {
        return;
    }

    FILE *fp = fopen("compras.txt", "a");
    if (fp) {
        char data[MAX_DATA];
        obterDataAtual(data);
        int id = obterProximoIdCompra();
        fprintf(fp, "%d;%s;%s;%s;%s;%.2f;%s;Pendente\n", id, titulo, autor, f_nome, vendedor, preco, data);
        fclose(fp);
    }
}

void trocarLivroEspecifico(char *titulo, char *autor, char *owner) {
    // Verificar se o livro está disponível
    if (!verificarDisponibilidadeLivro(titulo, autor, owner)) {
        limparEcra();
        linha_h_topo();
        texto_centrado("LIVRO INDISPONÍVEL PARA TROCA");
        texto_centrado("Este livro não está disponível no momento.");
        linha_h_fim();
        pausar();
        return;
    }
    
    // Carregar os livros do utilizador
    FILE *fl = fopen("livros.txt", "r");
    if (!fl) {
        return;
    }

    Livro *meus_livros = NULL;
    int nmeus = 0, capmeus = 10;
    char linha[500];

    meus_livros = malloc((size_t)capmeus * sizeof(Livro));
    if (!meus_livros) {
        fclose(fl);
        return;
    }

    while (fgets(linha, sizeof(linha), fl)) {
        Livro livro_temp;
        if (!carregar_livro_de_linha(linha, &livro_temp)) {
            continue;
        }
        
        if (strcmp(livro_temp.owner, f_nome) == 0) {
            if (nmeus == capmeus) {
                capmeus *= 2;
                Livro *tmp = realloc(meus_livros, (size_t)capmeus * sizeof(Livro));
                if (!tmp) {
                    fclose(fl);
                    free(meus_livros);
                    return;
                }
                meus_livros = tmp;
            }
            meus_livros[nmeus++] = livro_temp;
        }
    }
    fclose(fl);

    if (nmeus == 0) {
        limparEcra();
        linha_h_topo();
        texto_esquerda("Não é possível fazer troca pois não tem");
        texto_esquerda("livros para trocar!");
        linha_h_fim();
        free(meus_livros);
        pausar();
        return;
    }

    limparEcra();
    linha_h_topo();
    texto_centrado("SELECIONE UM LIVRO PARA TROCAR");
    linha_h_meio();

    for (int i = 0; i < nmeus; i++) {
        texto_esquerda("[%d] %s (%s)", i + 1, meus_livros[i].titulo, meus_livros[i].autor);
    }
    texto_esquerda("[0] Cancelar");
    linha_h_fim();
    printf("\nEscolha um livro: ");
    int escolha;
    if (!ler_int_intervalo(&escolha, 0, nmeus)) {
        free(meus_livros);
        return;
    }

    if (escolha <= 0 || escolha > nmeus) {
        free(meus_livros);
        return;
    }

    // Registar a troca
    FILE *fp = fopen("trocas.txt", "a");
    if (fp) {
        char data[MAX_DATA];
        obterDataAtual(data);
        int id = obterProximoIdTroca();
        fprintf(fp, "%d;%s;%s;%s;%s;%s;%s;Pendente;%s\n", id, meus_livros[escolha - 1].titulo, meus_livros[escolha - 1].autor, titulo, autor, f_nome, owner, data);
        fclose(fp);

        limparEcra();
        linha_h_topo();
        texto_centrado("TROCA PROPOSTA COM SUCESSO");
        linha_h_fim();
        pausar();
    }
    free(meus_livros);
}

void menuTransacoesUsuario(void) {
    // Carregar requisições recebidas (onde sou owner)
    FILE *fr = fopen("requisicoes.txt", "r");
    Requisicao *reqs = NULL; int nreq = 0, capreq = 10; char linhar[500];
    if (fr) {
        reqs = malloc((size_t)capreq * sizeof(Requisicao));
        while (fgets(linhar, sizeof linhar, fr)) {
            if (nreq == capreq) { capreq*=2; Requisicao *tmp=realloc(reqs, (size_t)capreq*sizeof(Requisicao)); if(!tmp){free(reqs); reqs=NULL; break;} reqs=tmp; }
            sscanf(linhar, "%d;%[^;];%[^;];%[^;];%[^;];%[^;];%[^\n]", &reqs[nreq].id, reqs[nreq].titulo, reqs[nreq].autor, reqs[nreq].requester, reqs[nreq].owner, reqs[nreq].status, reqs[nreq].data);
            nreq++;
        }
        fclose(fr);
    }

    // Carregar compras recebidas (onde sou vendedor)
    FILE *fc = fopen("compras.txt", "r");
    Compra *comps = NULL; int ncomp = 0, capcomp = 10; char linhac[500];
    if (fc) {
        comps = malloc((size_t)capcomp * sizeof(Compra));
        while (fgets(linhac, sizeof linhac, fc)) {
            if (ncomp == capcomp) { capcomp*=2; Compra *tmp=realloc(comps, (size_t)capcomp*sizeof(Compra)); if(!tmp){free(comps); comps=NULL; break;} comps=tmp; }
            sscanf(linhac, "%d;%[^;];%[^;];%[^;];%[^;];%f;%[^;];%[^\n]", &comps[ncomp].id, comps[ncomp].titulo, comps[ncomp].autor, comps[ncomp].comprador, comps[ncomp].vendedor, &comps[ncomp].preco, comps[ncomp].data, comps[ncomp].status);
            ncomp++;
        }
        fclose(fc);
    }

    // Carregar trocas recebidas (onde sou user2)
    FILE *ft = fopen("trocas.txt", "r");
    Troca *trocas = NULL; int nt = 0, capt = 10; char linhat[500];
    if (ft) {
        trocas = malloc((size_t)capt * sizeof(Troca));
        while (fgets(linhat, sizeof linhat, ft)) {
            if (nt == capt) { capt*=2; Troca *tmp=realloc(trocas, (size_t)capt*sizeof(Troca)); if(!tmp){free(trocas); trocas=NULL; break;} trocas=tmp; }
            sscanf(linhat, "%d;%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^\n]", &trocas[nt].id, trocas[nt].titulo1, trocas[nt].autor1, trocas[nt].titulo2, trocas[nt].autor2, trocas[nt].user1, trocas[nt].user2, trocas[nt].status, trocas[nt].data);
            nt++;
        }
        fclose(ft);
    }

    limparEcra();
    linha_h_topo();
    texto_centrado("PEDIDOS PENDENTES PARA RESPONDER");
    linha_h_meio();

    typedef struct {
        int tipo; // 0=req, 1=comp, 2=troca
        int id;
        char descricao[200];
    } PedidoPendente;

    PedidoPendente *pedidos = NULL; int npedidos = 0, cappedidos = 10;
    pedidos = malloc((size_t)cappedidos * sizeof(PedidoPendente));

    // Requisições pendentes
    if (reqs) {
        for (int i = 0; i < nreq; i++) {
            if (strcmp(reqs[i].owner, f_nome) == 0 && strcmp(reqs[i].status, "Pendente") == 0) {
                if (npedidos == cappedidos) { cappedidos*=2; PedidoPendente *tmp=realloc(pedidos, (size_t)cappedidos*sizeof(PedidoPendente)); if(!tmp)break; pedidos=tmp; }
                pedidos[npedidos].tipo = 0;
                pedidos[npedidos].id = reqs[i].id;
                snprintf(pedidos[npedidos].descricao, sizeof(pedidos[npedidos].descricao), "REQ #%d: %s - de %s", reqs[i].id, reqs[i].titulo, reqs[i].requester);
                npedidos++;
            }
        }
    }

    // Compras pendentes
    if (comps) {
        for (int i = 0; i < ncomp; i++) {
            if (strcmp(comps[i].vendedor, f_nome) == 0 && strcmp(comps[i].status, "Pendente") == 0) {
                if (npedidos == cappedidos) { cappedidos*=2; PedidoPendente *tmp=realloc(pedidos, (size_t)cappedidos*sizeof(PedidoPendente)); if(!tmp)break; pedidos=tmp; }
                pedidos[npedidos].tipo = 1;
                pedidos[npedidos].id = comps[i].id;
                snprintf(pedidos[npedidos].descricao, sizeof(pedidos[npedidos].descricao), "COMP #%d: %s (%.2f€) - de %s", comps[i].id, comps[i].titulo, comps[i].preco, comps[i].comprador);
                npedidos++;
            }
        }
    }

    // Trocas pendentes
    if (trocas) {
        for (int i = 0; i < nt; i++) {
            if (strcmp(trocas[i].user2, f_nome) == 0 && strcmp(trocas[i].status, "Pendente") == 0) {
                if (npedidos == cappedidos) { cappedidos*=2; PedidoPendente *tmp=realloc(pedidos, (size_t)cappedidos*sizeof(PedidoPendente)); if(!tmp)break; pedidos=tmp; }
                pedidos[npedidos].tipo = 2;
                pedidos[npedidos].id = trocas[i].id;
                snprintf(pedidos[npedidos].descricao, sizeof(pedidos[npedidos].descricao), "TROCA #%d: %s ⇄ %s - de %s", trocas[i].id, trocas[i].titulo1, trocas[i].titulo2, trocas[i].user1);
                npedidos++;
            }
        }
    }

    if (npedidos == 0) {
        texto_centrado("Nenhum pedido pendente para responder.");
        linha_h_fim();
        free(reqs); free(comps); free(trocas); free(pedidos);
        pausar();
        return;
    }

    for (int i = 0; i < npedidos; i++) {
        texto_esquerda("[%d] %s", i + 1, pedidos[i].descricao);
    }
    texto_esquerda("[0] Voltar");
    linha_h_fim();
    printf("Selecione um pedido: ");
    int selecionado;
    if (!ler_int_intervalo(&selecionado, 0, npedidos)) {
        opcaoInvalida();
        pausar();
        free(reqs); free(comps); free(trocas); free(pedidos);
        return;
    }

    if (selecionado <= 0 || selecionado > npedidos) {
        free(reqs); free(comps); free(trocas); free(pedidos);
        return;
    }

    int tipo_escolhido = pedidos[selecionado - 1].tipo;
    int id_escolhido = pedidos[selecionado - 1].id;

    limparEcra();
    linha_h_topo();
    texto_centrado("RESPONDER AO PEDIDO");
    linha_h_meio();
    texto_esquerda("1. Aceitar");
    texto_esquerda("2. Recusar");
    texto_esquerda("0. Cancelar");
    linha_h_fim();
    printf("Escolha uma opção: ");
    int opcao_resp;
    if (!ler_int_intervalo(&opcao_resp, 0, 2)) {
        opcaoInvalida();
        pausar();
        free(reqs); free(comps); free(trocas); free(pedidos);
        return;
    }

    if (opcao_resp == 1) {
        // Aceitar
        if (tipo_escolhido == 0) {
            // Antes de aceitar, confirmar que o livro ainda está disponível.
            Requisicao req_chk;
            int found_chk = 0;
            {
                FILE *fchk = fopen("requisicoes.txt", "r");
                if (fchk) {
                    char linha_chk[500];
                    while (fgets(linha_chk, sizeof linha_chk, fchk)) {
                        Requisicao r;
                        sscanf(linha_chk, "%d;%[^;];%[^;];%[^;];%[^;];%[^;];%[^\n]", &r.id, r.titulo, r.autor, r.requester, r.owner, r.status, r.data);
                        if (r.id == id_escolhido && strcmp(r.owner, f_nome) == 0) {
                            req_chk = r;
                            found_chk = 1;
                            break;
                        }
                    }
                    fclose(fchk);
                }
            }

            if (!found_chk) {
                limparEcra();
                linha_h_topo();
                texto_centrado("ERRO");
                texto_centrado("Não foi possível encontrar o pedido.");
                linha_h_fim();
                free(reqs); free(comps); free(trocas); free(pedidos);
                pausar();
                return;
            }

            if (!verificarDisponibilidadeLivro(req_chk.titulo, req_chk.autor, req_chk.owner)) {
                limparEcra();
                linha_h_topo();
                texto_centrado("NÃO É POSSÍVEL ACEITAR");
                linha_h_meio();
                texto_centrado("Não pode aceitar transações em livros indisponíveis.");
                linha_h_fim();
                free(reqs); free(comps); free(trocas); free(pedidos);
                pausar();
                return;
            }

            FILE *f = fopen("requisicoes.txt", "r");
            FILE *tmp = fopen("requisicoes_tmp.txt", "w");
            if (f && tmp) {
                char linha[500];
                Requisicao req_aceita;
                int found = 0;
                while (fgets(linha, sizeof linha, f)) {
                    Requisicao r;
                    sscanf(linha, "%d;%[^;];%[^;];%[^;];%[^;];%[^;];%[^\n]", &r.id, r.titulo, r.autor, r.requester, r.owner, r.status, r.data);
                    if (r.id == id_escolhido && strcmp(r.owner, f_nome) == 0) {
                        fprintf(tmp, "%d;%s;%s;%s;%s;Aceita;%s\n", r.id, r.titulo, r.autor, r.requester, r.owner, r.data);
                        req_aceita = r;
                        found = 1;
                    } else {
                        fputs(linha, tmp);
                    }
                }
                fclose(f); fclose(tmp);
                remove("requisicoes.txt");
                rename("requisicoes_tmp.txt", "requisicoes.txt");

                // Ao aceitar uma requisição, o livro continua a ser do owner, mas fica indisponível (emprestado).
                if (found) {
                    atualizarDisponibilidadeLivro(req_aceita.titulo, req_aceita.autor, req_aceita.owner, 0);
                }

                limparEcra(); linha_h_topo();
                texto_centrado("REQUISIÇÃO ACEITA");
                linha_h_fim();
            }
        } else if (tipo_escolhido == 1) {
            // Antes de aceitar, confirmar que o livro ainda está disponível.
            Compra compra_chk;
            int found_chk = 0;
            {
                FILE *fchk = fopen("compras.txt", "r");
                if (fchk) {
                    char linha_chk[500];
                    while (fgets(linha_chk, sizeof linha_chk, fchk)) {
                        Compra c;
                        sscanf(linha_chk, "%d;%[^;];%[^;];%[^;];%[^;];%f;%[^;];%[^\n]", &c.id, c.titulo, c.autor, c.comprador, c.vendedor, &c.preco, c.data, c.status);
                        if (c.id == id_escolhido && strcmp(c.vendedor, f_nome) == 0) {
                            compra_chk = c;
                            found_chk = 1;
                            break;
                        }
                    }
                    fclose(fchk);
                }
            }

            if (!found_chk) {
                limparEcra();
                linha_h_topo();
                texto_centrado("ERRO");
                texto_centrado("Não foi possível encontrar o pedido.");
                linha_h_fim();
                free(reqs); free(comps); free(trocas); free(pedidos);
                pausar();
                return;
            }

            if (!verificarDisponibilidadeLivro(compra_chk.titulo, compra_chk.autor, compra_chk.vendedor)) {
                limparEcra();
                linha_h_topo();
                texto_centrado("NÃO É POSSÍVEL ACEITAR");
                linha_h_meio();
                texto_centrado("Não pode aceitar transações em livros indisponíveis.");
                linha_h_fim();
                free(reqs); free(comps); free(trocas); free(pedidos);
                pausar();
                return;
            }

            FILE *f = fopen("compras.txt", "r");
            FILE *tmp = fopen("compras_tmp.txt", "w");
            if (f && tmp) {
                char linha[500];
                Compra compra_aceita;
                int found = 0;
                while (fgets(linha, sizeof linha, f)) {
                    Compra c;
                    sscanf(linha, "%d;%[^;];%[^;];%[^;];%[^;];%f;%[^;];%[^\n]", &c.id, c.titulo, c.autor, c.comprador, c.vendedor, &c.preco, c.data, c.status);
                    if (c.id == id_escolhido && strcmp(c.vendedor, f_nome) == 0) {
                        fprintf(tmp, "%d;%s;%s;%s;%s;%.2f;%s;Confirmada\n", c.id, c.titulo, c.autor, c.comprador, c.vendedor, c.preco, c.data);
                        compra_aceita = c;
                        found = 1;
                    } else {
                        fputs(linha, tmp);
                    }
                }
                fclose(f); fclose(tmp);
                remove("compras.txt");
                rename("compras_tmp.txt", "compras.txt");
                
                // Trocar a posse do livro
                if (found) {
                    cancelar_pendentes_por_livro(compra_aceita.titulo, compra_aceita.autor, compra_aceita.vendedor, compra_aceita.id, -1);
                    trocarPosseLivro(compra_aceita.titulo, compra_aceita.autor, compra_aceita.vendedor, compra_aceita.comprador);
                    // Depois de comprar, o livro passa para o comprador e fica indisponível até ele decidir disponibilizar.
                    atualizarDisponibilidadeLivro(compra_aceita.titulo, compra_aceita.autor, compra_aceita.comprador, 0);
                }
                
                limparEcra(); linha_h_topo();
                texto_centrado("COMPRA CONFIRMADA");
                texto_centrado("Livro transferido com sucesso!");
                linha_h_fim();
            }
        } else if (tipo_escolhido == 2) {
            // Antes de aceitar, confirmar que ambos os livros ainda estão disponíveis.
            Troca troca_chk;
            int found_chk = 0;
            {
                FILE *fchk = fopen("trocas.txt", "r");
                if (fchk) {
                    char linha_chk[500];
                    while (fgets(linha_chk, sizeof linha_chk, fchk)) {
                        Troca t;
                        sscanf(linha_chk, "%d;%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^\n]", &t.id, t.titulo1, t.autor1, t.titulo2, t.autor2, t.user1, t.user2, t.status, t.data);
                        if (t.id == id_escolhido && strcmp(t.user2, f_nome) == 0) {
                            troca_chk = t;
                            found_chk = 1;
                            break;
                        }
                    }
                    fclose(fchk);
                }
            }

            if (!found_chk) {
                limparEcra();
                linha_h_topo();
                texto_centrado("ERRO");
                texto_centrado("Não foi possível encontrar o pedido.");
                linha_h_fim();
                free(reqs); free(comps); free(trocas); free(pedidos);
                pausar();
                return;
            }

            if (!verificarDisponibilidadeLivro(troca_chk.titulo1, troca_chk.autor1, troca_chk.user1) ||
                !verificarDisponibilidadeLivro(troca_chk.titulo2, troca_chk.autor2, troca_chk.user2)) {
                limparEcra();
                linha_h_topo();
                texto_centrado("NÃO É POSSÍVEL ACEITAR");
                linha_h_meio();
                texto_centrado("Não pode aceitar transações em livros indisponíveis.");
                linha_h_fim();
                free(reqs); free(comps); free(trocas); free(pedidos);
                pausar();
                return;
            }

            FILE *f = fopen("trocas.txt", "r");
            FILE *tmp = fopen("trocas_tmp.txt", "w");
            if (f && tmp) {
                char linha[500];
                Troca troca_aceita;
                int found = 0;
                while (fgets(linha, sizeof linha, f)) {
                    Troca t;
                    sscanf(linha, "%d;%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^\n]", &t.id, t.titulo1, t.autor1, t.titulo2, t.autor2, t.user1, t.user2, t.status, t.data);
                    if (t.id == id_escolhido && strcmp(t.user2, f_nome) == 0) {
                        fprintf(tmp, "%d;%s;%s;%s;%s;%s;%s;Aceita;%s\n", t.id, t.titulo1, t.autor1, t.titulo2, t.autor2, t.user1, t.user2, t.data);
                        troca_aceita = t;
                        found = 1;
                    } else {
                        fputs(linha, tmp);
                    }
                }
                fclose(f); fclose(tmp);
                remove("trocas.txt");
                rename("trocas_tmp.txt", "trocas.txt");
                
                // Trocar a posse dos livros e marcá-los como indisponíveis
                if (found) {
                    // Cancelar pedidos pendentes que envolvam qualquer um dos dois livros.
                    cancelar_pendentes_por_livro(troca_aceita.titulo1, troca_aceita.autor1, troca_aceita.user1, -1, troca_aceita.id);
                    cancelar_pendentes_por_livro(troca_aceita.titulo2, troca_aceita.autor2, troca_aceita.user2, -1, troca_aceita.id);

                    // Livro 1 (do user1) vai para user2
                    trocarPosseLivro(troca_aceita.titulo1, troca_aceita.autor1, troca_aceita.user1, troca_aceita.user2);
                    atualizarDisponibilidadeLivro(troca_aceita.titulo1, troca_aceita.autor1, troca_aceita.user2, 0);
                    
                    // Livro 2 (do user2) vai para user1
                    trocarPosseLivro(troca_aceita.titulo2, troca_aceita.autor2, troca_aceita.user2, troca_aceita.user1);
                    atualizarDisponibilidadeLivro(troca_aceita.titulo2, troca_aceita.autor2, troca_aceita.user1, 0);
                }
                
                limparEcra(); linha_h_topo();
                texto_centrado("TROCA ACEITA");
                texto_centrado("Livros trocados automaticamente!");
                texto_centrado("Os livros foram marcados como indisponíveis");
                texto_centrado("(os novos donos podem voltar a disponibilizar)");
                linha_h_fim();
            }
        }
    } else if (opcao_resp == 2) {
        // Recusar
        if (tipo_escolhido == 0) {
            FILE *f = fopen("requisicoes.txt", "r");
            FILE *tmp = fopen("requisicoes_tmp.txt", "w");
            if (f && tmp) {
                char linha[500];
                while (fgets(linha, sizeof linha, f)) {
                    Requisicao r;
                    sscanf(linha, "%d;%[^;];%[^;];%[^;];%[^;];%[^;];%[^\n]", &r.id, r.titulo, r.autor, r.requester, r.owner, r.status, r.data);
                    if (r.id == id_escolhido && strcmp(r.owner, f_nome) == 0) {
                        fprintf(tmp, "%d;%s;%s;%s;%s;Rejeitada;%s\n", r.id, r.titulo, r.autor, r.requester, r.owner, r.data);
                    } else {
                        fputs(linha, tmp);
                    }
                }
                fclose(f); fclose(tmp);
                remove("requisicoes.txt");
                rename("requisicoes_tmp.txt", "requisicoes.txt");
                limparEcra(); linha_h_topo();
                texto_centrado("REQUISIÇÃO REJEITADA");
                linha_h_fim();
            }
        } else if (tipo_escolhido == 1) {
            FILE *f = fopen("compras.txt", "r");
            FILE *tmp = fopen("compras_tmp.txt", "w");
            if (f && tmp) {
                char linha[500];
                while (fgets(linha, sizeof linha, f)) {
                    Compra c;
                    sscanf(linha, "%d;%[^;];%[^;];%[^;];%[^;];%f;%[^;];%[^\n]", &c.id, c.titulo, c.autor, c.comprador, c.vendedor, &c.preco, c.data, c.status);
                    if (c.id == id_escolhido && strcmp(c.vendedor, f_nome) == 0) {
                        fprintf(tmp, "%d;%s;%s;%s;%s;%.2f;%s;Cancelada\n", c.id, c.titulo, c.autor, c.comprador, c.vendedor, c.preco, c.data);
                    } else {
                        fputs(linha, tmp);
                    }
                }
                fclose(f); fclose(tmp);
                remove("compras.txt");
                rename("compras_tmp.txt", "compras.txt");
                limparEcra(); linha_h_topo();
                texto_centrado("COMPRA CANCELADA");
                linha_h_fim();
            }
        } else if (tipo_escolhido == 2) {
            FILE *f = fopen("trocas.txt", "r");
            FILE *tmp = fopen("trocas_tmp.txt", "w");
            if (f && tmp) {
                char linha[500];
                while (fgets(linha, sizeof linha, f)) {
                    Troca t;
                    sscanf(linha, "%d;%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^\n]", &t.id, t.titulo1, t.autor1, t.titulo2, t.autor2, t.user1, t.user2, t.status, t.data);
                    if (t.id == id_escolhido && strcmp(t.user2, f_nome) == 0) {
                        fprintf(tmp, "%d;%s;%s;%s;%s;%s;%s;Rejeitada;%s\n", t.id, t.titulo1, t.autor1, t.titulo2, t.autor2, t.user1, t.user2, t.data);
                    } else {
                        fputs(linha, tmp);
                    }
                }
                fclose(f); fclose(tmp);
                remove("trocas.txt");
                rename("trocas_tmp.txt", "trocas.txt");
                limparEcra(); linha_h_topo();
                texto_centrado("TROCA REJEITADA");
                linha_h_fim();
            }
        }
    }

    free(reqs); free(comps); free(trocas); free(pedidos);
    pausar();
}
