/*
 * chatroom.cpp - Sala de chat multiusuario basada en memoria compartida
 *
 * Este programa de Jesús Torres <jmtorres@ull.es> está bajo una Licencia
 * Creative Commons Public Domain Dedication 1.0.
 *
 *     http://creativecommons.org/publicdomain/zero/1.0/deed.es
 */

#include <condition_variable>
#include <mutex>
#include <string>

#include "chatroom.h"

//
// Class ChatRoom::SharedMessage
//
// Estructura del buffer en memoria compartida usado para el intercambio
// de mensajes.
//

struct ChatRoom::SharedMessage
{
    /* TODO: Añadir:
     *  1. Un array char de al menos 1MB para almacenar el mensaje de texto.
     *  2. Una variable para almacenar el tamaño exacto de dicho mensaje
     *     almacenado en cada momento (que sin duda será menor del tamaño
     *     máximo de 1MB)
     *  3. Un contador del número de mensaje. Cada vez que se copie un nuevo
     *     mensaje este contador se incrementará para indicar que el mensaje
     *     es nuevo.
     */

    /* TODO: Añadir los objetos de sincronización que hagan falta. Por ejemplo
     * mutex, variables de condición, etc.
     */

    SharedMessage();
};

/* TODO: Usar el constructor para inicializar los miembros de la clase
 * ChatRoom::SharedMessage antes de que se usen por primera vez
 */
ChatRoom::SharedMessage::SharedMessage()
{

}

//
// Class ChatRoom
//
// Clase de la sala de chat basada en memoria compartida.
//

ChatRoom::ChatRoom()
    : sharedMessage_(nullptr),
      messageReceiveCounter_(0),      
      isSharedMemoryObjectOwner_(false)
{

}

ChatRoom::~ChatRoom()
{
    /* TODO: Si sharedMessage_ contiene un puntero a la memoria compartida
     * deshacer el mapeo con munmap().
     */

    /* TODO: Si somos quien creamos el objeto de memoria compartida
     * (se puede usar isSharedMemoryObjectOwner_ para indicarlo) destruir el
     * objeto de memoria compartida con shm_unlink().
     */
}

//
// Conectar a la sala de chat indicada con el identificador
//
// Devuelve 0 en caso de éxito o un código de error negativo que indica la
// función que falló (-2 => shm_open(), -3 => ftruncate, etc.) En caso de
// fallo la variable global errno indica el motivo del error.
//
int ChatRoom::connectTo(const std::string& chatRoomId)
{
    /* TODO: La conexión consiste en obtener una zona de memoria compartida
     * con el nombre indicado en chatRoomId y guarda el puntero en
     * sharedMessage_ para usarlo posteriormente en las funciones send() y
     * receive().
     *
     * 1. Crear el objeto de memoria compartida usando shm_open() con el nombre
     * de chatRoomId.
     *
     *      (a) Si el objeto no existía, consideramos que somos el propietario
     *      por lo que tenemos que inicializarlo y destruirlo cuando no haga
     *      falta (en el destructor de este objeto). Para recordar que somos
     *      el propietario podemos usar la variable isSharedMemoryObjectOwner_.
     *
     *      (b) Si el objeto existía, sólo somos otro cliente que se conecta
     *      a un objeto ya inicializado por su propietario. Por lo tanto no
     *      tenemos que inicializarlo ni destruirlo.
     *
     *      (c) Para saber si el objeto ya existe o no, puede ser útil emplear
     *      la opción O_EXCL de shm_open().
     *
     * 2. El tamaño inicial del objeto de memoria es 0 pero debe ser
     * sizeof(ChatRoom::SharedMessage). SI SOMOS EL PROPIETARIO, tenemos que
     * indicar su tamaño usando ftruncate() sobre el descriptor devuelto por
     * shm_open().
     *
     * 3. Mapear el objeto de memoria compartida en la memoria del proceso con
     * mmap().
     *
     *      (a) Necesitamos permisos de lectura (PROT_READ) y escritura
     *      (PROT_WRITE).
     *      (b) Necesitamos que el mapeo sea compartido (MAP_SHARED)
     *
     *  La función mmap() devuelve un puntero a dicha memoria compartida que
     *  debemos guardar en sharedMessage_;
     *
     * 4. Terminar...
     *
     *      (a) Si somos el propietario, la memoria compartida no se ha
     *      inicializado, por lo que está vacía. Debemos inicializar un
     *      objeto ChatRoom::SharedMessage en la memoria apuntada por
     *      sharedMessage_ usando el operador new de emplazamiento.
     *      Por ejemplo:
     *
     *          new(sharedMessage_) ChatRoom::SharedMessage;
     *
     *      Recuerda que no se puede usar el operador new convencional
     *
     *          new ChatRoom::SharedMessage;
     *
     *      porque no queremos que el sistema nos asigne una nueva zona de
     *      memoria, sino que use la memoria compartida que hemos mapeado.
     *
     *      (b) Si no somos el propietario, con tener el puntero en
     *      sharedMessage_ es suficiente.
     *
     * Recuerda que cada llamada al sistema puede fallar y deberías estar
     * atento y comprobar si pasa antes de continuar:
     *
     *      (a) Los detalles de cómo lo indica están en el manual (man).
     *      (b) Por lo general indican que hay un error devolviendo un valor
     *      negativo. Mientras que el motivo exacto se almacena en la variable
     *      global errno.
     *      (c) Como se dice arriba, ChatRoom::connectTo() debería devolver un
     *      valor negativo distinto en cada lugar posible de fallo, si falla
     *      (-2 => shm_open(), -3 => ftruncate, etc.) asegurando que se
     *      conserva el valor de errno para que quien la llama pueda comprobar
     *      el motivo del error.
     *
     * Si tiene éxito, ChatRoom::connectTo() debe devolver 0.
     */
}

void ChatRoom::run()
{
    // Volver si no estamos conectados a una sala de chat
    if ( sharedMessage_ == nullptr ) return;

    // El propietario es el único que envía
    if ( isSharedMemoryObjectOwner_ ) {
        runSender();
    }
    else {
        runReceiver();
    }
}

void ChatRoom::runSender()
{
    // run() sólo llamará a esta función si somos el propietario del objeto
    // de memoria compartida para enviar los mensajes del usuario

    /* TODO:
     * Bucle infinito que lee de la entrada estándar (std::cin) una línea
     * (ver std:getline()) y la manda usando la función send() de esta clase.
     *
     * Si el usuario escribe :quit, se debe salir salir del bucle para que
     * termine el programa.
     */
}

void ChatRoom::send(const std::string& message)
{
    /* TODO:
     * Escribir el código que envía un mensaje a los otros clientes.
     * Es decir...
     */

    /* TODO: Bloquear el objeto de sincronización que uses en
     * ChatRoom::SharedMessage. Mira std::mutex y std::unique_lock
     */

    /* TODO: Copiar el mensaje y su tamaño a ChatRoom::SharedMessage
     */

    /* TODO: Incrementar el contado de número de mensaje de
     * ChatRoom::SharedMessage
     */

    /* TODO: Notificar a los clientes que duermen a la espera de nuevos
     * mensajes que ya hay un mensaje nuevo. Mirar std::condition_variable
     */

    /* TODO: Desbloquear el objeto de sincronización
     */
}

void ChatRoom::runReceiver()
{
    // run() sólo llamará a esta función si NO somos el propietario del objeto
    // de memoria compartida para mostrar los mensajes recibidos

    /* TODO:
     * Bucle infinito que recibe un mensaje con la función receive() de esta
     * clase y lo muestra por la salida estándar.
     */
}

void ChatRoom::receive(std::string& message)
{
    /* TODO:
     * Escribir el código que recibe un mensaje enviado.
     * Es decir...
     */

    /* TODO: Bloquear el objeto de sincronización que uses en
     * ChatRoom::SharedMessage. Mira std::mutex y std::unique_lock
     */

    /* TODO: Comprobar si el contador de número de mensaje de
     * ChatRoom::SharedMessage ha cambiado desde la última vez. Si no ha
     * cambiado, esperar en una condición de variable a que el que envía
     * notifique que ha insertado un nuevo mensaje.
     *
     * Mirar std::condition_variable.
     *
     * Debido a que los hilos pueden despertarse de las condiciones de variable
     * de forma espúrea (sin que haya motivo para ello) la espera debe hacerse
     * en un bucle de la siguiente manera:
     *
     * while ( condicion ) {
     *      variable_de_condición.wait(...)
     * }
     */

    /* TODO: Leer el mensaje de ChatRoom::SharedMessage y copiarlo a message
     * teniendo en cuenta que conocemos su tamaño.
     */

    /* TODO: Desbloquear el objeto de sincronización
     */
}

void ChatRoom::execAndSend(const std::string& command)
{
    /* TODO: Por ahora nada. Nos olvidamos de esta función */
}
