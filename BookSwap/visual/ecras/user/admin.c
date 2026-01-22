#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "livros.h"
#include "transacoes.h"
#include "../user/users.h"
#include "admin.h"

typedef struct {
    char nome[MAX_NOME];
    char email[MAX_EMAIL];
    char password[MAX_PASSWORD];
} Usuario;

static void header_admin_usuarios(int paginaAtual, int totalPaginas, void *context) {
    (void)context;
    linha_h_topo();
    texto_centrado("GESTÃO DE UTILIZADORES");
    linha_h_meio();
    texto_centrado("PÁGINA %d de %d", paginaAtual, totalPaginas);
    linha_h_meio();
    texto_esquerda("Selecione um utilizador para editar.");
    linha_h_meio();
}

static int formatar_nome_usuario(char *buffer, size_t bufferSize, int index, void *item, void *context) {
    (void)index;
    (void)context;
    if (!buffer || bufferSize == 0 || !item) return 0;

    snprintf(buffer, bufferSize, "%s", (char *)item);
    return 0;
}

// Função para verificar se o utilizador atual é admin
int isAdmin(const char *nome_utilizador) {
    FILE *fa = fopen("admin.txt", "r");
    if (!fa) return 0;
    
    char linha[200];
    char nomeLido[MAX_NOME], email[MAX_EMAIL], pass[MAX_PASSWORD];
    
    while (fgets(linha, sizeof(linha), fa)) {
        if (sscanf(linha, "%[^;];%[^;];%[^\n]", nomeLido, email, pass) == 3) {
            if (strcmp(nomeLido, nome_utilizador) == 0) {
                fclose(fa);
                return 1;
            }
        }
    }
    
    fclose(fa);
    return 0;
}

// Painel de Administração Principal
void painelAdmin(const char *nome_user) {
    char opcao[10];
    while (1) {
        limparEcra();
        linha_h_topo();
        texto_centrado("PAINEL DE ADMINISTRAÇÃO");
        linha_h_meio();
        texto_esquerda("Admin: %s", nome_user);
        linha_h_meio();
        texto_esquerda(" 1. Gerir livros");
        texto_esquerda(" 2. Ver todas as transações do sistema");
        texto_esquerda(" 3. Estatísticas do sistema");
        texto_esquerda(" 4. Gerir utilizadores");
        linha_h_meio();
        texto_esquerda(" 0. Voltar ao menu principal                       ");
        linha_h_fim();
        printf("\nEscolha uma opção: ");
        fgets(opcao, sizeof(opcao), stdin);

        if (opcao[0] == '0') {
            break;
        } else if (opcao[0] == '1') {
            limparEcra();
            menuAdminLivros();
        } else if (opcao[0] == '2') {
            limparEcra();
            adminVerTodasTransacoes();
        } else if (opcao[0] == '3') {
            limparEcra();
            adminEstatisticas();
        } else if (opcao[0] == '4') {
            limparEcra();
            adminGerirUtilizadores();
        } else {
            opcaoInvalida();
            pausar();
        }
    }
}

// Ver todas as transações do sistema
void adminVerTodasTransacoes(void) {
    printf("╔════════════════════════════════════════════════════╗\n");
    printf("║        TODAS AS TRANSAÇÕES DO SISTEMA             ║\n");
    printf("╚════════════════════════════════════════════════════╝\n\n");

    // Carregar requisições
    FILE *fr = fopen("requisicoes.txt", "r");
    Requisicao *reqs = NULL; int nreq = 0, capreq = 10; char linhar[500];
    if (fr) {
        reqs = malloc((size_t)capreq * sizeof(Requisicao));
        if (reqs) {
            while (fgets(linhar, sizeof linhar, fr)) {
                if (nreq == capreq) { capreq*=2; Requisicao *tmp=realloc(reqs, (size_t)capreq*sizeof(Requisicao)); if(!tmp){free(reqs); reqs=NULL; break;} reqs=tmp; }
                sscanf(linhar, "%d;%[^;];%[^;];%[^;];%[^;];%[^;];%[^\n]", &reqs[nreq].id, reqs[nreq].titulo, reqs[nreq].autor, reqs[nreq].requester, reqs[nreq].owner, reqs[nreq].status, reqs[nreq].data);
                nreq++;
            }
        }
        fclose(fr);
    }

    // Carregar compras
    FILE *fc = fopen("compras.txt", "r");
    Compra *comps = NULL; int ncomp = 0, capcomp = 10; char linhac[500];
    if (fc) {
        comps = malloc((size_t)capcomp * sizeof(Compra));
        if (comps) {
            while (fgets(linhac, sizeof linhac, fc)) {
                if (ncomp == capcomp) { capcomp*=2; Compra *tmp=realloc(comps, (size_t)capcomp*sizeof(Compra)); if(!tmp){free(comps); comps=NULL; break;} comps=tmp; }
                sscanf(linhac, "%d;%[^;];%[^;];%[^;];%[^;];%f;%[^;];%[^\n]", &comps[ncomp].id, comps[ncomp].titulo, comps[ncomp].autor, comps[ncomp].comprador, comps[ncomp].vendedor, &comps[ncomp].preco, comps[ncomp].data, comps[ncomp].status);
                ncomp++;
            }
        }
        fclose(fc);
    }

    // Carregar trocas
    FILE *ft = fopen("trocas.txt", "r");
    Troca *trocas = NULL; int nt = 0, capt = 10; char linhat[500];
    if (ft) {
        trocas = malloc((size_t)capt * sizeof(Troca));
        if (trocas) {
            while (fgets(linhat, sizeof linhat, ft)) {
                if (nt == capt) { capt*=2; Troca *tmp=realloc(trocas, (size_t)capt*sizeof(Troca)); if(!tmp){free(trocas); trocas=NULL; break;} trocas=tmp; }
                sscanf(linhat, "%d;%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^\n]", &trocas[nt].id, trocas[nt].titulo1, trocas[nt].autor1, trocas[nt].titulo2, trocas[nt].autor2, trocas[nt].user1, trocas[nt].user2, trocas[nt].status, trocas[nt].data);
                nt++;
            }
        }
        fclose(ft);
    }

    printf("═══════════════ REQUISIÇÕES ═══════════════\n");
    if (reqs && nreq > 0) {
        for (int i = 0; i < nreq; i++) {
            printf("#%d | %s -> %s | \"%s\" | Status: %s | %s\n", 
                   reqs[i].id, reqs[i].requester, reqs[i].owner, 
                   reqs[i].titulo, reqs[i].status, reqs[i].data);
        }
    } else {
        printf("Nenhuma requisição encontrada.\n");
    }

    printf("\n═══════════════ COMPRAS ═══════════════\n");
    if (comps && ncomp > 0) {
        for (int i = 0; i < ncomp; i++) {
            printf("#%d | %s -> %s | \"%s\" | %.2f€ | Status: %s | %s\n", 
                   comps[i].id, comps[i].comprador, comps[i].vendedor, 
                   comps[i].titulo, comps[i].preco, comps[i].status, comps[i].data);
        }
    } else {
        printf("Nenhuma compra encontrada.\n");
    }

    printf("\n═══════════════ TROCAS ═══════════════\n");
    if (trocas && nt > 0) {
        for (int i = 0; i < nt; i++) {
            printf("#%d | %s ⇄ %s | \"%s\" ⇄ \"%s\" | Status: %s | %s\n", 
                   trocas[i].id, trocas[i].user1, trocas[i].user2,
                   trocas[i].titulo1, trocas[i].titulo2, trocas[i].status, trocas[i].data);
        }
    } else {
        printf("Nenhuma troca encontrada.\n");
    }

    free(reqs); free(comps); free(trocas);
    printf("\n");
    pausar();
}

// Estatísticas do sistema
void adminEstatisticas(void) {
    printf("╔════════════════════════════════════════════════════╗\n");
    printf("║         ESTATÍSTICAS DO SISTEMA                   ║\n");
    printf("╚════════════════════════════════════════════════════╝\n\n");

    // Contar livros
    int total_livros = 0, livros_disponiveis = 0, livros_indisponiveis = 0;
    FILE *fl = fopen("livros.txt", "r");
    if (fl) {
        char linha[500];
        while (fgets(linha, sizeof(linha), fl)) {
            Livro livro;
                if (!carregar_livro_de_linha(linha, &livro)) continue;
            total_livros++;
            if (livro.disponivel) livros_disponiveis++;
            else livros_indisponiveis++;
        }
        fclose(fl);
    }

    // Contar utilizadores
    int total_users = 0;
    FILE *fu = fopen("users.txt", "r");
    if (fu) {
        char linha[200];
        while (fgets(linha, sizeof(linha), fu)) {
            if (strlen(linha) > 5) total_users++;
        }
        fclose(fu);
    }

    // Contar transações
    int req_pendentes = 0, req_aceitas = 0, req_rejeitadas = 0;
    FILE *fr = fopen("requisicoes.txt", "r");
    if (fr) {
        char linha[500];
        while (fgets(linha, sizeof(linha), fr)) {
            char status[20];
            sscanf(linha, "%*d;%*[^;];%*[^;];%*[^;];%*[^;];%[^;]", status);
            if (strcmp(status, "Pendente") == 0) req_pendentes++;
            else if (strcmp(status, "Aceita") == 0) req_aceitas++;
            else if (strcmp(status, "Rejeitada") == 0) req_rejeitadas++;
        }
        fclose(fr);
    }

    int comp_pendentes = 0, comp_confirmadas = 0, comp_canceladas = 0;
    FILE *fc = fopen("compras.txt", "r");
    if (fc) {
        char linha[500];
        while (fgets(linha, sizeof(linha), fc)) {
            char status[20];
            sscanf(linha, "%*d;%*[^;];%*[^;];%*[^;];%*[^;];%*f;%*[^;];%[^\n]", status);
            if (strcmp(status, "Pendente") == 0) comp_pendentes++;
            else if (strcmp(status, "Confirmada") == 0) comp_confirmadas++;
            else if (strcmp(status, "Cancelada") == 0) comp_canceladas++;
        }
        fclose(fc);
    }

    int troca_pendentes = 0, troca_aceitas = 0, troca_rejeitadas = 0;
    FILE *ft = fopen("trocas.txt", "r");
    if (ft) {
        char linha[500];
        while (fgets(linha, sizeof(linha), ft)) {
            char status[20];
            sscanf(linha, "%*d;%*[^;];%*[^;];%*[^;];%*[^;];%*[^;];%*[^;];%[^;]", status);
            if (strcmp(status, "Pendente") == 0) troca_pendentes++;
            else if (strcmp(status, "Aceita") == 0) troca_aceitas++;
            else if (strcmp(status, "Rejeitada") == 0) troca_rejeitadas++;
        }
        fclose(ft);
    }

    printf("📚 LIVROS:\n");
    printf("   Total: %d livros\n", total_livros);
    printf("   Disponíveis: %d (%.1f%%)\n", livros_disponiveis, total_livros > 0 ? (livros_disponiveis * 100.0 / total_livros) : 0);
    printf("   Indisponíveis: %d (%.1f%%)\n\n", livros_indisponiveis, total_livros > 0 ? (livros_indisponiveis * 100.0 / total_livros) : 0);

    printf("👥 UTILIZADORES:\n");
    printf("   Total: %d utilizadores registados\n\n", total_users);

    printf("📋 REQUISIÇÕES:\n");
    printf("   Pendentes: %d\n", req_pendentes);
    printf("   Aceitas: %d\n", req_aceitas);
    printf("   Rejeitadas: %d\n\n", req_rejeitadas);

    printf("💰 COMPRAS:\n");
    printf("   Pendentes: %d\n", comp_pendentes);
    printf("   Confirmadas: %d\n", comp_confirmadas);
    printf("   Canceladas: %d\n\n", comp_canceladas);

    printf("🔄 TROCAS:\n");
    printf("   Pendentes: %d\n", troca_pendentes);
    printf("   Aceitas: %d\n", troca_aceitas);
    printf("   Rejeitadas: %d\n\n", troca_rejeitadas);

    pausar();
}

// Gerir utilizadores
void adminGerirUtilizadores(void) {
    while (1) {
        FILE *fu = fopen("users.txt", "r");
        if (!fu) {
            limparEcra();
            printf("Erro ao abrir ficheiro de utilizadores.\n");
            pausar();
            return;
        }

        Usuario *usuarios = NULL;
        int total = 0, capacidade = 10;
        char linha[200];

        usuarios = malloc((size_t)capacidade * sizeof(Usuario));
        if (!usuarios) {
            fclose(fu);
            return;
        }

        while (fgets(linha, sizeof(linha), fu)) {
            if (total == capacidade) {
                capacidade *= 2;
                Usuario *tmp = realloc(usuarios, (size_t)capacidade * sizeof(Usuario));
                if (!tmp) {
                    free(usuarios);
                    fclose(fu);
                    return;
                }
                usuarios = tmp;
            }
            if (sscanf(linha, "%[^;];%[^;];%[^\n]", usuarios[total].nome, usuarios[total].email, usuarios[total].password) == 3) {
                total++;
            }
        }
        fclose(fu);

        if (total == 0) {
            limparEcra();
            printf("Nenhum utilizador encontrado para gerir.\n");
            free(usuarios);
            pausar();
            return;
        }

        char (*nomes)[MAX_NOME] = malloc((size_t)total * MAX_NOME);
        if (!nomes) {
            free(usuarios);
            return;
        }

        for (int i = 0; i < total; i++) {
            strncpy(nomes[i], usuarios[i].nome, MAX_NOME - 1);
            nomes[i][MAX_NOME - 1] = '\0';
        }

        int escolha = menu_paginar_itens(nomes, total, 10, MAX_NOME, formatar_nome_usuario, header_admin_usuarios, NULL);
        if (escolha == 0) {
            free(nomes);
            free(usuarios);
            return;
        }

        char nomeSelecionado[MAX_NOME];
        strncpy(nomeSelecionado, nomes[escolha - 1], MAX_NOME - 1);
        nomeSelecionado[MAX_NOME - 1] = '\0';
        free(nomes);

        int idx = -1;
        for (int i = 0; i < total; i++) {
            if (strcmp(usuarios[i].nome, nomeSelecionado) == 0) {
                idx = i;
                break;
            }
        }

        if (idx == -1) {
            limparEcra();
            linha_h_topo();
            texto_centrado("ERRO");
            linha_h_meio();
            texto_esquerda("Utilizador não encontrado.");
            linha_h_fim();
            free(usuarios);
            pausar();
            continue;
        }
        limparEcra();
        linha_h_topo();
        texto_centrado("EDITAR UTILIZADOR");
        linha_h_meio();
        texto_esquerda("Nome: %s", usuarios[idx].nome);
        texto_esquerda("Email: %s", usuarios[idx].email);
        linha_h_meio();
        texto_esquerda("1. Alterar nome");
        texto_esquerda("2. Alterar password");
        texto_esquerda("3. Eliminar utilizador");
        texto_esquerda("0. Voltar à lista");
        linha_h_fim();
        printf("║ Escolha uma opção: ");
        int opcao;
        if (!ler_int_intervalo(&opcao, 0, 3)) {
            opcaoInvalida();
            free(usuarios);
            pausar();
            continue;
        }
        
        if (opcao == 0) {
            free(usuarios);
            continue;
        }
        
        if (opcao == 3) {
            // Eliminar utilizador
            limparEcra();
            linha_h_topo();
            texto_centrado("ELIMINAR UTILIZADOR");
            linha_h_meio();
            texto_esquerda("Utilizador: %s", usuarios[idx].nome);
            linha_h_meio();
            linha_h_fim();
            
            if (confirmar_sn("║ Tem a certeza?")) {
                FILE *tmp = fopen("users.tmp", "w");
                if (!tmp) {
                    printf("Erro ao criar ficheiro temporário.\n");
                    free(usuarios);
                    pausar();
                    continue;
                }
                
                for (int i = 0; i < total; i++) {
                    if (i != idx) {
                        fprintf(tmp, "%s;%s;%s\n", usuarios[i].nome, usuarios[i].email, usuarios[i].password);
                    }
                }
                fclose(tmp);
                
                remove("users.txt");
                rename("users.tmp", "users.txt");

                limparEcra();
                linha_h_topo();
                texto_centrado("SUCESSO");
                linha_h_meio();
                texto_esquerda("Utilizador eliminado.");
                linha_h_fim();
            } else {
                limparEcra();
                linha_h_topo();
                texto_centrado("CANCELADO");
                linha_h_meio();
                texto_esquerda("Operação cancelada.");
                linha_h_fim();
            }
            free(usuarios);
            pausar();
            continue;
        }
        
        char novo_valor[MAX_NOME];
        if (opcao == 1) {
            limparEcra();
            linha_h_topo();
            texto_centrado("ALTERAR NOME");
            linha_h_meio();
            linha_h_fim();
            printf("║ Novo nome: ");
            if (!ler_linha_sem_ponto_virgula(novo_valor, sizeof(novo_valor))) {
                free(usuarios);
                aviso_ponto_virgula_nao_permitido();
                pausar();
                continue;
            }
            strcpy(usuarios[idx].nome, novo_valor);
        } else if (opcao == 2) {
            limparEcra();
            linha_h_topo();
            texto_centrado("ALTERAR PASSWORD");
            linha_h_meio();
            linha_h_fim();
            printf("║ Nova password: ");
            if (!ler_linha_sem_ponto_virgula(novo_valor, sizeof(novo_valor))) {
                free(usuarios);
                aviso_ponto_virgula_nao_permitido();
                pausar();
                continue;
            }
            
            // Encriptar a nova password
            char encryptedPass[MAX_PASSWORD * 2];
            encryptPasswordHex(novo_valor, encryptedPass);
            strcpy(usuarios[idx].password, encryptedPass);
        } else {
            opcaoInvalida();
            free(usuarios);
            pausar();
            continue;
        }
        
        // Gravar alterações no ficheiro
        FILE *tmp = fopen("users.tmp", "w");
        if (!tmp) {
            printf("Erro ao criar ficheiro temporário.\n");
            free(usuarios);
            pausar();
            continue;
        }
        
        for (int i = 0; i < total; i++) {
            fprintf(tmp, "%s;%s;%s\n", usuarios[i].nome, usuarios[i].email, usuarios[i].password);
        }
        fclose(tmp);
        
        remove("users.txt");
        rename("users.tmp", "users.txt");

        limparEcra();
        linha_h_topo();
        texto_centrado("SUCESSO");
        linha_h_meio();
        texto_esquerda("Dados atualizados com sucesso.");
        linha_h_fim();
        free(usuarios);
        pausar();
    }
}
