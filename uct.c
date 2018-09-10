#include <node_api.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <time.h>

napi_value uct();
napi_value Init(napi_env, napi_value exports);

struct stru_me {
    char *name;
    int32_t deal_score;
    int32_t cards_count;
    char *cards[64];
    char *candidate_cards[64];
};

struct player {
    char *name;
    int32_t deal_score;
    int32_t cards_count;
    char *round_card;
};

struct node {
    char *move;
    struct node *parent;
    struct node *children[32];
    uint32_t wins;
    uint32_t visits;
    char *untried_moves[64];
};

int get_parameter_me(napi_env env, napi_value me_js_obj, struct stru_me *me);
int get_parameter_players(napi_env env, napi_value players_js_array, struct player players[]);
int get_player_info(napi_env env, napi_value player_js_obj, struct player *player);
int get_parameter_player_order(napi_env env, napi_value player_order_js_array, char *player_order[]);
int get_parameter_left_cards(napi_env env, napi_value left_cards_js_array, char *left_cards[]);

char *do_uct(int32_t itermax, struct stru_me me, struct player players[], char *player_order[], char *left_cards[]);
void init_rootnode(struct node *, struct stru_me *me);

size_t get_untried_moves_count(char *arr[], size_t len);
size_t get_child_nodes_count(struct node *arr[], size_t len);

char *uct_select_child(const struct node *n);

napi_value uct(napi_env env, napi_callback_info info)
{
    napi_status status;
    int is_call_success;

    int32_t itermax;
    struct stru_me me;
    struct player players[3];
    char *player_order[4];
    char *left_cards[64];

    napi_value argv[5];
    size_t argc = 5;

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

    return 0;
}

char *do_uct(int32_t itermax, struct stru_me me, struct player players[], char *player_order[], char *left_cards[])
{
    char *result_action = "7C";
    int32_t i;

    struct node rootnode;
    init_rootnode(&rootnode, &me);

    struct node *action_node;

    struct stru_me cloned_me;
    struct player cloned_players[3];
    char *cloned_player_order[4];
    char *cloned_left_cards[64];

    size_t untried_moves_count, child_nodes_count, random_index;

    char *selected_move;

    // initialize random seed
    srand(time(NULL));

    for (i = 0; i < itermax; i++) {
        action_node = &rootnode;

        clone_me(&me, &cloned_me);
        clone_players(players, cloned_players);
        clone_player_order(player_order, cloned_player_order);
        clone_left_cards(left_cards, cloned_left_cards);

        // Select
        untried_moves_count = get_untried_moves_count(action_node->untried_moves, sizeof(action_node->untried_moves) / sizeof(action_node->untried_moves[0]));
        child_nodes_count = get_child_nodes_count(action_node->children, sizeof(action_node->children) / sizeof(action_node->children[0]));

        while (untried_moves_count == 0 && child_nodes_count != 0) {
            selected_move = uct_select_child(action_node);
            do_move(selected_move);

            untried_moves_count = get_untried_moves_count(action_node->untried_moves, sizeof(action_node->untried_moves) / sizeof(action_node->untried_moves[0]));
            child_nodes_count = get_child_nodes_count(action_node->children, sizeof(action_node->children) / sizeof(action_node->children[0]));
        }

        // Expand
        untried_moves_count = get_untried_moves_count(action_node->untried_moves, sizeof(action_node->untried_moves) / sizeof(action_node->untried_moves[0]));

        if (untried_moves_count > 0) {
            random_index = rand() % untried_moves_count;
            selected_move = action_node->untried_moves[random_index];
            do_move(selected_move);
            action_node = node_add_child(selected_move, action_node);
        }

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
