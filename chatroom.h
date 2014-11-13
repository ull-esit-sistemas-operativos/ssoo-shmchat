/*
 * chatroom.h - Sala de chat multiusuario basada en memoria compartida
 *
 * Este programa de Jesús Torres <jmtorres@ull.es> está bajo una Licencia
 * Creative Commons Public Domain Dedication 1.0.
 *
 *     http://creativecommons.org/publicdomain/zero/1.0/deed.es
 */

#ifndef CHATROOM_H
#define CHATROOM_H

#include <string>

class ChatRoom
{
public:

    ChatRoom();
    ~ChatRoom();

    // Conectar a la sala de chat indicada
    int connectTo(const std::string& chatRoomId);
    // Ejecutar el chat
    void run();

private:
    struct SharedMessage;

    // Identificador de la sala y nombre del objeto de memoria compartida
    std::string chatRoomId_;
    // Buffer en memoria compartida para el intercambio de mensajes
    SharedMessage* sharedMessage_;
    // Número de secuencia del último mensaje leido con receive()
    unsigned messageReceiveCounter_;

    // Indicador de si el objeto es el propietario del objeto de memoria
    // compartida. El propietario es el responsable de su destrucción
    bool isSharedMemoryObjectOwner_;

    // Enviar un mensaje a la sala de chat
    void send(const std::string& message);
    // Recibir un mensaje de la sala de chat
    void receive(std::string& message);
};

#endif // CHATROOM_H
