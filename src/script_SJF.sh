#!/bin/bash

# Verifica se o Makefile existe
if [ -f "Makefile" ]; then
    # Executa o make
    make
    sleep 2
    # Limpa a tela
    clear
    gnome-terminal -- "./orchestrator" SJF
    start=$(date +%s%N)
    ./client execute 40 -u "ls -l"
    ./client execute 30 -u "ls"
    end=$(date +%s%N)
    runtime=$(( (end-start)/1000000 ))
    echo -e "\n----------\e[32mTeste SJF\e[0m----------\n"
    echo -e "\e[32m\nExecução da tarefa 1\e[0m : ls -l\n"
    cat "Resultados/output_1.txt"
    sleep 1
    echo -e "\e[32m\nExecução da tarefa 2\e[0m : ls\n"
    cat "Resultados/output_2.txt"
    echo -e "\e[32m\nTempo de execução\e[0m: $runtime milissegundos\n"
    echo -e "\n------------------------------\n"
    sleep 5
    # Executa make clean
    make clean
    sleep 2
    clear
else
    echo "Erro: Makefile não encontrado."
    exit 1
fi