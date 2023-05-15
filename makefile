SRC = src/main.c
LNK = -lm -lncursesw

cr:
	make c && make r

c: $(SRC)
	gcc -o out/spacewar $(SRC) $(LNK)

r:
	./out/spacewar
