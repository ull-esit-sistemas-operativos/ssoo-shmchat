/*
 * chatroom.cpp - Sala de chat multiusuario basada en memoria compartida
 *
 * Este programa de Jesús Torres <jmtorres@ull.es> está bajo una Licencia
 * Creative Commons Public Domain Dedication 1.0.
 *
 *     http://creativecommons.org/publicdomain/zero/1.0/deed.es
 */

/* @1@NOTA
 * El proyecto utiliza aspectos introducidos con C++11. Por ello hay que
 * añadir al archivo de proyecto la línea CONFIG += c++11 o compilar
 * desde la línea de comandos usando incluyendo la opción -std=c++11
 */

#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <iostream>

/* @1@NOTA
 * Al investigar como funcionan las llamadas al sistema hay que consultar las
 * páginas del manual (p. ej. man shm_open) y observar en la parte superior
 * las cabeceras que hacen falta para añadirlas aquí.
 *
 * En el caso de shm_open() incluso se dice que hay que enlazar con -lrt. Por
 * lo que es necesario añadir la línea LIBS += -lrt al archivo de proyecto o
 * compilar desde la línea de comandos incluyendo la opción -lrt
 */

#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "chatroom.h"

//
// Class ChatRoom::SharedMessage
//
// Estructura del buffer en memoria compartida usado para el intercambio
// de mensajes.
//

struct ChatRoom::SharedMessage
{
    char message[1048576];
    unsigned messageSize;
    unsigned messageCounter;

    char username[255];
    unsigned usernameSize;

    std::mutex mutex;
    std::condition_variable newMessage;

    SharedMessage();
};

ChatRoom::SharedMessage::SharedMessage()
    : messageSize(0),
      messageCounter(0),
      mutex(),
      newMessage()
{

}

//
// Class ChatRoom
//
// Clase de la sala de chat basada en memoria compartida.
//

ChatRoom::ChatRoom()
    : chatRoomId_(),
      sharedMessage_(nullptr),
      messageReceiveCounter_(0),      
      isSharedMemoryObjectOwner_(false),
      stopThreads(false)
{

}

ChatRoom::~ChatRoom()
{
    // Dejar de mapear la memoria del objeto de memoria compartida en el
    // espacio de direcciones del proceso
    if ( sharedMessage_ != nullptr )
        munmap(sharedMessage_, sizeof(SharedMessage));

    // Si somos los propietarios, destruir el objeto de memoria compartida
    if ( isSharedMemoryObjectOwner_ )
        shm_unlink(chatRoomId_.c_str());
}

//
// Conectar a la sala de chat indicada con el identificador
//
// Devuelve 0 en caso de éxito o un código de error negativo que indica la
// función que falló (-2 => shm_open(), -3 => ftruncate, etc.) En caso de
// fallo la variable global errno indica el motivo del error.
//
int ChatRoom::connectTo(const std::string& chatRoomId, const std::string& username)
{
    bool owner = false;
    int returnValue = -1;
    int error;

    // Volver si ya estamos conectados a una sala de chat
    if ( sharedMessage_ != nullptr ) {
        return returnValue;
    }

    username_ = username;

    --returnValue;

    // Fijar la máscara de modo de creación (UMASK) a 0 para que el objeto de
    // memoria compartida se pueda crear con permisos para todos los usuarios
    mode_t oldUmask = umask(0);

    // 'man shm_open()' indica que añadir una '/' al nombre es necesario
    // para ser portables
    std::string name = "/" + chatRoomId;

    // Intentar crear el objeto de memoria compartida
    int fd = shm_open(name.c_str(), O_RDWR | O_CREAT | O_EXCL, 0666);
    if ( fd >= 0 ) {
        // Creado. Recordar que somos el propietario
        owner = true;
    }
    else if ( fd < 0 && errno == EEXIST ) {
        // Error. El objeto de memoria compartida ya existe. Intentar abrirlo
        fd = shm_open(name.c_str(), O_RDWR, 0666);
    }

    /* @1@NOTA
     * El objeto de memoria compartida se puede ver en el sistema de archivos
     * en /dev/shm
     */

    /* @1@NOTA
     * Hay que notar que el responsable del objeto de memoria compartida
     * debe destruirlo con shm_unlink() al salir para que se pueda volver a
     * crear en la siguiente ejecución. Lamentablemente si el proceso recibe
     * una señal como SIGINT (Ctrl+C) o SIGHUP (al cerrar la ventana de la
     * terminal) el código que elimina el objeto no se ejecutará. Así que
     * tendremos que borrarlo a mano con 'rm /dev/shm/nombre' para que todo
     * vuelva a funcionar.
     */

    error = errno;

    // Comprobar si se ha podido crear o abrir el objeto de memoria
    // compartida. Los descriptores de archivo válidos siempre son >= 0
    if ( fd >= 0 ) {

        --returnValue;

        // Ajustar el tamaño del nuevo objeto para que quepa la estructura
        // SharedMessage. Inicialmente el tamaño es 0.
        if ( !owner || ftruncate(fd, sizeof(SharedMessage)) == 0) {

            --returnValue;

            // Maper el objeto de memoria compartida en el espacio de
            // direcciones del proceso
            void* addr = mmap(nullptr, sizeof(SharedMessage),
                              PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

            error = errno;

            // Comprobar si se ha podido mapear el objeto en la memoria
            if ( addr != MAP_FAILED ) {

                // Cerrar el descriptor de archivo del objeto. Con la memoria
                // mapeada ya no lo necesitamos.
                close(fd);

                chatRoomId_ = name;
                isSharedMemoryObjectOwner_ = owner;
                if ( owner )
                    // Si somos el propietario, inicializar una estructura
                    // SharedMessage en la memoria compartida usando el
                    // operador new con el emplazamiento indicado por addr.
                    sharedMessage_ = new(addr) SharedMessage;
                else
                    // Si no somos el propietario, ya hay una estructura
                    // SharedMessage inicializada por el propietario en la
                    // memoria compartida.
                    sharedMessage_ = (SharedMessage*)addr;

                return 0;
            }
        }

        error = errno;

        close(fd);
        if ( owner ) {
            shm_unlink(name.c_str());
        }
    }

    umask(oldUmask);

    errno = error;
    return returnValue;

}

void ChatRoom::run()
{
    // Volver si no estamos conectados a una sala de chat
    if ( sharedMessage_ == nullptr ) return;

    /* @2@NOTA
     * Como usamos métodos, hay que pasar como primer argumento el puntero
     * al objeto.
     */
    std::thread send_thread(&ChatRoom::runSender, this);
    std::thread receive_thread(&ChatRoom::runReceiver, this);

    // Espera a que el hilo de envío termine.
    send_thread.join();

    /* @2@NOTA
     * Terminar correctamente los hilos es importante. Si no hacemos nada
     * el hilo de recepción terminará al salir de esta función. Sin embargo
     * no lo hará de forma normal sino que el runtime llamará a
     * std::terminate(), que por defecto hará que llegue la señal SIGABORT
     * para abortar el proceso. Así el proceso terminará inmeditamente,
     * sin que el destructor de este objeto tenga oportunidad de destruir
     * el objeto de memoria compartida con shm_unlink();
     *
     * Esto no es bueno pero tampoco es grave, ya que ahora que todo el
     * mundo envía y recibe mensajes, no necesitamos el papel del propietario
     * como el único que envía mensajes.
     *
     * Además esto se puede resolver ejecutando receive_thread.detach()
     * antes de salir de esta función, ya que así no se ejecutará
     * std::terminate()
     *
     * Sin embargo, en ambos casos, corremos el riesgo de que el hilo sea
     * interrumpido con el mutex bloqueado. Entonces ningún otro hilo
     * de ningún otro proceso podrá volver a usar las sala hasta que sea
     * destruida y vuelta a crear. Por eso es mejor indicar al hilo de
     * recepción que termine por sus propios medios.
     */
    stopThreads = true;
    sharedMessage_->newMessage.notify_all();
    receive_thread.join();
}

void ChatRoom::runSender()
{
    // El fin de archivo (EOF) en la entrada estándar se indica
    // pulsando Ctrl+D
    while ( ! std::cin.eof() ) {
        std::string message;
        std::getline(std::cin, message);

        if ( message == ":quit" ) break;
        send(message);
    }
}

void ChatRoom::send(const std::string& message)
{
    /* @1@NOTA
     * Hay que explicar la estructura básica del acceso y salida a una sección
     * crítica (lock() .... unlock()) y la función de las variables de
     * condición: wait() y notify_all().
     *
     * Aunque el mutex tiene los métodos lock() y unlock(), el método wait()
     * de la variable de condición espera un objecto std::unique_lock().
     * Además es recomendable usar este objeto porque se encarga de bloquear
     * el mutex durante su construcción, asegurándose de liberarlo durante
     * su destrucción. En este caso, al salir de la función.
     */

    // Bloquear el mutex hasta salir de la función
    std::unique_lock<std::mutex> lock(sharedMessage_->mutex);

    message.copy(sharedMessage_->message, message.length());
    sharedMessage_->messageSize = message.length();
    sharedMessage_->messageCounter++;

    username_.copy(sharedMessage_->username, username_.length());
    sharedMessage_->usernameSize = username_.length();

    sharedMessage_->newMessage.notify_all();
}

void ChatRoom::runReceiver()
{
    while ( ! stopThreads ) {
        std::string message;
        std::string username;

        receive(message, username);
        if ( ! stopThreads && username != username_ ) {
            std::cout << "++ " << username << ": " << message << std::endl;
        }
    }
}

void ChatRoom::receive(std::string& message, std::string& username)
{
    // Bloquear el mutex hasta salir de la función
    std::unique_lock<std::mutex> lock(sharedMessage_->mutex);

    while ( ! stopThreads &&
            messageReceiveCounter_ >= sharedMessage_->messageCounter)
    {
        sharedMessage_->newMessage.wait(lock);
    }

    if ( ! stopThreads ) {
        messageReceiveCounter_ = sharedMessage_->messageCounter;
        message.assign(sharedMessage_->message, sharedMessage_->messageSize);
        username.assign(sharedMessage_->username, sharedMessage_->usernameSize);
    }
}
