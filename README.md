# Plataforma de Gestão de Livros (BookSwap)

> Sistema de troca, empréstimo e venda de livros entre alunos do IPCA

![Linguagem](https://img.shields.io/badge/Linguagem-C-blue)
![Plataforma](https://img.shields.io/badge/Plataforma-Windows-lightgrey)
![Versão](https://img.shields.io/badge/Versão-1.0-green)
![Disciplina](https://img.shields.io/badge/Disciplina-PIP-orange)

---

## Sobre o Projeto

A **BookSwap** é uma aplicação de consola desenvolvida em C que permite aos alunos do IPCA gerir e partilhar livros entre si. O sistema suporta três tipos de transações: **requisições** (empréstimos), **compras** e **trocas**.

### Funcionalidades Principais

| Funcionalidade | Descrição |
|----------------|-----------|
| **Autenticação** | Login e registo com passwords encriptadas (XOR + Hex) |
| **Catálogo** | Consultar, pesquisar e filtrar livros por disponibilidade |
| **Registar Livros** | Adicionar livros ao catálogo com título, autor, categoria e condição |
| **Requisições** | Pedir livros emprestados a outros utilizadores |
| **Compras** | Comprar livros de outros utilizadores |
| **Trocas** | Propor trocas de livros entre utilizadores |
| **Perfil** | Editar dados pessoais e alterar password |
| **Administração** | Painel de admin para gerir utilizadores, livros e estatísticas |

---

## Começar

### Pré-requisitos

- **Windows 10** ou superior
- **GCC** (MinGW ou MSYS2) adicionado ao PATH
- **PowerShell** ou Command Prompt

---

## Contas de demonstração

### Admin (2 contas)

- `admin@ipca.pt` / password: `admin123`
- `admin2@ipca.pt` / password: `admin456`

### Utilizadores (10 contas)

- `a20001@alunos.ipca.pt` / password: `ana20001`
- `a20002@alunos.ipca.pt` / password: `bruno20002`
- `a20003@alunos.ipca.pt` / password: `carla20003`
- `a20004@alunos.ipca.pt` / password: `diogo20004`
- `a20005@alunos.ipca.pt` / password: `eva20005`
- `a20006@alunos.ipca.pt` / password: `filipe20006`
- `a20007@alunos.ipca.pt` / password: `gabi20007`
- `a20008@alunos.ipca.pt` / password: `hugo20008`
- `a20009@alunos.ipca.pt` / password: `ines20009`
- `a20010@alunos.ipca.pt` / password: `joao20010`

### Email autorizado mas não registado

- `email@alunos.ipca.pt` está em `IPCAlunos.txt` mas **não existe em `users.txt`**.
- Ao fazer login com este email, o sistema cria a conta na primeira autenticação.

### Instalação

1. **Clonar ou copiar** o repositório
2. **Navegar** até à pasta do projeto:
   ```powershell
   cd BookSwap
   ```

3. **Compilar** usando o script:
   ```powershell
   .\compilar.bat
   ```
   
   Ou manualmente:
   ```powershell
   gcc -Wall -Wextra -I./util -I./visual -I./visual/ecras/user -I./visual/ecras/menu -I./visual/ecras/livros util/*.c visual/main.c visual/ecras/user/*.c visual/ecras/menu/*.c visual/ecras/livros/*.c -o programa.exe
   ```

4. **Executar**:
   ```powershell
   .\programa.exe
   ```

---

## Exemplo de Utilização

```
╔══════════════════════════════════════════════════════════╗
║                                                          ║
║       SISTEMA DE TROCA E EMPRÉSTIMO DE LIVROS           ║
║                    IPCA - 2025                           ║
║                                                          ║
╠══════════════════════════════════════════════════════════╣
║ 1. Registar novo utilizador                              ║
║ 2. Login                                                 ║
║ 0. Sair                                                  ║
║                                                          ║
╚══════════════════════════════════════════════════════════╝
```

---

## Estrutura do Projeto

```
BookSwap/
├── util/                       # Funções utilitárias
│   ├── util.c                  # Implementação (UI, parsing, etc.)
│   └── util.h                  # Definições e constantes
│
├── visual/                     # Interface do utilizador
│   ├── main.c                  # Ponto de entrada
│   └── ecras/
│       ├── user/               # Gestão de utilizadores
│       │   ├── users.c/h       # Login, registo, encriptação
│       │   ├── admin.c/h       # Painel de administração
│       │   └── editar_dados.c/h
│       ├── menu/               # Navegação
│       │   ├── menu.c/h        # Menu principal
│       │   └── menu_inicial.c/h
│       └── livros/             # Gestão de livros
│           ├── livros.c/h      # CRUD de livros
│           └── transacoes.c/h  # Requisições, compras, trocas
│
├── *.txt                       # Ficheiros de dados
└── compilar.bat                # Script de compilação
```

---

## Ficheiros de Dados

| Ficheiro | Formato | Descrição |
|----------|---------|-----------|
| `users.txt` | `nome;email;password` | Utilizadores registados |
| `admin.txt` | `nome;email;password` | Administradores do sistema |
| `livros.txt` | `titulo;autor_id;owner;categoria_id;condicao_id;disponivel` | Catálogo de livros (IDs traduzidos via `autores.txt` e `categorias.txt`) |
| `requisicoes.txt` | `id;titulo;autor;requester;owner;status;data` | Pedidos de empréstimo |
| `compras.txt` | `id;titulo;autor;comprador;vendedor;preco;data;status` | Histórico de compras |
| `trocas.txt` | `id;titulo1;autor1;titulo2;autor2;user1;user2;status;data` | Histórico de trocas |
| `autores.txt` | `id;nome` | Lista de autores |
| `categorias.txt` | `id;nome` | Lista de categorias |
| `IPCAlunos.txt` | `email` | Emails autorizados a registar |

---

## Regras importantes (integridade e consistência)

### Input do utilizador

- O projeto guarda dados em ficheiros com separador `;`.
- Por isso, **qualquer input do utilizador não pode conter `;`** (para não corromper o formato dos `.txt`).
- A leitura de números foi reforçada: em vez de `scanf`, usamos leitura por linha (`fgets`) e conversão (`strtol`/`strtof`) para evitar problemas de buffer e validar intervalos.

### Regras das transações (o que acontece ao “Aceitar”)

- **Requisição (empréstimo)**: não muda a posse; ao ser aceite, o livro fica **indisponível**.
- **Compra**: ao ser aceite, o livro muda de dono (vendedor → comprador) e fica **indisponível**.
- **Troca**: ao ser aceite, os dois livros trocam de dono e ficam **indisponíveis**.

### Concorrência (2+ pedidos para o mesmo livro)

- Se um livro já estiver **indisponível**, o sistema **bloqueia a aceitação** de novos pedidos que dependam dele.
- Ao aceitar uma **compra** ou **troca**, o sistema faz **cancelamento automático** de outros pedidos pendentes que envolvam o(s) mesmo(s) livro(s), para evitar inconsistências.

---

## Resolução de Problemas

| Problema | Solução |
|----------|---------|
| `gcc` não reconhecido | Instalar MinGW/MSYS2 e adicionar ao PATH |
| Caracteres estranhos | Executar no PowerShell: `[Console]::OutputEncoding = [System.Text.Encoding]::UTF8` |
| Dados não guardados | Verificar permissões de escrita na pasta |
| Erro ao abrir ficheiros | Executar o programa dentro da pasta `BookSwap/` |

---

## Autores

Desenvolvido por alunos do IPCA no âmbito da disciplina **Programação Imperativa e Procedimental (PIP)**.

---

## Licença

Este projeto foi desenvolvido para fins académicos.

---

<div align="center">

**IPCA** • Programação Imperativa e Procedimental • 2025/2026

</div>
