cmake_minimum_required(VERSION 3.15)
project(Pokladna_C64 C)

add_custom_command(
        OUTPUT Pokladna_C64.prg
        COMMAND cl65 --target c64 -o Pokladna_C64.prg ${CMAKE_SOURCE_DIR}/main.c
        DEPENDS ${CMAKE_SOURCE_DIR}/main.c
        COMMENT "Building Pokladna_C64.prg with cl65"
        COMMAND rm -rf Pokladna.d64 && c1541 -format "pokladna,01" d64 Pokladna.d64 -write Pokladna_C64.prg pokladna
)

add_custom_target(Pokladna_C64 ALL
        DEPENDS Pokladna_C64.prg
)
