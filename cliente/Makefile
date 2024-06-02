# Nombre del archivo de salida
TARGET = cliente

# Compilador a usar
CC = g++

# Flags del compilador
CFLAGS = -Wall -std=c++11

# Archivos fuente
SRCS = cliente_2_0.cpp

# Regla por defecto
all: $(TARGET)

# Regla para compilar el programa
$(TARGET): $(SRCS)
    $(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

# Regla para limpiar los archivos generados
clean:
    rm -f $(TARGET)