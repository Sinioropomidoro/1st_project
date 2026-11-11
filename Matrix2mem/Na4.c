#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 4

int board[N][N];
int score = 0;

#define CELL_W 7
#define RESET "\033[0m"

const char *tile_color(int value)
{
    switch (value) {
        case 0:    return "\033[90m";
        case 2:    return "\033[97m";
        case 4:    return "\033[93m";
        case 8:    return "\033[33m";
        case 16:   return "\033[91m";
        case 32:   return "\033[31m";
        case 64:   return "\033[95m";
        case 128:  return "\033[92m";
        case 256:  return "\033[32m";
        case 512:  return "\033[96m";
        case 1024: return "\033[36m";
        case 2048: return "\033[1;94m";
        default:   return "\033[1;95m";
    }
}

void print_border(void)
{
    printf("+");
    for (int k = 0; k < N * CELL_W; k++) {
        printf("-");
    }
    printf("+\n");
}

void print_board(void)
{
    int total_w = N * CELL_W + 2;
    char buf[64];
    int len = snprintf(buf, sizeof(buf), "score: %d", score);
    int pad = (total_w - len) / 2;
    if (pad < 0) {
        pad = 0;
    }

    printf("\n");
    print_border();
    for (int i = 0; i < N; i++) {
        printf("|");
        for (int j = 0; j < N; j++) {
            printf("%s%*d %s", tile_color(board[i][j]), CELL_W - 1, board[i][j], RESET);
        }
        printf("|\n");
    }
    print_border();
    printf("%*s%s\n", pad, "", buf);
}

int add_random_tile(void)
{
    int empty_i[N * N], empty_j[N * N];
    int count = 0;

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (board[i][j] == 0) {
                empty_i[count] = i;
                empty_j[count] = j;
                count++;
            }
        }
    }
    if (count == 0) {
        return 0;
    }

    int k = rand() % count;
    board[empty_i[k]][empty_j[k]] = (rand() % 10 == 0) ? 4 : 2;
    return 1;
}

int slide_line(int line[N])
{
    int result[N] = {0};
    int pos = 0;
    int last_merged = 0;
    int changed = 0;

    for (int i = 0; i < N; i++) {
        if (line[i] == 0) {
            continue;
        }
        if (pos > 0 && result[pos - 1] == line[i] && !last_merged) {

            result[pos - 1] *= 2;
            score += result[pos - 1];
            last_merged = 1;
        } else {
            result[pos] = line[i];
            pos++;
            last_merged = 0;
        }
    }

    for (int i = 0; i < N; i++) {
        if (line[i] != result[i]) {
            changed = 1;
        }
        line[i] = result[i];
    }
    return changed;
}

int move(char dir)
{
    int changed = 0;
    int line[N];

    for (int k = 0; k < N; k++) {

        for (int t = 0; t < N; t++) {
            switch (dir) {
                case 'a': line[t] = board[k][t];         break;
                case 'd': line[t] = board[k][N - 1 - t]; break;
                case 'w': line[t] = board[t][k];         break;
                case 's': line[t] = board[N - 1 - t][k]; break;
            }
        }

        if (slide_line(line)) {
            changed = 1;
        }

        for (int t = 0; t < N; t++) {
            switch (dir) {
                case 'a': board[k][t]         = line[t]; break;
                case 'd': board[k][N - 1 - t] = line[t]; break;
                case 'w': board[t][k]         = line[t]; break;
                case 's': board[N - 1 - t][k] = line[t]; break;
            }
        }
    }
    return changed;
}

int has_moves(void)
{
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (board[i][j] == 0) {
                return 1;
            }
            if (j + 1 < N && board[i][j] == board[i][j + 1]) {
                return 1;
            }
            if (i + 1 < N && board[i][j] == board[i + 1][j]) {
                return 1;
            }
        }
    }
    return 0;
}

int main(void)
{
    srand((unsigned)time(NULL));

    add_random_tile();
    add_random_tile();

    char c;
    while (1) {
        print_board();

        if (!has_moves()) {
            printf("Ходов не осталось. Игра окончена! Итоговый счёт: %d\n", score);
            break;
        }

        printf("Ход (w/a/s/d, q - выход): ");
        if (scanf(" %c", &c) != 1) {
            break;
        }

        if (c == 'q') {
            printf("Выход. Счёт: %d\n", score);
            break;
        }
        if (c != 'w' && c != 'a' && c != 's' && c != 'd') {
            printf("Неизвестная команда.\n");
            continue;
        }

        if (move(c)) {
            add_random_tile();
        }
    }

    return 0;
}