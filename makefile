SRC = src/main.c src/utils.c
LNK = -lm -lncursesw

cr: $(SRC)
	gcc -o out/spacewar $(SRC) $(LNK) && ./out/spacewar

c: $(SRC)
	gcc -o out/spacewar $(SRC) $(LNK)

r:
	./out/spacewar
