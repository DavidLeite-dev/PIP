#ifdef _WIN32
#include <windows.h>
#endif
#include "util.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <limits.h>

#define WIDTH 60

void limparBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void limparEcra(void) {
    system("cls"); // No Windows usamos "cls"; noutros sistemas pode ser "clear".
}

void pausar(void) {
    printf("\nPrima Enter para continuar...");
    limparBuffer();
}

void opcaoInvalida(void) {
    limparEcra();
    linha_h_topo();
    texto_centrado("OPÇÃO INVÁLIDA!");
    texto_centrado("Por favor, tente novamente.");
    linha_h_fim();
    return;
}

int contem_ponto_virgula(const char *texto) {
    return (texto && strchr(texto, ';') != NULL) ? 1 : 0;
}

void aviso_ponto_virgula_nao_permitido(void) {
    limparEcra();
    linha_h_topo();
    texto_centrado("ENTRADA INVÁLIDA");
    linha_h_meio();
    texto_esquerda("O carácter ';' não é permitido.");
    linha_h_fim();
}

int ler_linha_sem_ponto_virgula(char *destino, size_t destinoTamanho) {
    if (!destino || destinoTamanho == 0) return 0;
    int tamanho = (destinoTamanho > (size_t)INT_MAX) ? INT_MAX : (int)destinoTamanho;
    if (tamanho < 2) return 0;
    if (!fgets(destino, tamanho, stdin)) return 0;
    destino[strcspn(destino, "\r\n")] = '\0';
    if (destino[0] == '\0') return 0;
    if (contem_ponto_virgula(destino)) return 0;
    garantir_utf8(destino, destinoTamanho);
    return 1;
}

int ler_char_sem_ponto_virgula(char *destino) {
    if (!destino) return 0;

    char buf[64];
    if (!fgets(buf, sizeof(buf), stdin)) return 0;

    size_t i = 0;
    while (buf[i] && isspace((unsigned char)buf[i])) i++;
    if (!buf[i]) return 0;
    if (buf[i] == ';') return 0;

    *destino = buf[i];
    return 1;
}

int ler_int_intervalo(int *destino, int min, int max) {
    if (!destino) return 0;

    char buf[64];
    if (!fgets(buf, sizeof(buf), stdin)) return 0;
    buf[strcspn(buf, "\r\n")] = '\0';
    if (buf[0] == '\0') return 0;
    if (contem_ponto_virgula(buf)) return 0;

    char *endptr;
    long v = strtol(buf, &endptr, 10);
    while (*endptr != '\0' && isspace((unsigned char)*endptr)) endptr++;
    if (*endptr != '\0') return 0;
    if (v < min || v > max) return 0;

    *destino = (int)v;
    return 1;
}

int ler_float_intervalo(float *destino, float min, float max) {
    if (!destino) return 0;

    char buf[64];
    if (!fgets(buf, sizeof(buf), stdin)) return 0;
    buf[strcspn(buf, "\r\n")] = '\0';
    if (buf[0] == '\0') return 0;
    if (contem_ponto_virgula(buf)) return 0;

    for (char *p = buf; *p; ++p) {
        if (*p == ',') *p = '.';
    }

    char *endptr;
    float v = strtof(buf, &endptr);
    while (*endptr != '\0' && isspace((unsigned char)*endptr)) endptr++;
    if (*endptr != '\0') return 0;
    if (v < min || v > max) return 0;

    *destino = v;
    return 1;
}

int confirmar_sn(const char *mensagem) {
    char input[10];
    while (1) {
        printf("%s (S/N): ", mensagem);
        if (!fgets(input, sizeof(input), stdin)) {
            continue;
        }
        input[strcspn(input, "\r\n")] = '\0';
        
        if (input[0] == '\0') {
            opcaoInvalida();
            continue;
        }
        
        char c = (char)tolower((unsigned char)input[0]);
        if (c == 's' && input[1] == '\0') {
            return 1;
        } else if (c == 'n' && input[1] == '\0') {
            return 0;
        } else {
            opcaoInvalida();
        }
    }
}

void linha_h_topo() {
    printf("╔");
    for (int i = 0; i < WIDTH - 2; i++) {
        printf("═");
    }
    printf("╗\n");
}

void linha_h_fim() {
    printf("╚");
    for (int i = 0; i < WIDTH - 2; i++) {
        printf("═");
    }
    printf("╝\n");
}
void linha_h_meio() {
    printf("╠");
    for (int i = 0; i < WIDTH - 2; i++) {
        printf("═");
    }
    printf("╣\n");
}

void linha_vazia() {
    printf("║");
    for (int i = 0; i < WIDTH - 2; i++) {
        printf(" ");
    }
    printf("║\n");
}

static int utf8_length(const char *texto) {
    if (!texto) return 0;
    int count = 0;
    size_t i = 0;
    while (texto[i] != '\0') {
        unsigned char c = (unsigned char)texto[i];
        size_t advance = 1;
        if ((c & 0xE0) == 0xC0) {
            advance = 2;
        } else if ((c & 0xF0) == 0xE0) {
            advance = 3;
        } else if ((c & 0xF8) == 0xF0) {
            advance = 4;
        } else if ((c & 0xFC) == 0xF8) {
            advance = 5;
        } else if ((c & 0xFE) == 0xFC) {
            advance = 6;
        }
        i += advance;
        count++;
    }
    return count;
}

void texto_centrado(const char *formato, ...) {
    char buffer[WIDTH * 2];
    va_list args;

    va_start(args, formato);
    vsnprintf(buffer, sizeof(buffer), formato, args);
    va_end(args);

    int len = utf8_length(buffer);
    int espacos = (WIDTH - 2 - len) / 2;
    if (espacos < 0) espacos = 0;

    printf("║");
    for (int i = 0; i < espacos; i++) printf(" ");
    printf("%s", buffer);
    for (int i = 0; i < WIDTH - 2 - espacos - len; i++) printf(" ");
    printf("║\n");
}

void texto_esquerda(const char *formato, ...) {
    char buffer[WIDTH * 2];
    va_list args;

    va_start(args, formato);
    vsnprintf(buffer, sizeof(buffer), formato, args);
    va_end(args);

    int len = utf8_length(buffer);
    int spaces = WIDTH - 4 - len;
    if (spaces < 0) spaces = 0;

    printf("║ %s", buffer);
    for (int i = 0; i < spaces; i++) printf(" ");
    printf(" ║\n");
}

static int texto_ja_e_utf8(const char *texto) {
    if (!texto) return 0;
    const unsigned char *bytes = (const unsigned char *)texto;
    while (*bytes) {
        unsigned char c = *bytes;
        if (c <= 0x7F) {
            bytes++;
            continue;
        }

        int esperado = 0;
        if ((c & 0xE0) == 0xC0) {
            esperado = 1;
        } else if ((c & 0xF0) == 0xE0) {
            esperado = 2;
        } else if ((c & 0xF8) == 0xF0) {
            esperado = 3;
        } else {
            return 0;
        }

        bytes++;
        for (int i = 0; i < esperado; ++i) {
            if (bytes[i] == '\0' || (bytes[i] & 0xC0) != 0x80) {
                return 0;
            }
        }
        bytes += esperado;
    }
    return 1;
}

void garantir_utf8(char *texto, size_t tamanho) {
    if (!texto || tamanho == 0) return;
    if (texto_ja_e_utf8(texto)) return;

#ifdef _WIN32
    int wide_len = MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, texto, -1, NULL, 0);
    if (wide_len == 0) return;

    wchar_t *wide = malloc((size_t)wide_len * sizeof(wchar_t));
    if (!wide) return;
    if (!MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, texto, -1, wide, wide_len)) {
        free(wide);
        return;
    }

    int utf8_len = WideCharToMultiByte(CP_UTF8, 0, wide, -1, NULL, 0, NULL, NULL);
    if (utf8_len == 0) {
        free(wide);
        return;
    }

    char *temp = malloc((size_t)utf8_len);
    if (!temp) {
        free(wide);
        return;
    }
    WideCharToMultiByte(CP_UTF8, 0, wide, -1, temp, utf8_len, NULL, NULL);

    if (tamanho > 0) {
        strncpy(texto, temp, tamanho - 1);
        texto[tamanho - 1] = '\0';
    }

    free(temp);
    free(wide);
#else
    (void)tamanho; // assume locale already UTF-8 on non-Windows
#endif
}

int menu_paginar_itens(void *itens, int total, int porPagina, size_t itemSize, MenuItemFormatter formatter, MenuPageHeader header, void *context) {
    if (!itens || total <= 0 || porPagina <= 0 || itemSize == 0 || !formatter) return 0;

    char input[32];
    int pagina = 0;
    int totalPaginas = (total + porPagina - 1) / porPagina;

    while (1) {
        int inicio = pagina * porPagina;
        int fim = inicio + porPagina;
        if (fim > total) fim = total;

        limparEcra();
        if (header) header(pagina + 1, totalPaginas, context);

        for (int i = inicio; i < fim; i++) {
            char linha[WIDTH * 2];
            formatter(linha, sizeof(linha), i, (char *)itens + (size_t)i * itemSize, context);
            texto_esquerda("%2d. %s", i + 1, linha);
        }

        texto_esquerda("(P) Próxima | (A) Anterior | (0) Cancelar");
        linha_h_fim();
        printf("║Escolha uma opção: ");
        if (!fgets(input, sizeof(input), stdin)) continue;
        input[strcspn(input, "\r\n")] = '\0';
        if (input[0] == '\0') continue;

        if (input[0] == '0' && input[1] == '\0') {
            return 0;
        }
        if ((input[0] == 'p' || input[0] == 'P') && fim < total) {
            pagina++;
            continue;
        }
        if ((input[0] == 'a' || input[0] == 'A') && pagina > 0) {
            pagina--;
            continue;
        }

        char *endptr;
        long escolha = strtol(input, &endptr, 10);
        if (*endptr == '\0' && escolha >= 1 && escolha <= total) {
            return (int)escolha;
        }

        texto_esquerda("Opção inválida.");
    }
}

static int textos_iguais(const char *a, const char *b) {
    if (!a || !b) return 0;
    while (*a && *b) {
        unsigned char ca = (unsigned char) tolower((unsigned char)*a);
        unsigned char cb = (unsigned char) tolower((unsigned char)*b);
        if (ca != cb) return 0;
        a++; b++;
    }
    return *a == '\0' && *b == '\0';
}

static int obter_id_por_ficheiro(const char *arquivo, const char *valor) {
    if (!valor) return 0;
    char valor_limpo[MAX_AUTOR + 1];
    strncpy(valor_limpo, valor, sizeof(valor_limpo) - 1);
    valor_limpo[sizeof(valor_limpo) - 1] = '\0';
    valor_limpo[strcspn(valor_limpo, "\r\n")] = '\0';
    garantir_utf8(valor_limpo, sizeof(valor_limpo));

    FILE *fp = fopen(arquivo, "a+");
    if (!fp) return 0;
    rewind(fp);

    char linha[256];
    int max_id = 0;
    while (fgets(linha, sizeof(linha), fp)) {
        char *sep = strchr(linha, ';');
        if (!sep) continue;
        *sep = '\0';
        int id = atoi(linha);
        char *nome = sep + 1;
        nome[strcspn(nome, "\r\n")] = '\0';
        if (id > max_id) max_id = id;
        garantir_utf8(nome, strlen(nome) + 1);
        if (textos_iguais(nome, valor_limpo)) {
            fclose(fp);
            return id;
        }
    }

    int novo_id = max_id + 1;
    fprintf(fp, "%d;%s\n", novo_id, valor_limpo);
    fclose(fp);
    return novo_id;
}

static const char *traduzir_por_ficheiro(const char *arquivo, int id, char *buffer, size_t bufsize) {
    if (id <= 0) return "Desconhecido";
    FILE *fp = fopen(arquivo, "r");
    if (!fp) return "Desconhecido";

    char linha[256];
    while (fgets(linha, sizeof(linha), fp)) {
        char *sep = strchr(linha, ';');
        if (!sep) continue;
        *sep = '\0';
        int item_id = atoi(linha);
        char *nome = sep + 1;
        nome[strcspn(nome, "\r\n")] = '\0';
        if (item_id == id) {
            garantir_utf8(nome, strlen(nome) + 1);
            strncpy(buffer, nome, bufsize - 1);
            buffer[bufsize - 1] = '\0';
            fclose(fp);
            return buffer;
        }
    }

    fclose(fp);
    return "Desconhecido";
}

const char *traduzir_condicao(int condicao) {
    switch (condicao) {
        case CONDICAO_NOVO:
            return "Novo";
        case CONDICAO_BOM:
            return "Bom";
        case CONDICAO_RAZOAVEL:
            return "Razoável";
        default:
            return "Desconhecida";
    }
}

int obter_id_autor(const char *autor) {
    return obter_id_por_ficheiro(ARQUIVO_AUTORES, autor);
}

const char *traduzir_autor(int id) {
    static char buffer[MAX_AUTOR + 1];
    return traduzir_por_ficheiro(ARQUIVO_AUTORES, id, buffer, sizeof(buffer));
}

int obter_id_categoria(const char *categoria) {
    return obter_id_por_ficheiro(ARQUIVO_CATEGORIAS, categoria);
}

const char *traduzir_categoria(int id) {
    static char buffer[MAX_CATEGORIA + 1];
    return traduzir_por_ficheiro(ARQUIVO_CATEGORIAS, id, buffer, sizeof(buffer));
}

void fpopeec(int linhas) {
    // Apaga as últimas N linhas no terminal (ANSI). Útil para simular “submenus” e atualizar texto sem poluir o ecrã.
    for (int i = 0; i < linhas; ++i) {
        printf("\x1b[1A"); // Sobe uma linha
        printf("\x1b[2K"); // Limpa a linha inteira
    }
}

static int obter_versao_windows(int *major, int *minor, int *build) {
#ifdef _WIN32
    if (major) *major = 0;
    if (minor) *minor = 0;
    if (build) *build = 0;

    typedef struct {
        unsigned long dwOSVersionInfoSize;
        unsigned long dwMajorVersion;
        unsigned long dwMinorVersion;
        unsigned long dwBuildNumber;
        unsigned long dwPlatformId;
        wchar_t szCSDVersion[128];
    } RTL_OSVERSIONINFOW_Compat;

    typedef long (WINAPI *RtlGetVersionPtr)(RTL_OSVERSIONINFOW_Compat *);

    HMODULE ntdll = GetModuleHandleA("ntdll.dll");
    if (!ntdll) return 0;

    RtlGetVersionPtr rtlGetVersion = (RtlGetVersionPtr)GetProcAddress(ntdll, "RtlGetVersion");
    if (!rtlGetVersion) return 0;

    RTL_OSVERSIONINFOW_Compat info;
    memset(&info, 0, sizeof(info));
    info.dwOSVersionInfoSize = (unsigned long)sizeof(info);
    if (rtlGetVersion(&info) != 0) return 0;

    if (major) *major = (int)info.dwMajorVersion;
    if (minor) *minor = (int)info.dwMinorVersion;
    if (build) *build = (int)info.dwBuildNumber;
    return 1;
#else
    (void)major;
    (void)minor;
    (void)build;
    return 0;
#endif
}

int verificarRequisitosAmbiente(void) {
#ifdef _WIN32
    int major = 0, minor = 0, build = 0;
    int temVersao = obter_versao_windows(&major, &minor, &build);

    // Se não conseguirmos detetar a versão, não bloqueamos o utilizador.
    if (!temVersao) return 1;

    if (major >= 10) return 1;

    limparEcra();
    linha_h_topo();
    texto_centrado("ERRO: REQUISITOS NÃO CUMPRIDOS");
    linha_h_meio();
    texto_esquerda("Este programa foi feito para Windows 10/11.");
    texto_esquerda("Sistema detetado: Windows %d.%d (build %d)", major, minor, build);
    linha_h_meio();
    texto_esquerda("Atualize o Windows para continuar.");
    linha_h_fim();
    pausar();
    return 0;
#else
    limparEcra();
    linha_h_topo();
    texto_centrado("ERRO: REQUISITOS NÃO CUMPRIDOS");
    linha_h_meio();
    texto_esquerda("Este programa foi feito para Windows 10/11.");
    linha_h_meio();
    texto_esquerda("Execute o programa num PC com Windows.");
    linha_h_fim();
    pausar();
    return 0;
#endif
}