#!/bin/bash

# Verifica se o Makefile existe
if [ -f "Makefile" ]; then
    # Executa o make
    make
    sleep 2
    # Limpa a tela
    clear
    gnome-terminal -- "./orchestrator" FCFS
    ./client execute 200 -u "ls -l"
    echo "Execução da tarefa 1 : ls -l"
    sleep 2
    cat "Resultados/output_1.txt"
    sleep 5
    # Executa make clean
    make clean
    sleep 2
    clear
else
    echo "Erro: Makefile não encontrado."
    exit 1
fi

