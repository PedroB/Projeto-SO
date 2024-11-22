CC = gcc

# Para mais informações sobre as flags de warning, consulte a informação adicional no lab_ferramentas
CFLAGS = -g -std=c17 -D_POSIX_C_SOURCE=200809L \
		 -Wall -Werror -Wextra \
		 -Wcast-align -Wconversion -Wfloat-equal -Wformat=2 -Wnull-dereference -Wshadow -Wsign-conversion -Wswitch-enum -Wundef -Wunreachable-code -Wunused \
		 -fsanitize=address -fsanitize=undefined

ifneq ($(shell uname -s),Darwin) # if not MacOS
	CFLAGS += -fmax-errors=5
endif

all: kvs

kvs: main.c constants.h operations.o parser.o kvs.o
	$(CC) $(CFLAGS) $(SLEEP) -o kvs main.c operations.o parser.o kvs.o

%.o: %.c %.h
	$(CC) $(CFLAGS) -c ${@:.o=.c}

run: kvs
	@./kvs

# Regra para rodar os testes
run_tests: all
	@echo "Executando testes..."
	@python3 generate_tests.py  # Gera os arquivos .job e esperados
	@for job in tests/*.job; do \
		echo "Testando $$job..."; \
		./kvs < $$job > tests/$$(basename $$job .job).out; \
		diff tests/$$(basename $$job .job).out tests/expected/$$(basename $$job .job).out || echo "Erro no teste $$job!"; \
	done
	@echo "Todos os testes passaram!"

# Limpeza de arquivos gerados
clean:
	rm -f *.o kvs tests/*.out

format:
	@which clang-format >/dev/null 2>&1 || echo "Please install clang-format to run this command"
	clang-format -i *.c *.h
