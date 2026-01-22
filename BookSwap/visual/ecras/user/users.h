#ifndef USERS_H
#define USERS_H

#include <util.h>

/* Paths para ficheiros de utilizadores (relativo ao repositório) */
#define USERS_FILE "users.txt"
#define USERS_TMP  "users.tmp"
#define USERS_BAK  "users.bak"

/* Estrutura para sessão do utilizador */
typedef struct {
    int logado;  /* 0=não logado, 1=logado, -1=sair */
    char nome[MAX_NOME];
    char email[MAX_EMAIL];
} UserSession;

/* Operações sobre o ficheiro de utilizadores */
void normalizeUsersFile(void);

/* Autenticação / gestão de utilizadores */
int login(UserSession *session);
void logout(UserSession *session);
void atualizarNomeLogado(UserSession *session);
void registerUser(UserSession *session);

/* Login e Registo */
/* Encriptação de passwords */
void encryptPasswordHex(const char *plainPassword, char *encryptedHex);
void decryptPasswordHex(const char *encryptedHex, char *plainPassword);

#endif /* USERS_H */
