#include "menu.h"
#include "../user/users.h"
#include "../user/editar_dados.h"
#include "../livros/livros.h"
#include "../livros/transacoes.h"
#include "../user/admin.h"
#include "../../../util/util.h"
#include <stdio.h>

void menu_principal(const char *nome_utilizador) {
    limparEcra();
    atualizarNomeLogado();
    int opcao;

    linha_h_topo();
    texto_centrado("MENU PRINCIPAL");
    linha_h_meio();
    texto_esquerda("Utilizador: %s", nome_utilizador);
    linha_h_meio();
    texto_esquerda("1. Consultar catálogo de livros");
    texto_esquerda("2. Registar livro");
    texto_esquerda("3. Meus livros");
    texto_esquerda("4. Consultar os meus pedidos");
    texto_esquerda("5. Transações  ");
    texto_esquerda("6. Alterar dados pessoais");
    if (isAdmin(nome_utilizador)) {
        texto_esquerda("7. [ADMIN] Painel de Administração");
    }

    linha_h_meio();

    texto_esquerda("9. Logout");
    texto_esquerda("0. Sair do programa");

    linha_h_fim();
    printf("Escolha uma opção: ");
    if (!ler_int_intervalo(&opcao, 0, 9)) {
        opcaoInvalida();
        return;
    }

    switch (opcao) {
        case 1: limparEcra(); consultarLivros(); break;
        case 2: limparEcra(); adicionarLivro(); break;
        case 3: limparEcra(); listarMeusLivros(); break;
        case 4: limparEcra(); consultarMeusPedidos(); break;
        case 5: limparEcra(); menuTransacoesUsuario(); break;
        case 6: limparEcra(); editarDadosPessoais(); break;
        case 7: limparEcra(); painelAdmin(); break;
        case 9: logout(); break;
        case 0:
            limparEcra();
            printf("Obrigado por utilizar o sistema. Até breve!\n");
            utilizadorLogado = -1;
            break;
        default:
            opcaoInvalida();
            break;
    }
}