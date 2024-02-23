#
#                  Politecnico di Milano
#
#        Studente: Caravano Andrea
#
# Ultima modifica: 10/07/2023
#
#     Descrizione: Script di testing automatizzato
#                  per la Prova Finale (Progetto) di Algoritmi e Principi dell'Informatica 2023
#

#!/usr/bin/env bash

echo "Hai bisogno di indicazioni su come impostare le cartelle e i file di test? Rispondi con S per indicazioni, altrimenti batti semplicemente invio."
read -p "" tutorial
continua_tutorial=$(echo "$tutorial" | tr '[:upper:]' '[:lower:]')

# nome del file sorgente personale
NOME_SORGENTE="main.c"

# nome del binario compilato
NOME_COMPILATO="main"

# nome della cartella contenente i test open
CARTELLA_OPEN="archivio_test_aperti"

# numero del più piccolo test open (lo script chiede di selezionare quello da cui partire, in ogni caso)
PRIMO_OPEN=1

# numero del più grande test open
ULTIMO_OPEN=111

if [[ "${continua_tutorial:0:1}" == "s" ]]; then
    echo "Lo script si aspetta sia presente la cartella contenente i test open, con il nome di $CARTELLA_OPEN, nella stessa cartella in cui è contenuto lo script".
    echo "Lo script si aspetta sia presente il sorgente C11 della soluzione personale alla Prova Finale, nominato come "$NOME_SORGENTE
    echo "Lo script si occuperà di compilare il sorgente con le opzioni corrette, indicando eventuali errori di compilazione. Ci si senta liberi di modificare le opzioni di compilazione nel sorgente dello script."
    echo "Lo script si aspetta pieni diritti di scrittura, lettura ed esecuzione nella cartella in cui si trova. Non ne verranno generate di esteriori."
    echo "Lo script assume siano installati gcc, valgrind e le dipendenze necessarie alla compilazione ed esecuzione del sorgente. Seguire le indicazioni fornite dai docenti in tal senso."
fi

# chiude lo script in caso di errore
set -e
# set -ex per esplicitare le azioni svolte dallo script

# monta la cartella in cui è contenuto lo script
dir="$(cd -P -- "$(dirname -- "$0")" && pwd -P)"
cd "${dir}"

echo "Verifica esistenza cartella test open: "

if [ ! -d "$CARTELLA_OPEN" ]; then
    echo "Non trovata, verifica sia presente la cartella $CARTELLA_OPEN, contenente i test open pubblici."
    exit 1
fi

echo "Cartella trovata."

echo "Verifica presenza test open:"

for ((i=PRIMO_OPEN; i<=ULTIMO_OPEN; i++));
do
    if [ ! -f "$CARTELLA_OPEN/open_$i.txt" ]; then
        echo "Test open $i non trovato."
        exit 1
    fi

    if [ ! -f "$CARTELLA_OPEN/open_$i.output.txt" ]; then
        echo "Output test open $i non trovato."
        exit 1
    fi
done

echo "Verifica esistenza sorgente personale: "

if [ ! -f "$NOME_SORGENTE" ]; then
    echo "File sorgente non trovato."
    exit 1
fi

echo "File sorgente trovato correttamente. Si procede alla compilazione e al testing."

echo "Preparazione cartella per output personali."

if [ ! -d "output_personali" ]; then
    mkdir output_personali
fi

echo "Compilazione sorgente."

gcc -Wall -Werror -std=gnu11 -O2 -g3 -lm $NOME_SORGENTE -o $NOME_COMPILATO

echo "Compilazione completata. Avvio testing."

read -p "Inserisci il primo test da avviare (tra $PRIMO_OPEN e $ULTIMO_OPEN): " primo_testare
read -p "Inserisci l'ultimo test da avviare (tra $PRIMO_OPEN e $ULTIMO_OPEN): " ultimo_testare

echo "Desideri controllare anche gli errori di memoria con Valgrind? Confermane l'uso con S, altrimenti batti invio semplicemente."
read -p "" usa_valgrind
continua_valgrind=$(echo "$usa_valgrind" | tr '[:upper:]' '[:lower:]')

if [[ $primo_testare =~ ^[0-9]+$ && $ultimo_testare =~ ^[0-9]+$ && $primo_testare -ge 1 && $primo_testare -le 111 && $ultimo_testare -ge 1 && $ultimo_testare -le 111 && $ultimo_testare -ge $primo_testare ]]; then
    echo "Avvio testing..."

    for ((i=primo_testare; i<=ultimo_testare; i++));
    do
        ./"$NOME_COMPILATO" < $CARTELLA_OPEN/open_$i.txt > output_personali/personale_$i.txt
        echo "Esecuzione test n. $i. Vengono evidenziate le sole differenze! (output vuoto = ok)"
        sdiff -l $CARTELLA_OPEN/open_$i.output.txt output_personali/personale_$i.txt | cat -n | grep -v -e '($' || true
        if [[ "${continua_valgrind:0:1}" == "s" ]]; then
            valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./"$NOME_COMPILATO" < $CARTELLA_OPEN/open_$i.txt > /dev/null
        fi
    done
else
    echo "Non hai inserito identificativi validi. Terminazione"
    exit 1
fi
