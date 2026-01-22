#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "editar_dados.h"
#include "users.h"
#include "../menu/menu.h"
#include "util.h"

// Permite editar os dados da conta (nome ou password). As alterações são gravadas em users.txt.
void editarDadosPessoais(UserSession *session) {
    if (strlen(session->email) == 0) {
        printf("Nenhum utilizador logado. Faça login primeiro.\n");
        return;
    }

    limparEcra();
    linha_h_topo();
    texto_centrado("EDITAR DADOS PESSOAIS");
    linha_h_meio();
    texto_esquerda("1. Alterar nome");
    texto_esquerda("2. Alterar password");
    texto_esquerda("0. Voltar ao menu principal");
    linha_h_fim();
    printf("Escolha uma opção: ");
    int opcao;
    if (!ler_int_intervalo(&opcao, 0, 2)) {
        opcaoInvalida();
        return;
    }

    if (opcao == 0) return;

    char novo_nome[MAX_NOME] = "";
    char nova_password[MAX_PASSWORD] = "";

    if (opcao == 1) {
        limparEcra();
        linha_h_topo();
        texto_centrado("ALTERAR NOME");
        linha_h_meio();
        texto_esquerda("Escreva o novo nome.");
        linha_h_meio();
        printf("║ Novo nome: ");
        if (!ler_linha_sem_ponto_virgula(novo_nome, sizeof(novo_nome))) {
            aviso_ponto_virgula_nao_permitido();
            pausar();
            return;
        } else {
            fpopeec(1);
            texto_esquerda("Novo nome: %s", novo_nome);
            linha_h_fim();
            if (!confirmar_sn("Confirmar alteração?")) {
                limparEcra();
                texto_esquerda("Alteração cancelada pelo utilizador.");
                pausar();
                return;
            }
        }
        garantir_utf8(novo_nome, sizeof(novo_nome));
    } else if (opcao == 2) {
        limparEcra();
        linha_h_topo();
        texto_centrado("ALTERAR PASSWORD");
        linha_h_meio();
        texto_esquerda("Escreva a nova password.");
        linha_h_meio();
        printf("║ Nova password: ");
        if (!ler_linha_sem_ponto_virgula(nova_password, sizeof(nova_password))) {
            aviso_ponto_virgula_nao_permitido();
            pausar();
            return;
        } else {
            fpopeec(1);
            texto_esquerda("Nova password: %s", nova_password);
            linha_h_fim();
            if (!confirmar_sn("Confirmar alteração?")) {
                limparEcra();
                texto_esquerda("Alteração cancelada pelo utilizador.");
                pausar();
                return;
            }
        }
    } else {
        opcaoInvalida();
        return;
    }

    FILE *f = fopen(USERS_FILE, "r");
    if (f == NULL) {
        printf("Erro ao abrir ficheiro users.txt\n");
        return;
    }

    FILE *tmp = fopen(USERS_TMP, "w");
    if (tmp == NULL) {
        fclose(f);
        printf("Erro ao criar ficheiro temporário.\n");
        return;
    }

    char name[MAX_NOME], password[MAX_PASSWORD];
    int found = 0;
    char linha[256];
    while (fgets(linha, sizeof linha, f) != NULL) {
        size_t L = strlen(linha);
        while (L > 0 && (linha[L-1] == '\n' || linha[L-1] == '\r')) linha[--L] = '\0';
        if (sscanf(linha, "%49[^;];%19[^\\n]", name, password) == 2 ||
            sscanf(linha, "%49s %19s", name, password) == 2) {
            garantir_utf8(name, sizeof(name));
            if (strcmp(name, session->nome) == 0) {
                // Linha do utilizador atual: aplica alteração //
                found = 1;
                if (opcao == 1) {
                    strcpy(name, novo_nome);
                    strcpy(session->nome, novo_nome); // atualiza session //
                } else if (opcao == 2) {
                    /* Encriptar nova password */
                    char encryptedPass[MAX_PASSWORD * 2];
                    encryptPasswordHex(nova_password, encryptedPass);
                    strcpy(password, encryptedPass);
                }
                fprintf(tmp, "%s;%s\n", name, password);
            } else {
                fprintf(tmp, "%s;%s\n", name, password);
            }
        }
    }

    fclose(f);
    fclose(tmp);
    if (!found) {
        limparEcra();
        linha_h_topo();
        texto_centrado("ERRO");
        linha_vazia();
        texto_esquerda("Utilizador não encontrado no ficheiro.");
        linha_h_fim();
        pausar();
        remove(USERS_TMP);
        return;
    }

    // Substituir ficheiro original pelo temporário
    if (remove(USERS_FILE) != 0) {
        limparEcra();
        linha_h_topo();
        texto_centrado("ERRO");
        linha_vazia();
        texto_esquerda("Erro ao remover ficheiro original.");
        linha_h_fim();
        pausar();
        return;
    }
    if (rename(USERS_TMP, USERS_FILE) != 0) {
        limparEcra();
        linha_h_topo();
        texto_centrado("ERRO");
        linha_vazia();
        texto_esquerda("Erro ao renomear ficheiro temporário.");
        linha_h_fim();
        pausar();
        return;
    }

    limparEcra();
    linha_h_topo();
    texto_centrado("SUCESSO");
    linha_vazia();
    texto_esquerda("Dados atualizados com sucesso.");
    linha_h_fim();
    pausar();
}