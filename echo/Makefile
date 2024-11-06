.PHONY: all debug release clean

Compiler = g++
RELEASE_FLAGS = -O2 -Wall -DNDEBUG 
DEBUG_FLAGS   = -g -O0 -Wall 

CLIENT_EXEC  = echo_client
SERVER_EXEC  = echo_serv

CLIENT_SRC   = echo_client.cpp
SERVER_SRC   = tcp.cpp

BOOST = -lboost_program_options

all: debug release

$(CLIENT_EXEC): $(CLIENT_SRC)
	$(Compiler) $(FLAGS) $(CLIENT_SRC) -o $(CLIENT_EXEC) $(BOOST)

$(SERVER_EXEC): $(SERVER_SRC)
	$(Compiler) $(FLAGS) $(SERVER_SRC) -o $(SERVER_EXEC) $(BOOST)

# Цель для отладочной сборки
debug: FLAGS = $(DEBUG_FLAGS)
debug: $(CLIENT_EXEC) $(SERVER_EXEC)

# Цель для релизной сборки
release: FLAGS = $(RELEASE_FLAGS)
release: $(CLIENT_EXEC) $(SERVER_EXEC)

# Очистка скомпилированных файлов
clean:
	rm -f $(CLIENT_EXEC) $(SERVER_EXEC)
