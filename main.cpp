/*
 * main.cpp - Chat multiusuario basado en memoria compartida
 *
 * Este programa de Jesús Torres <jmtorres@ull.es> está bajo una Licencia
 * Creative Commons Public Domain Dedication 1.0.
 *
 *     http://creativecommons.org/publicdomain/zero/1.0/deed.es
 */

#include <iostream>
#include <stdlib.h>
#include <cstring>

#include "chatroom.h"

//
// Función principal o punto de entrada del programa
//

int main()
{
    ChatRoom chatRoom;

    // Conectar a la sala de chat. Usar el nombre del usuario como
    // identificador de la sala
    const std::string username = getenv("USER");

    int result = chatRoom.connectTo(username);
    if ( result < 0 ) {
        std::cerr << "error: ";
        switch (result) {
        case -1:
            std::cerr << "instancia ya conectada a una sala";
            break;
        case -2:
            std::cerr << "shm_open(): "
                      << strerror(errno);
            break;
        case -3:
            std::cerr << "ftruncate(): "
                      << strerror(errno);
            break;
        case -4:
            std::cerr << "mmap(): "
                 << strerror(errno);
            break;
        default:
            std::cerr << "error desconocido";
        }
        std::cerr << std::endl;

        exit(3);
    }

    // Ejecutar el chat
    chatRoom.run();

    return 0;
}

