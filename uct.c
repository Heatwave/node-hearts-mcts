#include <node_api.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <time.h>

#define MAX_CARDS_LEN 64
#define MAX_CHILDREN_LEN 32

napi_value uct();
napi_value Init(napi_env, napi_value exports);

struct stru_me {
    char *name;
    int32_t deal_score;
    int32_t cards_count;
    char *round_card;
    char *cards[MAX_CARDS_LEN];
    char *candidate_cards[MAX_CARDS_LEN];
};

struct player {
    char *name;
    int32_t deal_score;
    int32_t cards_count;
    char *round_card;
    char *cards[MAX_CARDS_LEN];
};

struct node {
    char *move;
    struct node *parent;
    struct node *children[MAX_CHILDREN_LEN];
    uint32_t wins;
    uint32_t visits;
    char *untried_moves[MAX_CARDS_LEN];
};

int get_parameter_me(napi_env env, napi_value me_js_obj, struct stru_me *me);
int get_parameter_players(napi_env env, napi_value players_js_array, struct player players[]);
int get_player_info(napi_env env, napi_value player_js_obj, struct player *player);
int get_parameter_player_order(napi_env env, napi_value player_order_js_array, char *player_order[]);
int get_parameter_left_cards(napi_env env, napi_value left_cards_js_array, char *left_cards[]);

void shuffle_left_cards(char **left_cards);
void fisher_yates(char **arr, size_t len);
void distribute_left_cards(char **left_cards, struct player players[]);

char *do_uct(int32_t itermax, struct stru_me me, struct player players[], char *player_order[], char *left_cards[]);
void init_rootnode(struct node *, struct stru_me *me);
void clone_me(struct stru_me *ori_me, struct stru_me *cloned_me);
void clone_players(struct player ori_players[], struct player cloned_players[]);
void clone_player_order(char **player_order, char **cloned_player_order);

size_t get_untried_moves_count(char *arr[], size_t len);
size_t get_child_nodes_count(struct node *arr[], size_t len);

void do_move(char *selected_move, struct stru_me *cloned_me, struct player cloned_players[], char *cloned_player_order[]);
char get_played_suit(struct stru_me *cloned_me, struct player cloned_players[], char *cloned_player_order[]);
void pick_card_me(struct stru_me *cloned_me, char played_suit, char *selected_move);
void pick_card_player(struct player *cur_player, char played_suit);

char *uct_select_child(const struct node *n);
struct node node_add_child(char *selected_move, struct node *action_node, struct stru_me *cloned_me);

void init_childnode(struct node *child, char *move, struct node *parent, struct stru_me *me);


// entry function
napi_value uct(napi_env env, napi_callback_info info)
{
    napi_status status;
    int is_call_success;

    int32_t itermax;
    struct stru_me me;
    struct player players[3];
    char *player_order[4];
    char *left_cards[MAX_CARDS_LEN];

    napi_value argv[5];
    size_t argc = 5;

    // initialize random seed
    srand(time(NULL));

    napi_get_cb_info(env, info, &argc, argv, NULL, NULL);
    if (argc != 5)
    {
        napi_throw_error(env, "EINVAL", "arguments number shoule be 5");
        return NULL;
    }

    status = napi_get_value_int32(env, argv[0], &itermax);
    if (status != napi_ok) return NULL;

    is_call_success = get_parameter_me(env, argv[1], &me);
    if (is_call_success != 0) return NULL;

    is_call_success = get_parameter_players(env, argv[2], players);
    if (is_call_success != 0) return NULL;

    is_call_success = get_parameter_player_order(env, argv[3], player_order);
    if (is_call_success != 0) return NULL;

    is_call_success = get_parameter_left_cards(env, argv[4], left_cards);
    if (is_call_success != 0) return NULL;

    napi_value action_js;

    char *action;

    action = do_uct(itermax, me, players, player_order, left_cards);

    status = napi_create_string_utf8(env, action, NAPI_AUTO_LENGTH, &action_js);
    return action_js;
}

int get_parameter_me(napi_env env, napi_value me_js_obj, struct stru_me *me)
{
    napi_status status;

    napi_value name_js;
    status = napi_get_named_property(env, me_js_obj, "player_name", &name_js);
    if (status != napi_ok) return 1;

    size_t name_len;
    status = napi_get_value_string_utf8(env, name_js, NULL, 0, &name_len);
    if (status != napi_ok) return 1;

    me->name = malloc(name_len + 1);
    status = napi_get_value_string_utf8(env, name_js, me->name, name_len+1, 0);
    if (status != napi_ok) return 1;


    napi_value deal_score_js;
    status = napi_get_named_property(env, me_js_obj, "deal_score", &deal_score_js);
    if (status != napi_ok) return 1;

    status = napi_get_value_int32(env, deal_score_js, &me->deal_score);
    if (status != napi_ok) return 1;


    napi_value cards_count_js;
    status = napi_get_named_property(env, me_js_obj, "cards_count", &cards_count_js);
    if (status != napi_ok) return 1;

    status = napi_get_value_int32(env, cards_count_js, &me->cards_count);
    if (status != napi_ok) return 1;

    me->round_card = NULL;

    napi_value cards_js;
    status = napi_get_named_property(env, me_js_obj, "cards", &cards_js);
    if (status != napi_ok) return 1;

    uint32_t i, cards_len;
    status = napi_get_array_length(env, cards_js, &cards_len);
    if (status != napi_ok) return 1;

    napi_value card_str;
    size_t card_str_len;
    for (i = 0; i < cards_len; i++) {
        status = napi_get_element(env, cards_js, i, &card_str);
        if (status != napi_ok) return 1;

        status = napi_get_value_string_utf8(env, card_str, NULL, 0, &card_str_len);
        if (status != napi_ok) return 1;
        me->cards[i] = malloc(card_str_len + 1);
        status = napi_get_value_string_utf8(env, card_str, me->cards[i], card_str_len+1, 0);
        if (status != napi_ok) return 1;
    }

    for ( ; i < sizeof(me->cards) / sizeof(me->cards[0]); i++) {
        me->cards[i] = NULL;
    }

    napi_value candidate_cards_js;
    status = napi_get_named_property(env, me_js_obj, "candidate_cards", &candidate_cards_js);
    if (status != napi_ok) return 1;

    uint32_t candidate_cards_len;
    status = napi_get_array_length(env, candidate_cards_js, &candidate_cards_len);
    if (status != napi_ok) return 1;

    for (i = 0; i < candidate_cards_len; i++) {
        status = napi_get_element(env, candidate_cards_js, i, &card_str);
        if (status != napi_ok) return 1;

        status = napi_get_value_string_utf8(env, card_str, NULL, 0, &card_str_len);
        if (status != napi_ok) return 1;
        me->candidate_cards[i] = malloc(card_str_len + 1);
        status = napi_get_value_string_utf8(env, card_str, me->candidate_cards[i], card_str_len+1, 0);
        if (status != napi_ok) return 1;
    }

    for ( ; i < sizeof(me->candidate_cards) / sizeof(me->candidate_cards[0]); i++) {
        me->candidate_cards[i] = NULL;
    }

    return 0;
}

int get_parameter_players(napi_env env, napi_value players_js_array, struct player players[])
{
    napi_status status;
    uint32_t i, players_len;

    status = napi_get_array_length(env, players_js_array, &players_len);
    if (status != napi_ok) return 1;


    int is_get_player_success;
    napi_value player_js_obj;
    for (i = 0; i < players_len; i++) {
        status = napi_get_element(env, players_js_array, i, &player_js_obj);
        if (status != napi_ok) return 1;
        is_get_player_success = get_player_info(env, player_js_obj, &players[i]);
        if (is_get_player_success != 0) return 1;
    }

    return 0;
}

int get_player_info(napi_env env, napi_value player_js_obj, struct player *player)
{
    napi_status status;

    napi_value name_js;
    status = napi_get_named_property(env, player_js_obj, "player_name", &name_js);
    if (status != napi_ok) return 1;

    size_t name_len;
    status = napi_get_value_string_utf8(env, name_js, NULL, 0, &name_len);
    if (status != napi_ok) return 1;

    player->name = malloc(name_len + 1);
    status = napi_get_value_string_utf8(env, name_js, player->name, name_len+1, 0);
    if (status != napi_ok) return 1;


    napi_value deal_score_js;
    status = napi_get_named_property(env, player_js_obj, "deal_score", &deal_score_js);
    if (status != napi_ok) return 1;

    status = napi_get_value_int32(env, deal_score_js, &player->deal_score);
    if (status != napi_ok) return 1;


    napi_value cards_count_js;
    status = napi_get_named_property(env, player_js_obj, "cards_count", &cards_count_js);
    if (status != napi_ok) return 1;

    status = napi_get_value_int32(env, cards_count_js, &player->cards_count);
    if (status != napi_ok) return 1;


    napi_value round_card_js;
    status = napi_get_named_property(env, player_js_obj, "round_card", &round_card_js);
    if (status != napi_ok) return 1;

    size_t round_card_len;
    status = napi_get_value_string_utf8(env, round_card_js, NULL, 0, &round_card_len);
    if (status != napi_ok) return 1;

    player->round_card = malloc(round_card_len + 1);
    status = napi_get_value_string_utf8(env, round_card_js, player->round_card, round_card_len+1, 0);
    if (status != napi_ok) return 1;

    size_t i;
    for (i = 0; i< MAX_CARDS_LEN; i++) {
        player->cards[i] = NULL;
    }


    return 0;
}

int get_parameter_player_order(napi_env env, napi_value player_order_js_array, char *player_order[])
{
    napi_status status;
    uint32_t i, player_order_len;

    status = napi_get_array_length(env, player_order_js_array, &player_order_len);
    if (status != napi_ok) return 1;

    napi_value player_name_js;
    size_t player_name_len;
    for (i = 0; i < player_order_len; i++) {
        status = napi_get_element(env, player_order_js_array, i, &player_name_js);
        if (status != napi_ok) return 1;
        status = napi_get_value_string_utf8(env, player_name_js, NULL, 0, &player_name_len);
        if (status != napi_ok) return 1;
        player_order[i] = malloc(player_name_len+1);
        status = napi_get_value_string_utf8(env, player_name_js, player_order[i],
                    player_name_len+1, 0);
        if (status != napi_ok) return 1;
    }

    return 0;
}

int get_parameter_left_cards(napi_env env, napi_value left_cards_js_array, char *left_cards[])
{
    napi_status status;
    uint32_t i, left_cards_len;

    status = napi_get_array_length(env, left_cards_js_array, &left_cards_len);
    if (status != napi_ok) return 1;

    napi_value left_card_js;
    size_t card_len;
    for (i = 0; i < left_cards_len; i++) {
        status = napi_get_element(env, left_cards_js_array, i, &left_card_js);
        if (status != napi_ok) return 1;
        status = napi_get_value_string_utf8(env, left_card_js, NULL, 0, &card_len);
        if (status != napi_ok) return 1;
        left_cards[i] = malloc(card_len+1);
        status = napi_get_value_string_utf8(env, left_card_js, left_cards[i],
                    card_len+1, 0);
        if (status != napi_ok) return 1;
    }

    for ( ; i < MAX_CARDS_LEN; i++) {
        left_cards[i] = NULL;
    }

    return 0;
}

void shuffle_left_cards(char **left_cards)
{
    size_t count = 0;
    char **pp = left_cards;
    while (*pp++ != NULL)
        ;
    count = pp - left_cards - 1;
    assert(count > 1);

    fisher_yates(left_cards, count);
}

void fisher_yates(char **arr, size_t len)
{
    size_t i = len, j;
    char *temp;

    assert(i > 1);
    while (--i) {
        j = rand() % (i+1);
        temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

void distribute_left_cards(char **left_cards, struct player players[])
{
    char **pp = left_cards;
    size_t i, j;
    int32_t cards_count;
    for (i = 0; i < 3; i++) {
        struct player *pl = &players[i];
        cards_count = pl->cards_count;
        for (j = 0; j < (size_t)cards_count; j++) {
            pl->cards[j] = strdup(*pp++);
        }
    }
    assert(*pp == NULL);

    // TODO: remove following test code
    for (i = 0; i < 3; i++) {
        struct player *pl = &players[i];
        cards_count = pl->cards_count;
        printf("name: %s\tcards: ", pl->name);
        for (j = 0; j < (size_t)cards_count; j++) {
            printf("%s ", pl->cards[j]);
        }
        printf("\n");
    }
}

char *do_uct(int32_t itermax, struct stru_me me, struct player players[], char *player_order[], char* left_cards[])
{
    char *result_action = "7C";
    int32_t i;

    struct node rootnode;
    init_rootnode(&rootnode, &me);

    struct node *action_node;

    struct stru_me cloned_me;
    struct player cloned_players[3];
    char *cloned_player_order[4];

    size_t untried_moves_count, child_nodes_count, random_index;

    char *selected_move;

    for (i = 0; i < itermax; i++) {
        action_node = &rootnode;

        clone_me(&me, &cloned_me);
        clone_players(players, cloned_players);
        clone_player_order(player_order, cloned_player_order);

        shuffle_left_cards(left_cards);
        distribute_left_cards(left_cards, cloned_players);

        // Select
        untried_moves_count = 0;
        size_t j;

        for (j = 0; j < MAX_CARDS_LEN; ++j) {
            if (action_node->untried_moves[j] != NULL)
                ++untried_moves_count;
        }
        //untried_moves_count = get_untried_moves_count(action_node->untried_moves, sizeof(action_node->untried_moves) / sizeof(action_node->untried_moves[0]));

        child_nodes_count = 0;
        for (j = 0; j < MAX_CHILDREN_LEN; ++j) {
            if (action_node->children[j] != NULL)
                ++child_nodes_count;
        }
        //child_nodes_count = get_child_nodes_count(action_node->children, sizeof(action_node->children) / sizeof(action_node->children[0]));

        while (untried_moves_count == 0 && child_nodes_count != 0) {
            selected_move = uct_select_child(action_node);
            do_move(selected_move, &cloned_me, cloned_players, cloned_player_order);

            untried_moves_count = 0;
            for (j = 0; j < MAX_CARDS_LEN; ++j) {
                if (action_node->untried_moves[j] != NULL)
                    ++untried_moves_count;
            }
            child_nodes_count = 0;
            for (j = 0; j < MAX_CHILDREN_LEN; ++j) {
                if (action_node->children[j] != NULL)
                    ++child_nodes_count;
            }
            //untried_moves_count = get_untried_moves_count(action_node->untried_moves, sizeof(action_node->untried_moves) / sizeof(action_node->untried_moves[0]));
            //child_nodes_count = get_child_nodes_count(action_node->children, sizeof(action_node->children) / sizeof(action_node->children[0]));
        }

        // Expand
        //untried_moves_count = get_untried_moves_count(action_node->untried_moves, sizeof(action_node->untried_moves) / sizeof(action_node->untried_moves[0]));
        untried_moves_count = 0;
        for (j = 0; j < MAX_CARDS_LEN; ++j) {
            if (action_node->untried_moves[j] != NULL)
                ++untried_moves_count;
        }

        if (untried_moves_count > 0) {
            random_index = rand() % untried_moves_count;
            selected_move = action_node->untried_moves[random_index];
            do_move(selected_move, &cloned_me, cloned_players, cloned_player_order);
            struct node new_child_node = node_add_child(selected_move, action_node, &cloned_me);
            action_node = &new_child_node;
            printf("action_node->move: %s\n", action_node->move);
        }

        untried_moves_count = 0;
        for (j = 0; j < MAX_CARDS_LEN; ++j) {
            if (action_node->untried_moves[j] != NULL)
                ++untried_moves_count;
        }
        printf("untried_moves_count: %zu\n", untried_moves_count);
        // Rollout

        // Backpropagate
    }

    return result_action;
}

void init_rootnode(struct node *rootnode, struct stru_me *me)
{
    rootnode->move = NULL;
    rootnode->parent = NULL;

    uint32_t i;
    for (i = 0; i < sizeof(rootnode->children) / sizeof(rootnode->children[0]); i++) {
        rootnode->children[i] = NULL;
    }

    rootnode->wins = 0;
    rootnode->visits = 0;

    char **pp = rootnode->untried_moves;
    size_t moves_count = 0;
    for (i = 0; i < sizeof(me->candidate_cards) / sizeof(me->candidate_cards[0]); i++) {
        if (me->candidate_cards[i] != NULL) {
            *pp = malloc(strlen(me->candidate_cards[i])+1);
            strcpy(*pp++, me->candidate_cards[i]);
            ++moves_count;
        }
    }

    for ( ; moves_count < sizeof(rootnode->untried_moves) / sizeof(rootnode->untried_moves[i]);
    moves_count++) {
        *pp++ = NULL;
    }
    assert((pp - rootnode->untried_moves) == sizeof(rootnode->untried_moves) / sizeof(rootnode->untried_moves[i]));
}

void clone_me(struct stru_me *ori_me, struct stru_me *cloned_me)
{
    cloned_me->name = strdup(ori_me->name);
    cloned_me->deal_score = ori_me->deal_score;
    cloned_me->cards_count = ori_me->cards_count;

    if (ori_me->round_card == NULL)
        cloned_me->round_card = NULL;
    else
        cloned_me->round_card = strdup(ori_me->round_card);

    size_t i;
    for (i = 0; i < MAX_CARDS_LEN; i++) {
        if (ori_me->cards[i] != NULL)
            cloned_me->cards[i] = strdup(ori_me->cards[i]);
        else
            cloned_me->cards[i] = NULL;
    }

    for (i = 0; i < MAX_CARDS_LEN; i++) {
        if (ori_me->candidate_cards[i] != NULL)
            cloned_me->candidate_cards[i] = strdup(ori_me->candidate_cards[i]);
        else
            cloned_me->candidate_cards[i] = NULL;
    }
}

void clone_players(struct player ori_players[], struct player cloned_players[])
{
    size_t i, j;
    for (i = 0; i < 3; i ++) {
        cloned_players[i].name = strdup(ori_players[i].name);
        cloned_players[i].deal_score = ori_players[i].deal_score;
        cloned_players[i].cards_count = ori_players[i].cards_count;
        cloned_players[i].round_card = strdup(ori_players[i].round_card);

        for (j = 0; j < MAX_CARDS_LEN; j++) {
            if (ori_players[i].cards[j] != NULL)
                cloned_players[i].cards[j] = strdup(ori_players[i].cards[j]);
            else
                cloned_players[i].cards[j] = NULL;
        }
    }
}

void clone_player_order(char **player_order, char **cloned_player_order)
{
    size_t i;
    for (i = 0; i < 4; i++) {
        *cloned_player_order++ = strdup(*player_order++);
    }
}

size_t get_untried_moves_count(char *arr[], size_t len)
{
    size_t i;
    size_t count = 0;

    for (i = 0; i < len; i++) {
        if (arr[i] != NULL)
            ++count;
    }

    return count;
}

size_t get_child_nodes_count(struct node *arr[], size_t len)
{
    size_t i;
    size_t count = 0;

    for (i = 0; i < len; i++) {
        if (arr[i] != NULL)
            ++count;
    }

    return count;
}

void do_move(char *selected_move, struct stru_me *cloned_me, struct player cloned_players[], char *cloned_player_order[])
{
    // no method to know the passed array size, so we assume the order count is 4
    size_t order_count = 4;

    char played_suit;

    played_suit = get_played_suit(cloned_me, cloned_players, cloned_player_order);

    char *order_player_name;

    char *cur_round_cards[4];
    char **pp = cur_round_cards;

    size_t i, j;
    for (i = 0; i < order_count; i++) {
        order_player_name = cloned_player_order[i];

        if (strcmp(order_player_name, cloned_me->name) == 0) {
            pick_card_me(cloned_me, played_suit, selected_move);
            *pp++ = strdup(cloned_me->round_card);
        } else {
            for (j = 0; j < 3; j++) {
                if (strcmp(order_player_name, cloned_players[j].name) == 0) {
                    pick_card_player(&cloned_players[j], played_suit);
                    *pp++ = strdup(cloned_players[j].round_card);
                }
            }
        }
    }

    assert((pp - cur_round_cards) == 4);
    // TODO: delete follwoing test code
    printf("cur_round_cards: ");
    for (i = 0; i < 4; i++) {
        printf("%s ", cur_round_cards[i]);
    }
    printf("\n");

    size_t hearts_count = 0;
    for (i = 0; i < 4; i++) {
        if (cur_round_cards[i][1] == 'H')
            ++hearts_count;
    }

    int32_t round_score = hearts_count * (-1);

    for (i = 0; i < 4; i++) {
        if (strcmp(cur_round_cards[i], "QS") == 0)
            round_score += -13;
    }

    size_t winner_index = 0;
    char max_rank = 0;

    for (i = 0; i < 4; i++) {
        if (cur_round_cards[i][1] != played_suit)
            continue;
        if (cur_round_cards[i][0] > max_rank) {
            max_rank = cur_round_cards[i][0];
            winner_index = i;
        }
    }
    //printf("winner_index: %d, max_rank: %c\n", winner_index, max_rank);

    char *winner_name = cloned_player_order[winner_index];
    if (strcmp(winner_name, cloned_me->name) == 0) {
        cloned_me->deal_score += round_score;
        //printf("me deal_score: %d\n", cloned_me->deal_score);
    } else {
        for (i = 0; i < 3; i++) {
            if (strcmp(winner_name, cloned_players[i].name) == 0) {
                cloned_players[i].deal_score += round_score;
                //printf("player deal_score: %d\n", cloned_players[i].deal_score);
                break;
            }
        }
    }

    cloned_me->round_card = "";
    for (i = 0; i < 3; i++)
        cloned_players[i].round_card = "";

    // TODO: delete following
    printf("me cards_count: %d\t", cloned_me->cards_count);
    for (i = 0; i < 3; i++)
        printf("i: %zu, cards_count: %d\t", i, cloned_players[i].cards_count);
    printf("\n");
}

char get_played_suit(struct stru_me *cloned_me, struct player cloned_players[], char *cloned_player_order[])
{
    assert(strlen(cloned_player_order[0]) > 0);

    char suit = 0;
    size_t i;

    if (strcmp(cloned_player_order[0], cloned_me->name) == 0) {
        if (cloned_me->round_card != NULL && strlen(cloned_me->round_card) > 1)
            suit = cloned_me->round_card[1];
    } else {
        for (i = 0; i < 3; i++) {
            if (strcmp(cloned_player_order[0], cloned_players[i].name) == 0) {
                if (cloned_players[i].round_card != NULL
                && strlen(cloned_players[i].round_card) > 1) {
                    suit = cloned_players[i].round_card[1];
                    break;
                }
            }
        }
    }

    // TODO: remove following test code
    printf("suit: %c\n", suit);

    return suit;
}

void pick_card_me(struct stru_me *cloned_me, char played_suit, char *selected_move)
{
    if (cloned_me->round_card != NULL && strlen(cloned_me->round_card) > 1)
        return;

    cloned_me->round_card = strdup(selected_move);

    char **pp = cloned_me->cards;
    while (strcmp(selected_move, *pp++) != 0)
        ;

    while (*(pp+1) != NULL && strlen(*(pp+1)) > 1) {
        *pp = *(pp+1);
        ++pp;
    }

    cloned_me->cards_count -= 1;
}

void pick_card_player(struct player *cur_player, char played_suit)
{
    if (cur_player->round_card != NULL && strlen(cur_player->round_card) > 1)
        return;

    char *able_to_played_cards[64];
    char **pp = able_to_played_cards;
    size_t i;
    for (i = 0; i < MAX_CARDS_LEN; i++) {
        if (cur_player->cards[i] != NULL && strlen(cur_player->cards[i]) > 1
        && cur_player->cards[i][1] == played_suit)
            *pp++ = strdup(cur_player->cards[i]);
    }

    while ((pp - able_to_played_cards) < 64) {
        *pp++ = NULL;
    }

    // TODO: delete following test code
    printf("able_to_played_cards: ");
    for (i = 0; i < 64; i++) {
        if (able_to_played_cards[i] != NULL)
            printf("i: %zu, %s ", i, able_to_played_cards[i]);
    }
    printf("\n");

    size_t able_to_played_count = 0;
    pp = able_to_played_cards;
    while (*pp != NULL) {
        ++pp;
        ++able_to_played_count;
    }

    char *selected;

    if (able_to_played_count == 0) {
        size_t cards_len = 0;
        pp = cur_player->cards;
        while (*pp != NULL && strlen(*pp) == 2) {
            ++pp;
            ++cards_len;
        }
        selected = cur_player->cards[rand() % cards_len];
    } else if (able_to_played_count == 1) {
        selected = able_to_played_cards[0];
    } else {
        selected = able_to_played_cards[rand() % able_to_played_count];
    }

    cur_player->round_card = strdup(selected);

    pp = cur_player->cards;
    while (strcmp(selected, *pp++) != 0)
        ;

    while (*(pp+1) != NULL && strlen(*(pp+1)) == 2) {
        *pp = *(pp+1);
        ++pp;
    }

    cur_player->cards_count -= 1;
}

char *uct_select_child(const struct node *n)
{
    char *selected;
    size_t i;
    double uct_value, best_value;
    for (i = 0; i < sizeof(n->children) / sizeof(n->children[0]); i++) {
        if (n->children[i] == NULL)
            continue;

        struct node *c = n->children[i];

        uct_value = c->wins / c->visits + sqrt(2 * log(n->visits) / c->visits);

        if (uct_value > best_value) {
            selected = c->move;
            best_value = uct_value;
        }
    }

    char *temp = strdup(selected);
    return temp;
}

struct node node_add_child(char *selected_move, struct node *action_node, struct stru_me *me)
{
    struct node **child = action_node->children;
    while (*child++ != NULL)
        ;
    assert((child - action_node->children) < MAX_CHILDREN_LEN);

    struct node new_child;
    init_childnode(&new_child, selected_move, action_node, me);

    *child = &new_child;

    // remove selected_move from rootnode untried_moves
    char **pp = action_node->untried_moves;
    while (strcmp(selected_move, *pp++) != 0)
        ;

    while (*(pp+1) != NULL && strlen(*(pp+1)) == 2) {
        *pp = *(pp+1);
        ++pp;
    }

    return new_child;
}

void init_childnode(struct node *child, char *move, struct node *parent, struct stru_me *me)
{
    child->move = strdup(move);
    child->parent = parent;
    child->wins = 0;
    child->visits = 0;

    size_t i;
    for (i = 0; i < MAX_CHILDREN_LEN; i++) {
        child->children[i] = NULL;
    }
}

napi_value Init(napi_env env, napi_value exports)
{
    napi_status status;
    napi_value func;

    status = napi_create_function(env, NULL, 0, uct, NULL, &func);
    if (status != napi_ok) return NULL;

    status = napi_set_named_property(env, exports, "uct", func);
    if (status != napi_ok) return NULL;

    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init);
