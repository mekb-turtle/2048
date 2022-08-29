#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <termios.h>
#include <signal.h>
#include <sys/time.h>
#include <poll.h>
#include <time.h>
#define delay() { render_board(); sleep_(30); }
#define PAD 8
#define PADV 2
#define PADP 2
#define SIZE 4
struct color {
	uint8_t fr, fg, fb, br, bg, bb;
	bool b;
} colors[] = {
	{ .fr = 0x00, .fg = 0x00, .fb = 0x00, .br = 0xee, .bg = 0xe4, .bb = 0xda, .b = 0 }, // 2
	{ .fr = 0x00, .fg = 0x00, .fb = 0x00, .br = 0xee, .bg = 0xe1, .bb = 0xc9, .b = 0 }, // 4
	{ .fr = 0x00, .fg = 0x00, .fb = 0x00, .br = 0xf3, .bg = 0xb2, .bb = 0x7a, .b = 0 }, // 8
	{ .fr = 0x00, .fg = 0x00, .fb = 0x00, .br = 0xf6, .bg = 0x96, .bb = 0x64, .b = 0 }, // 16
	{ .fr = 0xff, .fg = 0xff, .fb = 0xff, .br = 0xf7, .bg = 0x7c, .bb = 0x5f, .b = 0 }, // 32
	{ .fr = 0xff, .fg = 0xff, .fb = 0xff, .br = 0xf7, .bg = 0x5f, .bb = 0x3b, .b = 0 }, // 64
	{ .fr = 0x00, .fg = 0x00, .fb = 0x00, .br = 0xed, .bg = 0xd0, .bb = 0x73, .b = 0 }, // 128
	{ .fr = 0x00, .fg = 0x00, .fb = 0x00, .br = 0xed, .bg = 0xcc, .bb = 0x62, .b = 0 }, // 256
	{ .fr = 0x00, .fg = 0x00, .fb = 0x00, .br = 0xed, .bg = 0xc9, .bb = 0x50, .b = 1 }, // 512
	{ .fr = 0x00, .fg = 0x00, .fb = 0x00, .br = 0xed, .bg = 0xc5, .bb = 0x3f, .b = 1 }, // 1024
	{ .fr = 0x00, .fg = 0x00, .fb = 0x00, .br = 0xed, .bg = 0xc2, .bb = 0x2e, .b = 1 }, // 2048, this is the last official colour
	{ .fr = 0x00, .fg = 0x00, .fb = 0x00, .br = 0xe1, .bg = 0x96, .bb = 0xdf, .b = 0 }, // 4096
	{ .fr = 0xff, .fg = 0xff, .fb = 0xff, .br = 0xc6, .bg = 0x7d, .bb = 0xc4, .b = 0 }, // 8192
	{ .fr = 0xff, .fg = 0xff, .fb = 0xff, .br = 0xaf, .bg = 0x58, .bb = 0xc4, .b = 0 }, // 16384
	{ .fr = 0xff, .fg = 0xff, .fb = 0xff, .br = 0x8e, .bg = 0x0e, .bb = 0xc4, .b = 0 }, // 32768
	{ .fr = 0xff, .fg = 0xff, .fb = 0xff, .br = 0x6d, .bg = 0x00, .bb = 0xc4, .b = 0 }, // 65536
	{ .fr = 0xff, .fg = 0xff, .fb = 0xff, .br = 0x23, .bg = 0x24, .bb = 0x26, .b = 0 }, // 131072 and beyond
};
struct pos {
	uint8_t i, j;
};
unsigned long score = 0;
int newlines;
#define newline() { ++newlines; printf("\x1b[0m\n"); }
uint8_t      board[SIZE][SIZE];
uint8_t  old_board[SIZE][SIZE];
uint8_t old2_board[SIZE][SIZE];
void render_tile(uint8_t n, bool is_number) {
	char *s = malloc(PAD+PADP);
	for (uint8_t i = 0; i < PAD+PADP-1; ++i) { s[i] = ' '; } s[PAD+PADP-1] = '\0';
	if (is_number && n > 0) {
		unsigned long num;
		num = n == 0 ? 0 : 1ul << (unsigned long)n;
		uint8_t len = 1; {
			unsigned long num1 = num;
			while (num1 >= 10) { ++len; num1 /= 10; }
		}
		for (uint8_t i = 0; i < len; ++i) {
			s[(PAD+PADP+len)/2-i-1] = (num % 10) + '0';
			num /= 10;
		}
	}
	if (n > 0) {
		int l = (sizeof(colors) / sizeof(colors[0]));
		if (n > l) n = l;
		struct color c = colors[n-1];
		printf("\x1b[0;38;2;%i;%i;%i;48;2;%i;%i;%i%sm%s",
			c.fr, c.fg, c.fb,
			c.br, c.bg, c.bb,
			c.b ? ";1" : "",
			s);
	} else {
		printf("\x1b[0m%s", s);
	}
}
void sleep_(unsigned long ms) {
	struct timespec r = { ms/1e3, (ms%1000)*1e6 };
	nanosleep(&r, NULL);
}
void render_row(uint8_t i[SIZE], bool is_number) {
	for (int j = 0; j < SIZE; ++j) {
		render_tile(i[j], is_number);
	}
}
bool turn = 0;
void render_board() {
	if (turn) {
		for (int j = 0; j < newlines; ++j) printf("\x1bM");
	}
	turn = 1;
	newlines = 0;
	newline();
	for (int i = 0; i < SIZE; ++i) {
		for (int j = 0; j < PADV; ++j) {
			render_row(board[i], 0); newline();
		}
		render_row(board[i], 1); newline();
		for (int j = 0; j < PADV; ++j) {
			render_row(board[i], 0); newline();
		}
	}
	newline();
	newline();
}
unsigned int seedp;
int rand_() {
	return rand_r(&seedp);
}
void push(int8_t i_, int8_t j_) {
	for (int8_t o = 0; o < SIZE; ++o) {
		memcpy(old2_board, board, SIZE*SIZE);
		for (int8_t i = i_ > 0 ? SIZE - 1 - i_ : -i_; i_ > 0 ? i >= 0 : i < SIZE; i += i_ > 0 ? -1 : 1) {
		for (int8_t j = j_ > 0 ? SIZE - 1 - j_ : -j_; j_ > 0 ? j >= 0 : j < SIZE; j += j_ > 0 ? -1 : 1) {
			if (board[i][j] == 0) continue;
			if (board[i+i_][j+j_] != 0) continue;
			board[i+i_][j+j_] = board[i][j];
			board[i][j] = 0;
		}
		}
		if (memcmp(old2_board, board, SIZE*SIZE) != 0) delay() else return;
	}
}
void merge(int8_t i_, int8_t j_) {
	for (int8_t i = i_ > 0 ? SIZE - 1 - i_ : -i_; i_ > 0 ? i >= 0 : i < SIZE; i += i_ > 0 ? -1 : 1) {
	for (int8_t j = j_ > 0 ? SIZE - 1 - j_ : -j_; j_ > 0 ? j >= 0 : j < SIZE; j += j_ > 0 ? -1 : 1) {
		if (board[i][j] != board[i+i_][j+j_]) continue;
		if (board[i][j] == 0) continue;
		++board[i+i_][j+j_];
		board[i][j] = 0;
	}
	}
}
bool move(int8_t i, int8_t j) {
	memcpy(old_board, board, SIZE*SIZE);
	push(i, j);
	merge(i, j);
	delay();
	push(i, j);
	return memcmp(old_board, board, SIZE*SIZE) != 0;
}
bool spawn_random() {
	struct pos tile;
	struct pos empty_tiles[SIZE*SIZE];
	int l = 0;
	for (int i = 0; i < SIZE; ++i)
		for (int j = 0; j < SIZE; ++j) {
			if (board[i][j] == 0) empty_tiles[l++] = (struct pos) { .i = i, .j = j };
		}
	if (l == 0) return 0;
	tile = empty_tiles[rand_() % l];
	board[tile.i][tile.j] = rand_() % 10 == 0 ? 2 : 1;
	return 1;
}
bool is_game_over() {
	for (int i = 0; i < SIZE; ++i)
		for (int j = 0; j < SIZE; ++j) {
			if (i > 0)      if (board[i-1][j] == board[i][j]) return 0;
			if (j > 0)      if (board[i][j-1] == board[i][j]) return 0;
			if (i < SIZE-1) if (board[i+1][j] == board[i][j]) return 0;
			if (j < SIZE-1) if (board[i][j+1] == board[i][j]) return 0;
			if (board[i][j] == 0) return 0;
		}
	return 1;
}
struct termios term_old;
bool is_tty;
void end(int code) {
	if (tcsetattr(STDIN_FILENO, TCSANOW, &term_old) != 0) code = code || 1;
	exit(code);
}
void end1() { end(1); }
int poll_() {
	struct pollfd fds;
	fds.fd = STDIN_FILENO;
	fds.events = POLLIN;
	return poll(&fds, 1, 0);
}
int main() {
	struct timeval tv;
	if (gettimeofday(&tv, NULL) != 0) return 1;
	seedp = tv.tv_usec;
	memset(board, 0, SIZE*SIZE);
	/* int p = 0;
	for (int i = 0; i < SIZE; ++i)
		for (int j = 0; j < SIZE; ++j)
			board[i][j] = p++; */
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
	spawn_random();
	while (1) {
		render_board();
		if (is_game_over()) { printf("Game over!\n"); break; }
		while (poll_() > 0) {
			fgetc(stdin);
		}
		while (1) {
			int b = fgetc(stdin);
			if (b < 0 || b == 4 || b == 26 || b == 17) end(1); else
			if (b == 3) end(0); else
			if (b == 'w' || b == 'A') { if (!move(-1, 0)) continue; else break; } else
			if (b == 's' || b == 'B') { if (!move(+1, 0)) continue; else break; } else
			if (b == 'd' || b == 'C') { if (!move( 0,+1)) continue; else break; } else
			if (b == 'a' || b == 'D') { if (!move( 0,-1)) continue; else break; }
		}
		delay();
		spawn_random();
	}
	end(0);
	return 0;
}
