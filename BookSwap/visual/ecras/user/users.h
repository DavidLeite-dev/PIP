#ifndef USERS_H
#define USERS_H

#include <util.h>

/* Paths para ficheiros de utilizadores (relativo ao repositório) */
#define USERS_FILE "users.txt"
#define USERS_TMP  "users.tmp"
#define USERS_BAK  "users.bak"

/* Variáveis globais de autenticação */
extern int utilizadorLogado;
extern char f_nome[MAX_NOME];
extern char emailLogado[MAX_EMAIL];

/* Operações sobre o ficheiro de utilizadores */
void normalizeUsersFile(void);

/* Autenticação / gestão de utilizadores */
int login(void);
void logout(void);
void atualizarNomeLogado(void);
void registerUser(void);

/* Login e Registo */
/* Encriptação de passwords */
void encryptPasswordHex(const char *plainPassword, char *encryptedHex);
void decryptPasswordHex(const char *encryptedHex, char *plainPassword);

#endif /* USERS_H */
