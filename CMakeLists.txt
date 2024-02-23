#
#                  Politecnico di Milano
#
#        Studente: Caravano Andrea
#
# Ultima modifica: 25/06/2023
#
#     Descrizione: Opzioni compilazione per CLion/CMake con GCC/LLVM/LLDB e Address/Undefined Sanitizers + livelli di warning
#
# Sostituire "NomeProgetto" non è fondamentale, funziona ugualmente, ma nel caso può essere qualsiasi
# puoi cancellare vecchio compilato (eventuale caching) con Build -> Clean
# un codice di uscita > 0 vuol dire che c'è stato un qualche errore (magari per via dei Sanitizer)
# per controllare i Sanitizer, guarda in Run/Debug e apri "Sanitizers"
# riferimenti addizionali: https://felsoci.sk/blog/using-address-sanitizer-asan-in-a-cmake-project.html

cmake_minimum_required(VERSION 3.0)
project(NomeProgetto C)

set(CMAKE_C_STANDARD 11)
# output di compilazione CMake salvato in maniera completa
set(CMAKE_VERBOSE_MAKEFILE true)

add_executable(NomeProgetto main.c)

# Opzioni compilazione per target di Debug (di default impostato da CLion)
if (CMAKE_BUILD_TYPE MATCHES "Debug")
    set(
            CMAKE_C_FLAGS
            # opzioni compilatore (con ottimizzazione)
            # "${CMAKE_C_FLAGS} -Wall -Werror -std=gnu11 -O2 -g3 -fsanitize=undefined -fsanitize=address"
            # senza ottimizzazione (consigliato in fase di sviluppo)
            "${CMAKE_C_FLAGS} -Wall -Werror -std=gnu11 -g3 -fsanitize=undefined -fsanitize=address"
    )
    target_link_options(NomeProgetto # usare nome del Progetto
            # opzioni linker
            BEFORE PUBLIC -lm PUBLIC -fsanitize=undefined PUBLIC -fsanitize=address
            )
endif ()