# Makefile

# Nome do executável
TARGET = ep1_limpo

# Lista de arquivos fonte
SOURCES = ep1_limpo.c spend_time.c

# Lista de arquivos de cabeçalho
HEADERS = spend_time.h

# Comando de compilação
CC = gcc

# Opções de compilação
CFLAGS = -Wall -Wextra

#Links
LINKS = -pthread -lm

# Comando de ligação
$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES) $(LINKS)