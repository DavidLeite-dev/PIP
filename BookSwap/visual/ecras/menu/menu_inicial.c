#include "menu_inicial.h"
#include "../user/users.h"
#include "menu.h"
#include "../user/admin.h"
#include "util.h"
#include <stdio.h>

void mostrarMenuInicial(void) {
    limparEcra();
    int opcao;

    linha_h_topo();
    texto_centrado("BookSwap");
    texto_centrado("IPCA - 2026");
    linha_h_meio();
    texto_esquerda("1. Registar novo utilizador");
    texto_esquerda("2. Login");
    texto_esquerda("0. Sair");
    linha_h_fim();
    printf("\nEscolha uma opção: ");
    if (!ler_int_intervalo(&opcao, 0, 2)) {
        opcaoInvalida();
        return;
    }

    switch (opcao) {
        case 1: registerUser(); break;
        case 2:
            {
                int resultadoLogin = login();
                if (resultadoLogin == 2 || resultadoLogin == 1) {
                    menu_principal(f_nome);
                }
            }
            break;
        case 0:
            printf("\nObrigado por utilizar o sistema. Até breve!\n");
            utilizadorLogado = -1;
            break;
        default:
            opcaoInvalida();
            break;
    }
}