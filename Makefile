SRCFILES=src/main.c src/confparse.c src/dirdiscover.c src/compile.c src/link.c src/utils.c
build: $(SRCFILES)
	gcc $(SRCFILES) -o main -O0 -g -Wall -Wextra -Wpedantic -linih
	cp main test/main
check:
	clang-tidy $(SRCFILES)
