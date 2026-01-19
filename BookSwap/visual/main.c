#include <stdio.h>
#include <locale.h>
#include <windows.h>
#include "ecras/user/users.h"
#include "ecras/menu/menu_inicial.h"
#include "ecras/menu/menu.h"
#include "../util/util.h"

int main(void) {
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    setlocale(LC_ALL, ".65001");

    // Falha cedo se não cumprir requisitos (silencioso quando está OK)
    if (!verificarRequisitosAmbiente()) {
        return 1;
    }

    /* Normalizar users.txt uma vez ao arrancar (cria users.bak) */
    normalizeUsersFile();

    while (utilizadorLogado >= 0) {
        if (utilizadorLogado == 0) {
            mostrarMenuInicial();
        } else {
            menu_principal(f_nome);
        }
    }
    return 0;
}
