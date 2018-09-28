#ifndef NODE_HEARTS_MCTS_H
#define NODE_HEARTS_MCTS_H

#include <node_api.h>

#define MAX_CARDS_LEN 64
#define MAX_HAND_CARDS_LEN 16
#define MAX_CHILDREN_LEN 16

// #define NDEBUG

struct stru_me {
    char *name;
    int32_t deal_score;
    int32_t cards_count;
    char *round_card;
    char *cards[MAX_CARDS_LEN];
    char *candidate_cards[MAX_HAND_CARDS_LEN];
    char *score_cards[MAX_HAND_CARDS_LEN];
};

struct player {
    char *name;
    int32_t deal_score;
    int32_t cards_count;
    char *round_card;
    char *cards[MAX_CARDS_LEN];
    char *score_cards[MAX_HAND_CARDS_LEN];
};

struct node {
    char *move;
    struct node *parent;
    struct node *children[MAX_CHILDREN_LEN];
    double wins;
    uint32_t visits;
    char *untried_moves[MAX_CARDS_LEN];
};


napi_value Init(napi_env, napi_value exports);
napi_value uct();

int get_parameter_me(napi_env env, napi_value me_js_obj, struct stru_me *me);
int get_parameter_players(napi_env env, napi_value players_js_array, struct player players[]);
int get_player_info(napi_env env, napi_value player_js_obj, struct player *player);
int get_parameter_player_order(napi_env env, napi_value player_order_js_array, char *player_order[]);
int get_parameter_left_cards(napi_env env, napi_value left_cards_js_array, char *left_cards[]);

void clean_mem(struct stru_me *me, struct player players[], char *player_order[], char *left_cards[]);

char *do_uct(int32_t itermax, struct stru_me *me, struct player players[], char *player_order[], char *left_cards[]);

void shuffle_left_cards(char **left_cards);
void fisher_yates(char **arr, size_t len);
void distribute_left_cards(char **left_cards, struct player players[]);

void init_rootnode(struct node *, struct stru_me *me);
void clone_me(struct stru_me *ori_me, struct stru_me *cloned_me);
void clone_players(struct player ori_players[], struct player cloned_players[]);
void clone_player_order(char **player_order, char **cloned_player_order);

size_t get_untried_moves_count(char *untried_moves[], size_t len);

size_t get_child_nodes_count(struct node *arr[], size_t len);

void update_score_based_on_score_cards(struct stru_me *cloned_me, struct player players[]);
void update_node_with_result(struct node *action_node, struct stru_me *cloned_me, struct player players[]);

void do_move(char *selected_move, struct stru_me *cloned_me, struct player cloned_players[], char *cloned_player_order[]);
int rankcmp(char a, char b);
char get_played_suit(struct stru_me *cloned_me, struct player cloned_players[], char *cloned_player_order[]);
void pick_card_me(struct stru_me *cloned_me, char played_suit, char *selected_move);
void pick_card_player(struct player *cur_player, char played_suit);
void update_score_cards(char *score_cards[], char *cur_round_cards[]);

struct node *uct_select_child(const struct node *n);
struct node *node_add_child(char *selected_move, struct node *action_node, struct stru_me *cloned_me);

void init_childnode(struct node *child, char *move, struct node *parent, struct stru_me *me);

void clean_nodes_mem(struct node *rootnode);


napi_value simulation(napi_env env, napi_callback_info info);

double do_simulate(struct stru_me *me, char *left_cards[]);
void init_players(struct player players[], size_t players_len);
void init_play_order(char *order[], struct stru_me *me, struct player players[]);
void reset_play_order_on_start(char *order[], struct stru_me *me, struct player players[]);
void remove_card_from_cards(char *cards[], char *card);

#endif
