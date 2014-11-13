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
#include <vector>
#include <cstring>

#include "chatroom.h"

//
// Funciones para mostrar mensajes de ayuda y de error
//

const char* programShortName(const char* appName = NULL)
{
   static const char* name = appName;
   return name;
}

void showUsage (std::ostream& out = std::cout)
{
    out << "Modo de empleo: "
        << programShortName()
        << " [-h | --help] [[-u | --user] CHATROOMID]"
        << std::endl;
}

void showOptionError (const std::string& message, const std::string& option = std::string())
{
    std::cerr << programShortName() << ": " << message;
    if ( ! option.empty() ) {
        std::cerr << " -- " << option << std::endl;
    }
    showUsage(std::cerr);
}

//
// Función principal o punto de entrada del programa
//

int main(int argc, char** argv)
{
    ChatRoom chatRoom;
    std::string chatRoomId;
    std::string username = getenv("USER");

    std::vector<std::string> arguments(argv, argv +  argc);
    programShortName(basename(argv[0]));

    if (arguments.size() < 2) {
        showUsage();
        exit(2);
    }

    // Análisis de las opciones de línea de comandos
    for (unsigned i = 1; i < arguments.size(); ++i) {
        const std::string& option = arguments[i];

        if (option == "-h" || option == "--help") {
            showUsage();
            exit(0);
        }
        /* @2@NOTA
         * Incluir la posibilidad de indicar un nombre de usuario se podría
         * considerar como opcional.
         */
        else if (option == "-u" || option == "--user") {
            if ( ++i >= arguments.size()) {
                showOptionError("la opción requiere un argumento", option);
                exit(2);
            }
            username = arguments[i];
        }
        else if (option[0] != '-') {
            chatRoomId = option;
        }
        else {
            showOptionError("opción inválida", option);
            exit(2);
        }
    }

    if ( chatRoomId.empty() ) {
        showOptionError("es necesario indicar un nombre para la sala de chat");
        exit(2);
    }

    // Conectar a la sala de chat usando el identificador y nombre de usuario
    // indicado
    int result = chatRoom.connectTo(chatRoomId, username);
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

