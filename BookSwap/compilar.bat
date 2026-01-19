@echo off
echo Compilando o projeto BookSwap...
echo.

gcc -Wall -Wextra -I./util -I./visual -I./visual/ecras/user -I./visual/ecras/menu -I./visual/ecras/livros util/*.c visual/main.c visual/ecras/user/*.c visual/ecras/menu/*.c visual/ecras/livros/*.c -o programa.exe

if %ERRORLEVEL% EQU 0 (
    echo.
    echo Compilacao concluida com sucesso!
    echo Executavel criado: programa.exe
) else (
    echo.
    echo Erro durante a compilacao!
)
