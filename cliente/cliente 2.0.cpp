#include <iostream>  // Para operaciones de entrada/salida estándar
#include <string>  // Para trabajar con cadenas de texto
#include <cstring>  // Para trabajar con cadenas de caracteres y funciones relacionadas
#include <sys/socket.h>  // Para operaciones de socket
#include <arpa/inet.h>  // Para operaciones de internet
#include <unistd.h>  // Para operaciones del sistema operativo, como close()

// Definición de la clase Cliente
class Cliente {
private:
    // Variables miembro privadas
    int sock;  // Socket para la comunicación
    std::string direccionServidor;  // Dirección IP del servidor
    int puertoServidor;  // Puerto del servidor
    struct sockaddr_in servidor;  // Estructura para la dirección del servidor

public:
    // Constructor que inicializa la dirección y el puerto del servidor
    Cliente(std::string direccion, int puerto) : direccionServidor(direccion), puertoServidor(puerto) {
        // Crea un socket
        sock = socket(AF_INET, SOCK_STREAM, 0);
        // Si la creación del socket falla, imprime un error y termina el programa
        if (sock == -1) {
            std::cerr << "No se pudo crear el socket" << std::endl;
            exit(EXIT_FAILURE);
        }
        // Configura la dirección del servidor
        servidor.sin_addr.s_addr = inet_addr(direccionServidor.c_str());
        servidor.sin_family = AF_INET;
        servidor.sin_port = htons(puertoServidor);
    }

    // Método para conectar al servidor
    bool conectar() {
        // Intenta conectar al servidor
        if (connect(sock, (struct sockaddr *)&servidor, sizeof(servidor)) < 0) {
            // Si la conexión falla, imprime un error y devuelve false
            perror("Conexión fallida. Error");
            return false;
        }
        // Si la conexión es exitosa, imprime un mensaje y devuelve true
        std::cout << "Conectado" << std::endl;
        return true;
    }

    // Método para enviar datos al servidor
    bool enviarDatos(const std::string& datos) {
        // Intenta enviar los datos al servidor
        if (send(sock, datos.c_str(), datos.size(), 0) < 0) {
            // Si el envío falla, imprime un error y devuelve false
            perror("Envío fallido. Error");
            return false;
        }
        // Si el envío es exitoso, devuelve true
        return true;
    }

    // Método para recibir datos del servidor
    std::string recibirDatos() {
        // Buffer para los datos recibidos
        char buffer[4096];
        // Inicializa el buffer a 0
        memset(buffer, 0, sizeof(buffer));
        // Intenta recibir datos del servidor
        if (recv(sock, buffer, sizeof(buffer), 0) < 0) {
            // Si la recepción falla, imprime un error y devuelve una cadena vacía
            perror("Recepción fallida. Error");
            return "";
        }
        // Si la recepción es exitosa, devuelve los datos recibidos como una cadena
        return std::string(buffer);
    }

    // Método para cerrar la conexión al servidor
    void cerrar() {
        // Cierra el socket
        close(sock);
    }
};

// Inicio del programa
int main(int argc, char *argv[]) {
    // Comprueba si el número de argumentos es correcto
    if (argc != 3) {
        // Imprime un mensaje de error si el número de argumentos no es correcto
        std::cerr << "Uso: " << argv[0] << " <IP del servidor> <puerto del servidor>" << std::endl;
        // Termina el programa con un código de error
        return 1;
    }

    // Convierte el segundo argumento a una cadena y el tercero a un entero
    std::string direccionServidor = argv[1];
    int puertoServidor = std::stoi(argv[2]);
    // Crea un objeto Cliente con la dirección y el puerto del servidor
    Cliente cliente(direccionServidor, puertoServidor);

    // Intenta conectar el cliente al servidor
    if (!cliente.conectar()) {
        // Si la conexión falla, termina el programa con un código de error
        return 1;
    }

    // Entra en un bucle infinito
    while (true) {
        // Recibe un mensaje del servidor
        std::string mensajeServidor = cliente.recibirDatos();
        // Si la cadena está vacía, asume que el servidor se ha cerrado
        if (mensajeServidor.empty()) {
            std::cout << "El servidor se ha cerrado. Terminando el cliente." << std::endl;
            break;
        }
        // Imprime el mensaje
        std::cout << mensajeServidor << std::endl;

        // Si el mensaje indica que es el turno del cliente
        if (mensajeServidor.find("Tu turno") != std::string::npos) {
            // Solicita al usuario que introduzca un movimiento
            std::string movimiento;
            std::cout << "Introduce tu movimiento (o 'salir' para terminar): ";
            std::getline(std::cin, movimiento);
            // Si el usuario introduce "salir", rompe el bucle
            if (movimiento == "salir") {
                break;
            }
            // Envía el movimiento al servidor
            cliente.enviarDatos(movimiento);
        }
    }

    // Cierra la conexión al servidor
    cliente.cerrar();
    // Termina el programa con éxito
    return 0;
}