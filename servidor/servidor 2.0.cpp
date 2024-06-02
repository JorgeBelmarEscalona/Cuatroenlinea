#include <iostream>              // Para operaciones de entrada/salida
#include <thread>                // Para trabajar con hilos
#include <vector>                // Para trabajar con vectores
#include <sys/socket.h>          // Para trabajar con sockets
#include <netinet/in.h>          // Para trabajar con direcciones de Internet
#include <unistd.h>              // Para trabajar con llamadas al sistema Unix
#include <netdb.h>               // Para trabajar con la base de datos de red
#include <arpa/inet.h>           // Para trabajar con operaciones de Internet
#include <random>                // Para trabajar con números aleatorios
#include <chrono>                // Para trabajar con tiempo
#include <mutex>                 // Para trabajar con exclusión mutua
#include <condition_variable>    // Para trabajar con variables de condición

class Server {
private:
    // Un generador de números aleatorios
    std::default_random_engine generator;

    // El descriptor de archivo del socket del servidor
    int server_fd;

    // La dirección del socket del servidor
    struct sockaddr_in address;

    // Una opción para la configuración del socket
    int opt = 1;

    // El tamaño de la estructura de dirección
    int addrlen = sizeof(address);

    // Los descriptores de archivo de los sockets de los clientes
    int client_sockets[2];

    // Un vector de hilos para manejar a los clientes
    std::vector<std::thread> threads;

    // El tablero de juego
    std::vector<std::vector<char>> board;

    // Un booleano para indicar de quién es el turno
    bool turn;

    // Un mutex para proteger el acceso al tablero
    std::mutex board_mutex;

    // Una variable de condición para esperar y notificar cambios de turno
    std::condition_variable turn_cv;

public:
    // Constructor de la clase Server
    Server(int port) : board(6, std::vector<char>(7, ' ')), turn(false) {
        // Inicializa el generador de números aleatorios
        generator.seed(std::chrono::system_clock::now().time_since_epoch().count());

        // Obtiene el nombre del host
        char hostname[1024];
        hostname[1023] = '\0';
        gethostname(hostname, 1023);

        // Obtiene la dirección IP del host
        struct hostent* h;
        h = gethostbyname(hostname);
        char* ip = inet_ntoa(*((struct in_addr*)h->h_addr_list[0]));

        // Imprime la dirección IP y el puerto en el que se está ejecutando el servidor
        std::cout << "El servidor se está ejecutando en la dirección IP " << ip << " y el puerto " << port << std::endl;

        // Crea un socket para el servidor
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
            perror("socket failed");
            exit(EXIT_FAILURE);
        }

        // Configura el socket para permitir la reutilización de la dirección y el puerto
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
            perror("setsockopt");
            exit(EXIT_FAILURE);
        }

        // Configura la dirección y el puerto para el socket
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        // Vincula el socket a la dirección y el puerto
        if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
            perror("bind failed");
            exit(EXIT_FAILURE);
        }

        // Configura el socket para escuchar conexiones
        if (listen(server_fd, 2) < 0) {
            perror("listen");
            exit(EXIT_FAILURE);
        }
    }

    // Método para aceptar conexiones de los clientes
    void acceptConnections() {
        // Acepta conexiones de dos clientes
        for (int i = 0; i < 2; ++i) {
            // Acepta una conexión de un cliente
            if ((client_sockets[i] = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
                // Si hay un error, imprime el mensaje de error y termina el programa
                perror("accept");
                exit(EXIT_FAILURE);
            }

            // Imprime un mensaje indicando que el jugador se ha conectado
            std::cout << "Jugador " << i + 1 << " conectado." << std::endl;

            // Envía un mensaje al cliente indicando su número de jugador
            std::string message = "Eres el jugador " + std::string(i == 0 ? "C" : "S") + ".\n";
            send(client_sockets[i], message.c_str(), message.size(), 0);
        }

        // Establece que el jugador 1 (Cliente) comienza
        turn = true;

        // Crea un hilo para manejar cada cliente
        for (int i = 0; i < 2; ++i) {
            threads.emplace_back(&Server::handleClient, this, client_sockets[i], i + 1);
        }

        // Espera a que todos los hilos se hayan completado
        for (auto& thread : threads) {
            thread.join();
        }
    }

    // Método para verificar si un jugador ha ganado
    bool checkWin(char player) {
        // Verifica si hay cuatro fichas del jugador en una fila horizontal
        for (int i = 0; i < 6; ++i) {
            for (int j = 0; j < 4; ++j) {
                if (board[i][j] == player && board[i][j + 1] == player && board[i][j + 2] == player && board[i][j + 3] == player) {
                    return true;
                }
            }
        }

        // Verifica si hay cuatro fichas del jugador en una fila vertical
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 7; ++j) {
                if (board[i][j] == player && board[i + 1][j] == player && board[i + 2][j] == player && board[i + 3][j] == player) {
                    return true;
                }
            }
        }

        // Verifica si hay cuatro fichas del jugador en una fila diagonal de izquierda a derecha
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 4; ++j) {
                if (board[i][j] == player && board[i + 1][j + 1] == player && board[i + 2][j + 2] == player && board[i + 3][j + 3] == player) {
                    return true;
                }
            }
        }

        // Verifica si hay cuatro fichas del jugador en una fila diagonal de derecha a izquierda
        for (int i = 0; i < 3; ++i) {
            for (int j = 3; j < 7; ++j) {
                if (board[i][j] == player && board[i + 1][j - 1] == player && board[i + 2][j - 2] == player && board[i + 3][j - 3] == player) {
                    return true;
                }
            }
        }

        // Si no se encontró ninguna fila ganadora, devuelve false
        return false;
    }

    // Método para convertir el tablero a una cadena
    std::string boardToString() {
        // Crea una cadena vacía para almacenar el tablero
        std::string boardStr;

        // Añade la primera línea que contiene los números de las columnas
        boardStr += "  0 1 2 3 4 5 6\n";

        // Añade la segunda línea que contiene una serie de guiones
        boardStr += "  ---------------\n";

        // Recorre cada fila del tablero
        for (int i = 0; i < 6; ++i) {
            // Añade un carácter de barra vertical y un espacio al inicio de cada fila
            boardStr += "| ";

            // Recorre cada columna de la fila actual
            for (int j = 0; j < 7; ++j) {
                // Añade el carácter en la posición actual del tablero a la cadena
                boardStr += board[i][j];

                // Añade un espacio después de cada carácter
                boardStr += ' ';
            }

            // Añade un carácter de barra vertical y un salto de línea al final de cada fila
            boardStr += "|\n";
        }

        // Añade una última línea que contiene una serie de guiones
        boardStr += "  ---------------\n";

        // Devuelve la cadena que representa el tablero
        return boardStr;
    }

    // Método para manejar la comunicación con un cliente específico
    void handleClient(int socket, int player) {
        // Buffer para almacenar los datos recibidos del cliente
        char buffer[1024] = {0};

        // Entra en un bucle infinito
        while (true) {
            // Bloquea el mutex del tablero para evitar condiciones de carrera
            std::unique_lock<std::mutex> lock(board_mutex);

            // Espera hasta que sea el turno del jugador
            turn_cv.wait(lock, [this, player] { return turn == (player == 1); });

            // Convierte el tablero a una cadena y añade información sobre el turno del jugador
            std::string boardStr = boardToString() + "\n" + "Turno del jugador: " + std::string(turn ? "C" : "S") + "\n";

            // Envía el tablero y la información del turno al cliente
            send(socket, boardStr.c_str(), boardStr.size(), 0);

            // Pide al cliente que introduzca la columna
            send(socket, "Tu turno. Introduce la columna: ", 32, 0);

            // Lee la respuesta del cliente
            int valread = read(socket, buffer, 1024);

            // Convierte la respuesta a un entero
            int column = atoi(buffer);

            // Comprueba si el movimiento es válido
            if (column < 0 || column >= 7 || board[0][column] != ' ') {
                // Si no es válido, envía un mensaje al cliente y vuelve al inicio del bucle
                std::string message = "Movimiento inválido. Inténtalo de nuevo.";
                send(socket, message.c_str(), message.size(), 0);
            } else {
                // Si es válido, actualiza el tablero
                for (int i = 5; i >= 0; --i) {
                    if (board[i][column] == ' ') {
                        board[i][column] = player == 1 ? 'C' : 'S';
                        break;
                    }
                }

                // Convierte el tablero a una cadena
                std::string boardStr = boardToString();

                // Envía el tablero actualizado a ambos clientes
                send(client_sockets[0], boardStr.c_str(), boardStr.size(), 0);
                send(client_sockets[1], boardStr.c_str(), boardStr.size(), 0);

                // Comprueba si el jugador ha ganado
                if (checkWin(player == 1 ? 'C' : 'S')) {
                    // Si ha ganado, envía un mensaje a ambos clientes y termina el juego
                    std::string message = "El jugador " + std::string(player == 1 ? "C" : "S") + " ha ganado!";
                    send(client_sockets[0], message.c_str(), message.size(), 0);
                    send(client_sockets[1], message.c_str(), message.size(), 0);
                    std::cout << "El jugador " << (player == 1 ? "C" : "S") << " ha ganado!" << std::endl;

                    // Espera 5 segundos antes de cerrar las conexiones
                    std::this_thread::sleep_for(std::chrono::seconds(5));
                    close(client_sockets[0]);
                    close(client_sockets[1]);

                    // Termina el método
                    return;
                }

                // Cambia el turno al otro jugador
                turn = !turn;

                // Notifica a todos los hilos que están esperando en la variable de condición
                turn_cv.notify_all();
            }
        }
    }

    // Destructor de la clase Server
    ~Server() {
        // Cierra el descriptor de archivo del servidor
        close(server_fd);

        // Recorre los sockets de los clientes
        for (int i = 0; i < 2; ++i) {
            // Cierra el socket del cliente actual
            close(client_sockets[i]);
        }
    }
};

// La función principal que se ejecuta cuando se inicia el programa
int main(int argc, char const *argv[]) {
    // Comprueba si el número de argumentos proporcionados es diferente de 2
    if (argc != 2) {
        // Si es así, imprime un mensaje de error y la forma correcta de usar el programa
        std::cerr << "Uso: " << argv[0] << " <puerto>\n";
        // Termina el programa con un código de error
        return 1;
    }

    // Convierte el segundo argumento (el puerto) a un entero
    int port = atoi(argv[1]);
    // Crea un nuevo objeto Server que escucha en el puerto especificado
    Server server(port);
    // Hace que el servidor acepte conexiones de clientes
    server.acceptConnections();
    // Termina el programa con éxito
    return 0;
}