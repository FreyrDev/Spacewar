SRC = src/main.c src/utils.c
LNK = -lm -lncursesw
OUT = spacewar

cr: $(SRC)
	gcc -o $(OUT) $(SRC) $(LNK) && ./$(OUT)

c: $(SRC)
	gcc -o $(OUT) $(SRC) $(LNK)

r:
	./$(OUT)
