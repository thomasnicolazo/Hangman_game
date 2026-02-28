all:
	gcc hangman.c -o hangman -lncurses

rm:
	rm hangman