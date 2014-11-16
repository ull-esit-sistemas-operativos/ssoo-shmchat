/*
 * main.cpp - Chat multiusuario basado en memoria compartida
 *
 * Este programa de Jesús Torres <jmtorres@ull.es> está bajo una Licencia
 * Creative Commons Public Domain Dedication 1.0.
 *
 *     http://creativecommons.org/publicdomain/zero/1.0/deed.es
 */

#include "chatroom.h"

//
// Función principal o punto de entrada del programa
//

int main()
{
    std::string chatRoomId /* = Nombre de la sala de chat. TODO: Usa el nombre
                                de la cuenta de usuario. Puedes obtenerlo con
                                la función getenv() */;
    ChatRoom chatRoom;

    // Conectar a la sala de chat usando como identificador el nombre de la
    // cuenta del usuario
    int result = chatRoom.connectTo(chatRoomId);

    /* TODO: Mostrar un mensaje de error en la salida de error (std:cerr) en
     * caso de fallo en connecTo().
     *
     * connectTo() puede fallar por diversos motivos. Si se ha implementado
     * bien, debe devolver un 0 en caso de éxito o un número negativo en caso
     * de error. Este número debe indicar el lugar del fallo, mientras que la
     * variable global errno almacena el motivo del error.
     *
     * Con la función strerror(errno) se puede obtener una cadena estándar de
     * texto que describe el motivo del fallo (la proporciona el sistema
     * operativo)
     *
     * Toda esta información debe proporcionárse al usuario de forma
     * conveniente. Por ejemplo:
     *
     *  nombre del programa: lugar del error: motivo del error segun errno.
     */

    // Ejecutar el chat
    chatRoom.run();

    return 0;
}

