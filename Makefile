SRCFILES=src/main.c src/confparse.c src/dirdiscover.c src/compile.c src/link.c src/utils.c src/ini.c
build: $(SRCFILES)
	x86_64-w64-mingw32-gcc $(SRCFILES) -o main -O0 -g -Wall -Wextra -Wpedantic
	gcc $(SRCFILES) -o main -O0 -g -Wall -Wextra -Wpedantic
	cp main test/main
check:
	clang-tidy $(SRCFILES)
