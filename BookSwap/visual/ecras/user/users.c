#include "users.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../menu/menu.h" /* Para mostrarMenu() */
#include "util.h"

#define ENCRYPT_KEY "IPCA2025SecureKey"

/* Função para encriptar/desencriptar password usando XOR */
static void encryptPassword(const char *input, char *output) {
    size_t keyLen = strlen(ENCRYPT_KEY);
    size_t inputLen = strlen(input);
    
    for (size_t i = 0; i < inputLen; i++) {
        output[i] = (char)(input[i] ^ ENCRYPT_KEY[i % keyLen]);
    }
    output[inputLen] = '\0';
}

/* Função para converter bytes para hex */
static void toHex(const char *input, char *output) {
    const char hexChars[] = "0123456789ABCDEF";
    size_t len = strlen(input);
    
    for (size_t i = 0; i < len; i++) {
        unsigned char byte = (unsigned char)input[i];
        output[i * 2] = hexChars[byte >> 4];
        output[i * 2 + 1] = hexChars[byte & 0x0F];
    }
    output[len * 2] = '\0';
}

/* Função para converter hex para bytes */
static void fromHex(const char *input, char *output) {
    size_t len = strlen(input);
    
    for (size_t i = 0; i + 1 < len; i += 2) {
        char hex[3] = {input[i], input[i + 1], '\0'};
        output[i / 2] = (char)strtol(hex, NULL, 16);
    }
    output[len / 2] = '\0';
}

/* Encripta password e retorna em formato hexadecimal */
void encryptPasswordHex(const char *plainPassword, char *encryptedHex) {
    char encrypted[MAX_PASSWORD];
    encryptPassword(plainPassword, encrypted);
    toHex(encrypted, encryptedHex);
}

/* Desencripta password de formato hexadecimal */
void decryptPasswordHex(const char *encryptedHex, char *plainPassword) {
    char encrypted[MAX_PASSWORD];
    fromHex(encryptedHex, encrypted);
    encryptPassword(encrypted, plainPassword); /* XOR é simétrico */
}

void normalizeUsersFile(void) {
    FILE *f = fopen(USERS_FILE, "r");
    if (f == NULL) return;

    char **validLines = NULL;
    int numLines = 0, capacity = 10;
    validLines = malloc((size_t)capacity * sizeof(char*));
    if (!validLines) {
        fclose(f);
        return;
    }

    char line[512];
    char name[MAX_NOME], email[MAX_EMAIL], pass[MAX_PASSWORD];
    while (fgets(line, sizeof line, f) != NULL) {
        /* Procura no ficheiro os dados de utilizador separados com ';' */
        if (sscanf(line, "%49[^;];%49[^;];%20[^\n]", name, email, pass) == 3) {
            if (numLines == capacity) {
                capacity *= 2;
                validLines = realloc(validLines, (size_t)capacity * sizeof(char*));
                if (!validLines) {
                    fclose(f);
                    return;
                }
            }
            validLines[numLines] = malloc(strlen(line) + 1);
            if (!validLines[numLines]) {
                fclose(f);
                return;
            }
            strcpy(validLines[numLines], line);
            numLines++;
        } else {
            /* Ignorar linhas inválidas ou vazias; aviso para o utilizador */
            int i, nonw = 0;
            for (i = 0; line[i]; ++i) {
                if (line[i] != ' ' && line[i] != '\t' && line[i] != '\r' && line[i] != '\n') { nonw = 1; break; }
            }
            if (nonw) printf("Aviso: linha de 'users.txt' ignorada (formato inválido): %s", line);
            pausar();
        }
    }

    fclose(f);

    /* Reescrever o ficheiro com linhas válidas */
    f = fopen(USERS_FILE, "w");
    if (f == NULL) {
        for (int i = 0; i < numLines; i++) free(validLines[i]);
        free(validLines);
        return;
    }

    for (int i = 0; i < numLines; i++) {
        fprintf(f, "%s", validLines[i]);
        free(validLines[i]);
    }
    free(validLines);

    fclose(f);

    printf("users.txt normalizado\n");
}

/* Verifica se o email existe no ficheiro IPCAlunos.txt */
static int verificarAlunoIPCA(const char *email) {
    FILE *f = fopen("IPCAlunos.txt", "r");
    if (f == NULL) {
        printf("Aviso: Ficheiro IPCAlunos.txt não encontrado.\n");
        pausar();
        return 0;
    }

    char linha[256];
    while (fgets(linha, sizeof linha, f) != NULL) {
        size_t L = strlen(linha);
        while (L > 0 && (linha[L-1] == '\n' || linha[L-1] == '\r')) linha[--L] = '\0';
        if (strcmp(linha, email) == 0) {
            fclose(f);
            return 1; /* Aluno encontrado */
        }
    }

    fclose(f);
    return 0; /* Aluno não encontrado */
}

/* Verifica se o email/nome está no ficheiro admin.txt e retorna 1 se for admin */
static int verificarAdmin(const char *email_ou_nome, const char *password) {
    FILE *fa = fopen("admin.txt", "r");
    if (!fa) return 0;
    
    char linha[200];
    char nome[MAX_NOME], admin_email[MAX_EMAIL], admin_pass[MAX_PASSWORD];
    
    /* Encriptar a password fornecida */
    char encryptedPass[MAX_PASSWORD * 2];
    encryptPasswordHex(password, encryptedPass);
    
    while (fgets(linha, sizeof(linha), fa)) {
        size_t L = strlen(linha);
        while (L > 0 && (linha[L-1] == '\n' || linha[L-1] == '\r')) linha[--L] = '\0';
        
            if (sscanf(linha, "%[^;];%[^;];%[^\n]", nome, admin_email, admin_pass) == 3) {
                garantir_utf8(nome, sizeof(nome));
                garantir_utf8(admin_email, sizeof(admin_email));
            /* Verificar se corresponde ao email ou nome */
            if ((strcmp(email_ou_nome, admin_email) == 0 || strcmp(email_ou_nome, nome) == 0) && 
                strcmp(encryptedPass, admin_pass) == 0) {
                fclose(fa);
                return 1; /* É admin */
            }
        }
    }
    
    fclose(fa);
    return 0;
}

/* --- Implementação do login/logout/registo --- */

int login(UserSession *session) {
    limparEcra();
    
    char email[MAX_EMAIL], password[MAX_PASSWORD], nome[MAX_NOME];

    printf("╔════════════════════════════════════════════════════╗\n");
    printf("║                    LOGIN                           ║\n");
    printf("╚════════════════════════════════════════════════════╝\n\n");
    while (1) {
        printf("Email/ID: ");
        if (ler_linha_sem_ponto_virgula(email, sizeof(email))) break;
        printf("Entrada inválida: ';' não é permitido.\n");
    }
    
    while (1) {
        printf("Password: ");
        if (ler_linha_sem_ponto_virgula(password, sizeof(password))) break;
        printf("Entrada inválida: ';' não é permitido.\n");
    }
    
    /* PRIMEIRO: Verificar se é ADMIN */
    if (verificarAdmin(email, password)) {
        /* Login como admin - buscar dados do ficheiro admin.txt */
        FILE *fa = fopen("admin.txt", "r");
        if (fa) {
            char linha[200];
            char nome_admin[MAX_NOME], email_admin[MAX_EMAIL], pass_admin[MAX_PASSWORD];
            
            while (fgets(linha, sizeof(linha), fa)) {
                size_t L = strlen(linha);
                while (L > 0 && (linha[L-1] == '\n' || linha[L-1] == '\r')) linha[--L] = '\0';
                
                if (sscanf(linha, "%[^;];%[^;];%[^\n]", nome_admin, email_admin, pass_admin) == 3) {
                    if (strcmp(email, email_admin) == 0 || strcmp(email, nome_admin) == 0) {
                        session->logado = 1;
                        strcpy(session->email, email_admin);
                        strcpy(session->nome, nome_admin);
                        fclose(fa);
                        
                        limparEcra();
                        printf("╔════════════════════════════════════════════════════╗\n");
                        printf("║         LOGIN ADMINISTRADOR BEM SUCEDIDO           ║\n");
                        printf("╚════════════════════════════════════════════════════╝\n\n");
                        printf("✓ Bem-vindo, Administrador %s!\n", nome_admin);
                        printf("  Acesso total ao sistema concedido.\n\n");
                        pausar();
                        return 2; /* Retorna 2 para indicar login de admin */
                    }
                }
            }
            fclose(fa);
        }
    }
    
    /* SEGUNDO: Verificar se o email está na lista de alunos IPCA */
    if (!verificarAlunoIPCA(email)) {
        limparEcra();
        printf("╔════════════════════════════════════════╗\n");
        printf("║          ACESSO NÃO AUTORIZADO         ║\n");
        printf("╚════════════════════════════════════════╝\n\n");
        printf("O email '%s' não está registado como aluno IPCA.\n", email);
        printf("Apenas alunos IPCA podem aceder ao sistema.\n\n");
        pausar();
        return 0;
    }
    
    /* TERCEIRO: Verificar se o utilizador já existe em users.txt */
    FILE *f = fopen(USERS_FILE, "r");
    int utilizadorExiste = 0;
    
    if (f != NULL) {
        char linha[256];
        char temp_nome[MAX_NOME], temp_email[MAX_EMAIL], temp_password[MAX_PASSWORD];
        
        while (fgets(linha, sizeof linha, f) != NULL) {
            size_t L = strlen(linha);
            while (L > 0 && (linha[L-1] == '\n' || linha[L-1] == '\r')) linha[--L] = '\0';
            
            if (sscanf(linha, "%49[^;];%49[^;];%19[^\\n]", temp_nome, temp_email, temp_password) == 3 ||
                sscanf(linha, "%49s %49s %19s", temp_nome, temp_email, temp_password) == 3) {
                garantir_utf8(temp_nome, sizeof(temp_nome));
                garantir_utf8(temp_email, sizeof(temp_email));
                garantir_utf8(temp_password, sizeof(temp_password));
                if (strcmp(email, temp_email) == 0) {
                    utilizadorExiste = 1;
                    fclose(f);
                    
                    /* Desencriptar password armazenada e comparar */
                    char decryptedPass[MAX_PASSWORD * 2];
                    decryptPasswordHex(temp_password, decryptedPass);
                    
                    if (strcmp(password, decryptedPass) == 0) {
                        session->logado = 1;
                        strcpy(session->email, temp_email);
                        strcpy(session->nome, temp_nome);
                        limparEcra();
                        printf("╔════════════════════════════════════════════════════╗\n");
                        printf("║            LOGIN BEM SUCEDIDO                      ║\n");
                        printf("╚════════════════════════════════════════════════════╝\n\n");
                        printf("✓ Bem-vindo, %s!\n\n", temp_nome);
                        pausar();
                        return 1;
                    } else {
                        limparEcra();
                        printf("╔════════════════════════════════════════╗\n");
                        printf("║         PASSWORD INCORRETA             ║\n");
                        printf("╚════════════════════════════════════════╝\n\n");
                        pausar();
                        return 0;
                    }
                }
            }
        }
        fclose(f);
    }
    
    /* Se não existe, criar nova conta */
    if (!utilizadorExiste) {
        limparEcra();
        printf("╔════════════════════════════════════════╗\n");
        printf("║        PRIMEIRA AUTENTICAÇÃO           ║\n");
        printf("╚════════════════════════════════════════╝\n\n");
        printf("Este email está autorizado mas ainda não tem conta.\n");
        printf("Vamos criar a sua conta agora.\n\n");
        
        while (1) {
            printf("Nome completo: ");
            if (ler_linha_sem_ponto_virgula(nome, sizeof(nome))) break;
            printf("Entrada inválida: ';' não é permitido.\n");
        }
        garantir_utf8(nome, sizeof(nome));
        
        while (1) {
            printf("Escolha uma password: ");
            if (ler_linha_sem_ponto_virgula(password, sizeof(password))) break;
            printf("Entrada inválida: ';' não é permitido.\n");
        }
        
        /* Encriptar password antes de guardar */
        char encryptedPass[MAX_PASSWORD * 2];
        encryptPasswordHex(password, encryptedPass);
        
        /* Adicionar ao ficheiro users.txt */
        f = fopen(USERS_FILE, "a");
        if (f == NULL) {
            printf("Erro ao criar conta.\n");
            pausar();
            return 0;
        }
        
        fprintf(f, "%s;%s;%s\n", nome, email, encryptedPass);
        fclose(f);
        
        session->logado = 1;
        strcpy(session->email, email);
        strcpy(session->nome, nome);
        
        limparEcra();
        printf("╔════════════════════════════════════════════════════╗\n");
        printf("║        CONTA CRIADA COM SUCESSO                    ║\n");
        printf("╚════════════════════════════════════════════════════╝\n\n");
        printf("✓ Bem-vindo ao sistema, %s!\n\n", nome);
        pausar();
        return 1;
    }
    
    limparEcra();
    printf("╔════════════════════════════════════════╗\n");
    printf("║            ERRO NO LOGIN               ║\n");
    printf("╚════════════════════════════════════════╝\n\n");
    pausar();
    return 0;
}

void logout(UserSession *session) {
    limparEcra();
    session->logado = 0;
    strcpy(session->nome, "");
    strcpy(session->email, "");
    printf("Logout efetuado com sucesso.\n");
}

void atualizarNomeLogado(UserSession *session) {
    if (strlen(session->email) == 0) return;

    FILE *f = fopen(USERS_FILE, "r");
    if (f == NULL) return;

    char file_name[MAX_NOME], file_email[MAX_EMAIL], file_password[MAX_PASSWORD];

    char linha[256];
    while (fgets(linha, sizeof linha, f) != NULL) {
        size_t L = strlen(linha);
        while (L > 0 && (linha[L-1] == '\n' || linha[L-1] == '\r')) linha[--L] = '\0';
        if (sscanf(linha, "%49[^;];%49[^;];%19[^\\n]", file_name, file_email, file_password) == 3 ||
            sscanf(linha, "%49s %49s %19s", file_name, file_email, file_password) == 3) {
            garantir_utf8(file_name, sizeof(file_name));
            garantir_utf8(file_email, sizeof(file_email));
            if (strcmp(file_email, session->email) == 0) {
                strcpy(session->nome, file_name);
                break;
            }
        }
    }

    fclose(f);
}

void registerUser(UserSession *session) {
    limparEcra();
    
    char email[MAX_EMAIL];
    printf("=== REGISTO DE NOVO UTILIZADOR ===\n");
    while (1) {
        printf("Email: ");
        if (ler_linha_sem_ponto_virgula(email, sizeof(email))) break;
        printf("Entrada inválida: ';' não é permitido.\n");
    }

    /* Verificar se o email existe no ficheiro IPCAlunos.txt */
    if (!verificarAlunoIPCA(email)) {
        limparEcra();
        printf("╔════════════════════════════════════════╗\n");
        printf("║          ACESSO NÃO AUTORIZADO         ║\n");
        printf("╚════════════════════════════════════════╝\n\n");
        printf("O email '%s' não está registado como aluno IPCA.\n", email);
        printf("\nPara criar uma conta, por favor dirija-se a um trabalhador\n");
        printf("pessoalmente no departamento administrativo.\n\n");
        pausar();
        return;
    }

    FILE *f = fopen(USERS_FILE, "a+");
    if (f == NULL) {
        printf("Erro ao abrir ficheiro de utilizadores.\n");
        return;
    }

    char nome[MAX_NOME], password[MAX_PASSWORD];
    char ficheiro_nome[MAX_NOME], f_email[MAX_EMAIL], f_password[MAX_PASSWORD];

    while (1) {
        printf("Nome completo: ");
        if (ler_linha_sem_ponto_virgula(nome, sizeof(nome))) break;
        printf("Entrada inválida: ';' não é permitido.\n");
    }
    garantir_utf8(nome, sizeof(nome));
    while (1) {
        printf("Password: ");
        if (ler_linha_sem_ponto_virgula(password, sizeof(password))) break;
        printf("Entrada inválida: ';' não é permitido.\n");
    }

    /* Encriptar password */
    char encryptedPass[MAX_PASSWORD * 2];
    encryptPasswordHex(password, encryptedPass);

    rewind(f);

    char linha[256];
    while (fgets(linha, sizeof linha, f) != NULL) {
        size_t L = strlen(linha);
        while (L > 0 && (linha[L-1] == '\n' || linha[L-1] == '\r')) linha[--L] = '\0';
        if (sscanf(linha, "%49[^;];%49[^;];%19[^\\n]", ficheiro_nome, f_email, f_password) == 3 ||
            sscanf(linha, "%49s %49s %19s", ficheiro_nome, f_email, f_password) == 3) {
            garantir_utf8(ficheiro_nome, sizeof(ficheiro_nome));
            garantir_utf8(f_email, sizeof(f_email));
            if (strcmp(email, f_email) == 0) {
                fclose(f);
                printf("\nErro: Já existe um utilizador com o email %s!\n", email);
                pausar();
                return;
            }
        }
    }

    fprintf(f, "%s;%s;%s\n", nome, email, encryptedPass);
    fclose(f);

    printf("\nUtilizador registado com sucesso!\n");
    printf("Login automático efetuado. Bem-vindo, %s!\n", nome);

    session->logado = 1;
    strcpy(session->email, email);
    strcpy(session->nome, nome);

    menu_principal(session);  // direto para menu principal
}
