/*
 * Игра 2048 на C с графическим окном (SDL2): меню, ники, рейтинг, продолжение игры.
 *
 * Сборка (Linux / WSL):
 *     gcc game2048_gui.c -o game2048_gui $(sdl2-config --cflags --libs)
 *
 * Файлы, которые игра создаёт рядом с собой:
 *     players.txt - ники игроков и их лучший счёт (рейтинг)
 *     save.txt    - текущая незаконченная игра (для "CONTINUE")
 *     settings.txt - выбранная тема (клавиша T: тёмная / светлая)
 */

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 4
#define TILE 90
#define GAP 10
#define BOARD_SIZE (N * TILE + (N + 1) * GAP)
#define BOARD_X 20
#define BOARD_Y 120
#define WIN_W (BOARD_X * 2 + BOARD_SIZE)
#define WIN_H (BOARD_Y + BOARD_SIZE + 50)

#define MAX_PLAYERS 50
#define NICK_MAX 10
#define PICK_ROWS 8
#define PLAYERS_FILE "players.txt"
#define SAVE_FILE "save.txt"

#define SETTINGS_FILE "settings.txt"

/* Цвета интерфейса: заполняются функцией apply_theme() */
static SDL_Color C_BG, C_DARK, C_WHITE, C_FRAME, C_PALE, C_GRAY, C_SEL;
static SDL_Color C_BOX, C_LABEL, C_EMPTY, C_OVERLAY;
static int dark_theme = 1;     /* 1 - тёмная тема (по умолчанию), 0 - светлая */

static void apply_theme(void)
{
    if (dark_theme) {
        C_BG      = (SDL_Color){  6,   4,   7, 255};   /* 060407 */
        C_DARK    = (SDL_Color){152, 130, 185, 255};   /* 9882B9 - основной текст */
        C_WHITE   = (SDL_Color){240, 232, 245, 255};
        C_FRAME   = (SDL_Color){ 34,  25,  50, 255};   /* 221932 - рамка поля */
        C_PALE    = (SDL_Color){ 34,  25,  50, 255};   /* 221932 - кнопки, строки */
        C_GRAY    = (SDL_Color){119,  94, 136, 255};   /* 775E88 - подсказки */
        C_SEL     = (SDL_Color){114,  37,  72, 255};   /* 722548 - выбранный пункт */
        C_BOX     = (SDL_Color){ 84,  21,  51, 255};   /* 541533 - окошки счёта */
        C_LABEL   = (SDL_Color){152, 130, 185, 255};
        C_EMPTY   = (SDL_Color){  6,   4,   7, 255};
        C_OVERLAY = (SDL_Color){  6,   4,   7, 200};
    } else {
        C_BG      = (SDL_Color){250, 248, 239, 255};
        C_DARK    = (SDL_Color){119, 110, 101, 255};
        C_WHITE   = (SDL_Color){255, 255, 255, 255};
        C_FRAME   = (SDL_Color){187, 173, 160, 255};
        C_PALE    = (SDL_Color){238, 228, 218, 255};
        C_GRAY    = (SDL_Color){160, 150, 140, 255};
        C_SEL     = (SDL_Color){143, 122, 102, 255};
        C_BOX     = (SDL_Color){187, 173, 160, 255};
        C_LABEL   = (SDL_Color){238, 228, 218, 255};
        C_EMPTY   = (SDL_Color){205, 193, 180, 255};
        C_OVERLAY = (SDL_Color){238, 228, 218, 170};
    }
}

static void load_theme(void)
{
    FILE *f = fopen(SETTINGS_FILE, "r");
    int v;
    if (f != NULL) {
        if (fscanf(f, "%d", &v) == 1) {
            dark_theme = (v != 0);
        }
        fclose(f);
    }
    apply_theme();
}

static void toggle_theme(void)
{
    dark_theme = !dark_theme;
    apply_theme();
    FILE *f = fopen(SETTINGS_FILE, "w");
    if (f != NULL) {
        fprintf(f, "%d\n", dark_theme);
        fclose(f);
    }
}

/* ---------- Данные ---------- */

typedef struct {
    char nick[NICK_MAX + 1];
    int best;
} Player;

typedef enum { S_MENU, S_PICK, S_NICK, S_GAME } State;

int board[N][N];
int score = 0;

Player players[MAX_PLAYERS];
int player_count = 0;
int cur = -1;                 /* индекс текущего игрока в players[] */

State state = S_MENU;
int have_save = 0;            /* есть ли сохранённая игра */
int menu_sel = 0;
int pick_sel = 0;
int pick_scroll = 0;
char nick_buf[NICK_MAX + 1];
int nick_len = 0;
int sorted_idx[MAX_PLAYERS];

/* ---------- Логика игры ---------- */

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

void new_game(void)
{
    memset(board, 0, sizeof(board));
    score = 0;
    add_random_tile();
    add_random_tile();
}

/* ---------- Игроки, рейтинг и сохранения (файлы) ---------- */

static void load_players(void)
{
    FILE *f = fopen(PLAYERS_FILE, "r");
    if (f == NULL) {
        return;
    }
    while (player_count < MAX_PLAYERS &&
           fscanf(f, "%10s %d", players[player_count].nick, &players[player_count].best) == 2) {
        player_count++;
    }
    fclose(f);
}

static void save_players(void)
{
    FILE *f = fopen(PLAYERS_FILE, "w");
    if (f == NULL) {
        return;
    }
    for (int i = 0; i < player_count; i++) {
        fprintf(f, "%s %d\n", players[i].nick, players[i].best);
    }
    fclose(f);
}

static int find_player(const char *nick)
{
    for (int i = 0; i < player_count; i++) {
        if (strcmp(players[i].nick, nick) == 0) {
            return i;
        }
    }
    return -1;
}

static int add_player(const char *nick)
{
    if (player_count >= MAX_PLAYERS) {
        return -1;
    }
    strncpy(players[player_count].nick, nick, NICK_MAX);
    players[player_count].nick[NICK_MAX] = '\0';
    players[player_count].best = 0;
    return player_count++;
}

/* Заполняет sorted_idx индексами игроков по убыванию лучшего счёта */
static void build_ranking(void)
{
    for (int i = 0; i < player_count; i++) {
        sorted_idx[i] = i;
    }
    for (int i = 1; i < player_count; i++) {
        int v = sorted_idx[i];
        int j = i - 1;
        while (j >= 0 && players[sorted_idx[j]].best < players[v].best) {
            sorted_idx[j + 1] = sorted_idx[j];
            j--;
        }
        sorted_idx[j + 1] = v;
    }
}

static int read_save(char *nick, int *sc, int b[N][N])
{
    FILE *f = fopen(SAVE_FILE, "r");
    if (f == NULL) {
        return 0;
    }
    int ok = (fscanf(f, "%10s %d", nick, sc) == 2);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (ok && fscanf(f, "%d", &b[i][j]) != 1) {
                ok = 0;
            }
        }
    }
    fclose(f);
    return ok;
}

static void write_save(void)
{
    if (cur < 0) {
        return;
    }
    FILE *f = fopen(SAVE_FILE, "w");
    if (f == NULL) {
        return;
    }
    fprintf(f, "%s %d\n", players[cur].nick, score);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            fprintf(f, "%d ", board[i][j]);
        }
        fprintf(f, "\n");
    }
    fclose(f);
}

static void refresh_save_flag(void)
{
    char nick[NICK_MAX + 1];
    int sc;
    int tb[N][N];
    have_save = read_save(nick, &sc, tb);
}

static void begin_game(int player_index)
{
    cur = player_index;
    new_game();
    write_save();
    state = S_GAME;
}

static void continue_game(void)
{
    char nick[NICK_MAX + 1];
    int sc;
    int tb[N][N];

    if (!read_save(nick, &sc, tb)) {
        have_save = 0;
        return;
    }
    int idx = find_player(nick);
    if (idx < 0) {
        idx = add_player(nick);
    }
    if (idx < 0) {
        return;
    }
    cur = idx;
    score = sc;
    memcpy(board, tb, sizeof(board));
    state = S_GAME;
}

static void start_new_flow(void)
{
    if (player_count == 0) {
        nick_len = 0;
        nick_buf[0] = '\0';
        state = S_NICK;
    } else {
        pick_sel = 0;
        pick_scroll = 0;
        state = S_PICK;
    }
}

/* ---------- Пиксельный шрифт 5x5 (SDL_ttf не нужна) ---------- */

typedef struct {
    char ch;
    const char *rows[5];
} Glyph;

static const Glyph font[] = {
    {'0', {"01110", "10001", "10001", "10001", "01110"}},
    {'1', {"00100", "01100", "00100", "00100", "01110"}},
    {'2', {"01110", "10001", "00110", "01000", "11111"}},
    {'3', {"11110", "00001", "01110", "00001", "11110"}},
    {'4', {"10010", "10010", "11111", "00010", "00010"}},
    {'5', {"11111", "10000", "11110", "00001", "11110"}},
    {'6', {"01110", "10000", "11110", "10001", "01110"}},
    {'7', {"11111", "00001", "00010", "00100", "00100"}},
    {'8', {"01110", "10001", "01110", "10001", "01110"}},
    {'9', {"01110", "10001", "01111", "00001", "01110"}},
    {'A', {"01110", "10001", "11111", "10001", "10001"}},
    {'B', {"11110", "10001", "11110", "10001", "11110"}},
    {'C', {"01110", "10001", "10000", "10001", "01110"}},
    {'D', {"11110", "10001", "10001", "10001", "11110"}},
    {'E', {"11111", "10000", "11110", "10000", "11111"}},
    {'F', {"11111", "10000", "11110", "10000", "10000"}},
    {'G', {"01111", "10000", "10011", "10001", "01111"}},
    {'H', {"10001", "10001", "11111", "10001", "10001"}},
    {'I', {"01110", "00100", "00100", "00100", "01110"}},
    {'J', {"00111", "00010", "00010", "10010", "01100"}},
    {'K', {"10001", "10010", "11100", "10010", "10001"}},
    {'L', {"10000", "10000", "10000", "10000", "11111"}},
    {'M', {"10001", "11011", "10101", "10001", "10001"}},
    {'N', {"10001", "11001", "10101", "10011", "10001"}},
    {'O', {"01110", "10001", "10001", "10001", "01110"}},
    {'P', {"11110", "10001", "11110", "10000", "10000"}},
    {'Q', {"01110", "10001", "10101", "10010", "01101"}},
    {'R', {"11110", "10001", "11110", "10010", "10001"}},
    {'S', {"01111", "10000", "01110", "00001", "11110"}},
    {'T', {"11111", "00100", "00100", "00100", "00100"}},
    {'U', {"10001", "10001", "10001", "10001", "01110"}},
    {'V', {"10001", "10001", "10001", "01010", "00100"}},
    {'W', {"10001", "10001", "10101", "11011", "10001"}},
    {'X', {"10001", "01010", "00100", "01010", "10001"}},
    {'Y', {"10001", "01010", "00100", "00100", "00100"}},
    {'Z', {"11111", "00010", "00100", "01000", "11111"}},
    {'-', {"00000", "00000", "11111", "00000", "00000"}},
    {'+', {"00000", "00100", "01110", "00100", "00000"}},
    {'/', {"00001", "00010", "00100", "01000", "10000"}},
    {'.', {"00000", "00000", "00000", "00000", "00100"}},
    {':', {"00000", "00100", "00000", "00100", "00000"}},
    {'>', {"01000", "00100", "00010", "00100", "01000"}},
    {'?', {"01110", "00001", "00110", "00000", "00100"}},
};

static const Glyph *find_glyph(char c)
{
    int count = (int)(sizeof(font) / sizeof(font[0]));
    for (int i = 0; i < count; i++) {
        if (font[i].ch == c) {
            return &font[i];
        }
    }
    return NULL;
}

static int text_width(const char *s, int px)
{
    int len = (int)strlen(s);
    if (len == 0) {
        return 0;
    }
    return len * 6 * px - px;
}

static void fill_rect(SDL_Renderer *r, int x, int y, int w, int h, SDL_Color c)
{
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderFillRect(r, &rect);
}

static void draw_text(SDL_Renderer *r, const char *s, int x, int y, int px, SDL_Color c)
{
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
    for (; *s; s++) {
        const Glyph *g = find_glyph(*s);
        if (g != NULL) {
            for (int row = 0; row < 5; row++) {
                for (int col = 0; col < 5; col++) {
                    if (g->rows[row][col] == '1') {
                        SDL_Rect dot = {x + col * px, y + row * px, px, px};
                        SDL_RenderFillRect(r, &dot);
                    }
                }
            }
        }
        x += 6 * px;
    }
}

static void draw_text_centered(SDL_Renderer *r, const char *s, int center_x, int y, int px, SDL_Color c)
{
    draw_text(r, s, center_x - text_width(s, px) / 2, y, px, c);
}

static void draw_text_right(SDL_Renderer *r, const char *s, int right_x, int y, int px, SDL_Color c)
{
    draw_text(r, s, right_x - text_width(s, px), y, px, c);
}

/* ---------- Отрисовка экранов ---------- */

static SDL_Color tile_color(int v)
{
    if (dark_theme) {
        switch (v) {
            case 0:    return C_EMPTY;
            case 2:    return (SDL_Color){119,  94, 136, 255};   /* 775E88 */
            case 4:    return (SDL_Color){152, 130, 185, 255};   /* 9882B9 */
            case 8:    return (SDL_Color){114,  37,  72, 255};   /* 722548 */
            case 16:   return (SDL_Color){143,  45,  90, 255};
            case 32:   return (SDL_Color){176,  58, 110, 255};
            case 64:   return (SDL_Color){209,  74, 130, 255};
            case 128:  return (SDL_Color){183, 154, 214, 255};
            case 256:  return (SDL_Color){201, 178, 227, 255};
            case 512:  return (SDL_Color){218, 200, 238, 255};
            case 1024: return (SDL_Color){232, 220, 245, 255};
            case 2048: return (SDL_Color){255, 211, 232, 255};
            default:   return (SDL_Color){ 84,  21,  51, 255};   /* 541533 */
        }
    }
    switch (v) {
        case 0:    return C_EMPTY;
        case 2:    return (SDL_Color){238, 228, 218, 255};
        case 4:    return (SDL_Color){237, 224, 200, 255};
        case 8:    return (SDL_Color){242, 177, 121, 255};
        case 16:   return (SDL_Color){245, 149,  99, 255};
        case 32:   return (SDL_Color){246, 124,  95, 255};
        case 64:   return (SDL_Color){246,  94,  59, 255};
        case 128:  return (SDL_Color){237, 207, 114, 255};
        case 256:  return (SDL_Color){237, 204,  97, 255};
        case 512:  return (SDL_Color){237, 200,  80, 255};
        case 1024: return (SDL_Color){237, 197,  63, 255};
        case 2048: return (SDL_Color){237, 194,  46, 255};
        default:   return (SDL_Color){ 60,  58,  50, 255};
    }
}

static SDL_Color tile_text_color(int v)
{
    if (dark_theme) {
        if (v == 4 || v >= 128) {
            return (SDL_Color){34, 25, 50, 255};       /* 221932 */
        }
        return C_WHITE;
    }
    return (v <= 4) ? C_DARK : C_WHITE;
}

static int number_px(int len)
{
    if (len <= 4) {
        return 4;
    }
    return (len == 5) ? 3 : 2;
}

static void draw_score_box(SDL_Renderer *r, const char *label, int value, int x)
{
    char buf[32];
    int box_w = 110, box_h = 70;

    snprintf(buf, sizeof(buf), "%d", value);
    fill_rect(r, x, 25, box_w, box_h, C_BOX);
    draw_text_centered(r, label, x + box_w / 2, 33, 2, C_LABEL);
    draw_text_centered(r, buf, x + box_w / 2, 55, number_px((int)strlen(buf)), C_WHITE);
}

static void draw_ranking(SDL_Renderer *r, int y)
{
    char buf[32];

    draw_text_centered(r, "TOP PLAYERS", WIN_W / 2, y, 3, C_DARK);
    if (player_count == 0) {
        draw_text_centered(r, "NO PLAYERS YET", WIN_W / 2, y + 40, 3, C_GRAY);
        return;
    }

    build_ranking();
    int rows = (player_count < 5) ? player_count : 5;
    for (int i = 0; i < rows; i++) {
        const Player *p = &players[sorted_idx[i]];
        int ry = y + 35 + i * 30;

        snprintf(buf, sizeof(buf), "%d", i + 1);
        draw_text(r, buf, 40, ry, 3, C_DARK);
        draw_text(r, p->nick, 76, ry, 3, C_DARK);
        snprintf(buf, sizeof(buf), "%d", p->best);
        draw_text_right(r, buf, WIN_W - 40, ry, 3, C_DARK);
    }
}

static void render_menu(SDL_Renderer *r)
{
    const char *items[3];
    int n = 0;

    if (have_save) {
        items[n++] = "CONTINUE";
    }
    items[n++] = "NEW GAME";
    items[n++] = "QUIT";

    draw_text_centered(r, "2048", WIN_W / 2, 40, 10, C_DARK);

    for (int i = 0; i < n; i++) {
        int y = 140 + i * 64;
        int sel = (i == menu_sel);
        fill_rect(r, 95, y, 260, 50, sel ? C_SEL : C_PALE);
        draw_text_centered(r, items[i], WIN_W / 2, y + 15, 4, sel ? C_WHITE : C_DARK);
    }

    draw_ranking(r, 350);
    draw_text_centered(r, "UP/DOWN - SELECT   ENTER - OK", WIN_W / 2, 546, 2, C_GRAY);
    draw_text_centered(r, "T - CHANGE THEME", WIN_W / 2, 562, 2, C_GRAY);
}

static void render_pick(SDL_Renderer *r)
{
    char buf[32];
    int count = player_count + 1;

    draw_text_centered(r, "PLAYER", WIN_W / 2, 30, 7, C_DARK);

    for (int v = 0; v < PICK_ROWS && pick_scroll + v < count; v++) {
        int i = pick_scroll + v;
        int y = 110 + v * 48;
        int sel = (i == pick_sel);
        SDL_Color tc = sel ? C_WHITE : C_DARK;

        fill_rect(r, 40, y, 370, 40, sel ? C_SEL : C_PALE);
        if (i == 0) {
            draw_text(r, "+ NEW NICK", 56, y + 12, 3, tc);
        } else {
            draw_text(r, players[i - 1].nick, 56, y + 12, 3, tc);
            snprintf(buf, sizeof(buf), "%d", players[i - 1].best);
            draw_text_right(r, buf, 394, y + 12, 3, tc);
        }
    }

    draw_text_centered(r, "UP/DOWN - SELECT   ENTER - OK   ESC - BACK", WIN_W / 2, 555, 2, C_GRAY);
}

static void render_nick(SDL_Renderer *r)
{
    draw_text_centered(r, "NICK", WIN_W / 2, 60, 8, C_DARK);
    draw_text_centered(r, "TYPE LETTERS OR DIGITS", WIN_W / 2, 140, 2, C_GRAY);

    fill_rect(r, 55, 190, 340, 80, C_PALE);
    draw_text(r, nick_buf, 70, 205, 5, C_DARK);
    fill_rect(r, 70 + nick_len * 30, 238, 24, 5, C_SEL);

    draw_text_centered(r, "MAX 10 CHARS", WIN_W / 2, 290, 2, C_GRAY);
    draw_text_centered(r, "ENTER - OK   ESC - BACK", WIN_W / 2, 555, 2, C_GRAY);
}

static void render_game(SDL_Renderer *r)
{
    char buf[32];

    /* шапка: заголовок, ник, счёт и личный рекорд */
    draw_text(r, "2048", BOARD_X, 24, 6, C_DARK);
    if (cur >= 0) {
        draw_text(r, players[cur].nick, BOARD_X, 70, 3, C_DARK);
        draw_score_box(r, "BEST", players[cur].best, WIN_W - BOARD_X - 110);
    }
    draw_score_box(r, "SCORE", score, WIN_W - BOARD_X - 230);

    /* поле */
    fill_rect(r, BOARD_X, BOARD_Y, BOARD_SIZE, BOARD_SIZE, C_FRAME);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            int v = board[i][j];
            int x = BOARD_X + GAP + j * (TILE + GAP);
            int y = BOARD_Y + GAP + i * (TILE + GAP);
            fill_rect(r, x, y, TILE, TILE, tile_color(v));

            if (v != 0) {
                snprintf(buf, sizeof(buf), "%d", v);
                int len = (int)strlen(buf);
                int tpx = (len <= 2) ? 7 : (len == 3) ? 5 : (len == 4) ? 3 : 2;
                SDL_Color tc = tile_text_color(v);
                draw_text_centered(r, buf, x + TILE / 2, y + TILE / 2 - (5 * tpx) / 2, tpx, tc);
            }
        }
    }

    /* подсказки по управлению снизу */
    int hy = BOARD_Y + BOARD_SIZE + 12;
    draw_text_centered(r, "ARROWS / WASD - MOVE", WIN_W / 2, hy, 2, C_GRAY);
    draw_text_centered(r, "R - RESTART  T - THEME  ESC - MENU", WIN_W / 2, hy + 16, 2, C_GRAY);

    /* конец игры */
    if (!has_moves()) {
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        fill_rect(r, BOARD_X, BOARD_Y, BOARD_SIZE, BOARD_SIZE, C_OVERLAY);
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
        int cx = BOARD_X + BOARD_SIZE / 2;
        int cy = BOARD_Y + BOARD_SIZE / 2;
        draw_text_centered(r, "GAME OVER", cx, cy - 50, 5, C_DARK);
        draw_text_centered(r, "R - NEW GAME", cx, cy + 10, 3, C_DARK);
        draw_text_centered(r, "ESC - MENU", cx, cy + 45, 3, C_DARK);
    }
}

static void render(SDL_Renderer *r)
{
    fill_rect(r, 0, 0, WIN_W, WIN_H, C_BG);
    switch (state) {
        case S_MENU: render_menu(r); break;
        case S_PICK: render_pick(r); break;
        case S_NICK: render_nick(r); break;
        case S_GAME: render_game(r); break;
    }
    SDL_RenderPresent(r);
}

/* ---------- Обработка клавиш ---------- */

static int is_enter(SDL_Scancode sc)
{
    return sc == SDL_SCANCODE_RETURN || sc == SDL_SCANCODE_KP_ENTER;
}

static int is_up(SDL_Scancode sc)
{
    return sc == SDL_SCANCODE_UP || sc == SDL_SCANCODE_W;
}

static int is_down(SDL_Scancode sc)
{
    return sc == SDL_SCANCODE_DOWN || sc == SDL_SCANCODE_S;
}

/* Возвращает 0, если нужно выйти из программы */
static int handle_key(SDL_Scancode sc)
{
    if (sc == SDL_SCANCODE_T && state != S_NICK) {
        toggle_theme();
        return 1;
    }

    switch (state) {

    case S_MENU: {
        int count = have_save ? 3 : 2;
        if (is_up(sc)) {
            menu_sel = (menu_sel + count - 1) % count;
        } else if (is_down(sc)) {
            menu_sel = (menu_sel + 1) % count;
        } else if (is_enter(sc)) {
            int action = have_save ? menu_sel : menu_sel + 1;   /* 0 continue, 1 new, 2 quit */
            if (action == 0) {
                continue_game();
            } else if (action == 1) {
                start_new_flow();
            } else {
                return 0;
            }
        } else if (sc == SDL_SCANCODE_ESCAPE) {
            return 0;
        }
        break;
    }

    case S_PICK: {
        int count = player_count + 1;
        if (is_up(sc)) {
            pick_sel = (pick_sel + count - 1) % count;
        } else if (is_down(sc)) {
            pick_sel = (pick_sel + 1) % count;
        } else if (is_enter(sc)) {
            if (pick_sel == 0) {
                nick_len = 0;
                nick_buf[0] = '\0';
                state = S_NICK;
            } else {
                begin_game(pick_sel - 1);
            }
        } else if (sc == SDL_SCANCODE_ESCAPE) {
            menu_sel = 0;
            state = S_MENU;
        }
        if (pick_sel < pick_scroll) {
            pick_scroll = pick_sel;
        }
        if (pick_sel >= pick_scroll + PICK_ROWS) {
            pick_scroll = pick_sel - PICK_ROWS + 1;
        }
        break;
    }

    case S_NICK: {
        char ch = 0;
        if (sc >= SDL_SCANCODE_A && sc <= SDL_SCANCODE_Z) {
            ch = (char)('A' + (sc - SDL_SCANCODE_A));
        } else if (sc >= SDL_SCANCODE_1 && sc <= SDL_SCANCODE_9) {
            ch = (char)('1' + (sc - SDL_SCANCODE_1));
        } else if (sc == SDL_SCANCODE_0) {
            ch = '0';
        } else if (sc == SDL_SCANCODE_MINUS) {
            ch = '-';
        }

        if (ch != 0) {
            if (nick_len < NICK_MAX) {
                nick_buf[nick_len++] = ch;
                nick_buf[nick_len] = '\0';
            }
        } else if (sc == SDL_SCANCODE_BACKSPACE) {
            if (nick_len > 0) {
                nick_buf[--nick_len] = '\0';
            }
        } else if (is_enter(sc)) {
            if (nick_len > 0) {
                int idx = find_player(nick_buf);
                if (idx < 0) {
                    idx = add_player(nick_buf);
                }
                if (idx >= 0) {
                    save_players();
                    begin_game(idx);
                }
            }
        } else if (sc == SDL_SCANCODE_ESCAPE) {
            menu_sel = 0;
            state = S_MENU;
        }
        break;
    }

    case S_GAME: {
        char dir = 0;
        if (sc == SDL_SCANCODE_LEFT  || sc == SDL_SCANCODE_A) dir = 'a';
        if (sc == SDL_SCANCODE_RIGHT || sc == SDL_SCANCODE_D) dir = 'd';
        if (sc == SDL_SCANCODE_UP    || sc == SDL_SCANCODE_W) dir = 'w';
        if (sc == SDL_SCANCODE_DOWN  || sc == SDL_SCANCODE_S) dir = 's';

        if (dir != 0) {
            if (has_moves() && move(dir)) {
                add_random_tile();
                if (score > players[cur].best) {
                    players[cur].best = score;
                    save_players();
                }
                if (has_moves()) {
                    write_save();
                } else {
                    remove(SAVE_FILE);
                }
            }
        } else if (sc == SDL_SCANCODE_R) {
            new_game();
            write_save();
        } else if (sc == SDL_SCANCODE_ESCAPE) {
            refresh_save_flag();
            menu_sel = 0;
            state = S_MENU;
        }
        break;
    }

    }
    return 1;
}

/* ---------- main ---------- */

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    srand((unsigned)time(NULL));
    load_theme();
    load_players();
    refresh_save_flag();

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Ошибка SDL_Init: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("2048",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          WIN_W, WIN_H, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        fprintf(stderr, "Ошибка SDL_CreateWindow: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (renderer == NULL) {
        fprintf(stderr, "Ошибка SDL_CreateRenderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    render(renderer);

    int running = 1;
    SDL_Event e;
    while (running && SDL_WaitEvent(&e)) {
        if (e.type == SDL_QUIT) {
            running = 0;
        } else if (e.type == SDL_KEYDOWN) {
            running = handle_key(e.key.keysym.scancode);
        }
        render(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}