CC = g++
CPPFLAGS = -std=c++14 -g

.PHONY: all
all: main

main: main.o Controller.o Operate.o Table.o
	$(CC) $^ -o $@

Controller.o: Controller.cpp Controller.h Operate.h DataBase.h Table.h BPTree.h BPTreeNode.h SubClass.h
	$(CC) $(CPPFLAGS) $< -o $@ -c

Operate.o: Operate.cpp Operate.h DataBase.h Table.h BPTree.h BPTreeNode.h SubClass.h
	$(CC) $(CPPFLAGS) $< -o $@ -c

Table.o: Table.cpp Table.h BPTree.h BPTreeNode.h SubClass.h
	$(CC) $(CPPFLAGS) $< -o $@ -c

main.o: main.cpp OOPDB.h Controller.h Operate.h DataBase.h Table.h BPTree.h BPTreeNode.h SubClass.h
	$(CC) $(CPPFLAGS) $< -o $@ -c

.PHONY: clean
clean:
	rm main *.o -f
