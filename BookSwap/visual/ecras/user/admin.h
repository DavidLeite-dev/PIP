#ifndef ADMIN_H
#define ADMIN_H

// Verificação de permissões
int isAdmin(const char *nome_utilizador);

// Painel de administração completo
void painelAdmin(const char *nome_user);
void adminVerTodasTransacoes(void);
void adminEstatisticas(void);
void adminGerirUtilizadores(void);

#endif /* ADMIN_H */
