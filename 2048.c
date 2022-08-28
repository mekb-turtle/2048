#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <termios.h>
#include <signal.h>
#define PAD 6
#define PADV 2
#define PADP 2
#define SIZE 4
int newlines;
#define newline() { ++newlines; printf("\n"); }
uint8_t     board[SIZE][SIZE];
uint8_t old_board[SIZE][SIZE];
void render_tile(uint8_t n, bool is_number) {
	char *s = malloc(PAD+PADP);
	for (uint8_t i = 0; i < PAD+PADP-1; ++i) { s[i] = ' '; } s[PAD+PADP-1] = '\0';
	if (is_number) {
		unsigned long num;
		num = n == 0 ? 0 : 1ul << (unsigned long)(n-1);
		uint8_t len = 1; {
			unsigned long num1 = num;
			while (num1 >= 10) { ++len; num1 /= 10; }
		}
		for (uint8_t i = 0; i < len; ++i) {
			s[(PAD-len)-i] = (num % 10) + '0';
			num /= 10;
		}
	}
	printf("%s", s);
}
void render_row(uint8_t i[SIZE], bool is_number) {
	for (int j = 0; j < SIZE; ++j) {
		render_tile(i[j], is_number);
	}
}
void render_board() {
	newlines = 0;
	newline();
	for (int i = 0; i < SIZE; ++i) {
		for (int j = 0; j < PADV+(i==0); ++j) {
			render_row(board[i], 0); newline();
		}
		render_row(board[i], 1); newline();
		for (int j = 0; j < PADV+(i==SIZE-1); ++j) {
			render_row(board[i], 0); newline();
		}
	}
	newline();
	newline();
}
bool move(int8_t i, int8_t j) {
	memcpy(old_board, board, SIZE*SIZE);
	board[0][0] = 1;
	return memcmp(old_board, board, SIZE*SIZE) != 0;
}
bool spawn_random() {
	return 1;
}
struct termios term_old;
bool is_tty;
void end(int code) {
	if (tcsetattr(STDIN_FILENO, TCSANOW, &term_old) != 0) code = code || 1;
	exit(code);
}
void end1() { end(1); }
bool turn = 0;
int main() {
	memset(board, 0, SIZE*SIZE);
	is_tty = isatty(STDIN_FILENO);
	if (is_tty) {
		struct termios term_new;
		if (tcgetattr(STDIN_FILENO, &term_old) != 0) return 1;
		term_new = term_old;
		term_new.c_lflag &= ~(ICANON | ISIG | ECHOK | ECHONL | ECHO | ECHOKE | IEXTEN);
		if (tcsetattr(STDIN_FILENO, TCSANOW, &term_new) != 0) return 1;
	}
	signal(SIGTSTP, end1);
	signal(SIGINT,  end1);
	signal(SIGSTOP, end1);
	signal(SIGTERM, end1);
	signal(SIGHUP,  end1);
	while (1) {
		if (turn) {
			for (int j = 0; j < newlines; ++j) printf("\x1bM");
			newlines = 0;
		}
		turn = 1;
		render_board();
		while (1) {
			int b = fgetc(stdin);
			if (b < 0 || b == 4 || b == 26 || b == 17) end(1); else
			if (b == 3) end(0); else
			if (b == 'w' || b == 'A') { if (!move( 0, -1)) continue; else break; } else
			if (b == 's' || b == 'B') { if (!move( 0, +1)) continue; else break; } else
			if (b == 'd' || b == 'C') { if (!move(+1,  0)) continue; else break; } else
			if (b == 'a' || b == 'D') { if (!move(-1,  0)) continue; else break; }
		}
		if (!spawn_random()) { printf("Game over!"); break; }
	}
	end(0);
	return 0;
}
