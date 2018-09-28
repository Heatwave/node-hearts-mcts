#include <node_api.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#include "mcts.h"

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

    action = do_uct(itermax, &me, players, player_order, left_cards);

    status = napi_create_string_utf8(env, action, NAPI_AUTO_LENGTH, &action_js);
    if (status != napi_ok) return NULL;

    clean_mem(&me, players, player_order, left_cards);

    free(action);

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

    for ( ; i < MAX_HAND_CARDS_LEN; ++i) {
        me->candidate_cards[i] = NULL;
    }

    napi_value score_cards_js;
    status = napi_get_named_property(env, me_js_obj, "score_cards", &score_cards_js);
    if (status != napi_ok) return 1;

    uint32_t score_cards_len;
    status = napi_get_array_length(env, score_cards_js, &score_cards_len);
    if (status != napi_ok) return 1;

    for (i = 0; i < score_cards_len; ++i) {
        status = napi_get_element(env, score_cards_js, i, &card_str);
        if (status != napi_ok) return 1;

        status = napi_get_value_string_utf8(env, card_str, NULL, 0, &card_str_len);
        if (status != napi_ok) return 1;

        me->score_cards[i] = malloc(card_str_len + 1);
        status = napi_get_value_string_utf8(env, card_str, me->score_cards[i], card_str_len+1, 0);
        if (status != napi_ok) return 1;
    }

    for ( ; i < MAX_HAND_CARDS_LEN; ++i)
        me->score_cards[i] = NULL;

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


    napi_value card_str;
    napi_value score_cards_js;
    size_t card_str_len;
    status = napi_get_named_property(env, player_js_obj, "score_cards", &score_cards_js);
    if (status != napi_ok) return 1;

    uint32_t score_cards_len;
    status = napi_get_array_length(env, score_cards_js, &score_cards_len);
    if (status != napi_ok) return 1;

    for (i = 0; i < score_cards_len; ++i) {
        status = napi_get_element(env, score_cards_js, i, &card_str);
        if (status != napi_ok) return 1;

        status = napi_get_value_string_utf8(env, card_str, NULL, 0, &card_str_len);
        if (status != napi_ok) return 1;

        player->score_cards[i] = malloc(card_str_len + 1);
        status = napi_get_value_string_utf8(env, card_str, player->score_cards[i], card_str_len+1, 0);
        if (status != napi_ok) return 1;
    }

    for ( ; i < MAX_HAND_CARDS_LEN; ++i)
        player->score_cards[i] = NULL;

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

void clean_mem(struct stru_me *me, struct player players[], char *player_order[], char *left_cards[])
{
    free(me->name);

    if (me->round_card != NULL)
        free(me->round_card);

    size_t i, j;
    for (i = 0; i < MAX_CARDS_LEN; ++i)
        if (me->cards[i] != NULL)
            free(me->cards[i]);

    for (i = 0; i < MAX_HAND_CARDS_LEN; ++i)
        if (me->candidate_cards[i] != NULL)
            free(me->candidate_cards[i]);

    for (i = 0; i < MAX_HAND_CARDS_LEN; ++i)
        if (me->score_cards[i] != NULL)
            free(me->score_cards[i]);

    for (i = 0; i < 3; ++i) {
        if (players[i].name != NULL)
            free(players[i].name);

        if (players[i].round_card != NULL)
            free(players[i].round_card);

        for (j = 0; j < MAX_CARDS_LEN; ++j)
            if (players[i].cards[j] != NULL)
                free(players[i].cards[j]);

        for (j = 0; j < MAX_HAND_CARDS_LEN; ++j)
            if (players[i].score_cards[j] != NULL)
                free(players[i].score_cards[j]);
    }

    for (i = 0; i < 3; ++i)
        if (player_order[i] != NULL)
            free(player_order[i]);

    for (i = 0; i < MAX_CARDS_LEN; ++i)
        if (left_cards[i] != NULL)
            free(left_cards[i]);
}

napi_value simulation(napi_env env, napi_callback_info info)
{
    napi_status status;
    int is_call_success;

    struct stru_me *me = malloc(sizeof(struct stru_me));
    me->name = "me";
    me->deal_score = 0;
    me->cards_count = 13;
    me->round_card = NULL;

    char *left_cards[MAX_CARDS_LEN];

    napi_value argv[2];
    size_t argc = 2;

    // initialize random seed
    srand(time(NULL));

    napi_get_cb_info(env, info, &argc, argv, NULL, NULL);
    if (argc != 2)
    {
        napi_throw_error(env, "EINVAL", "arguments number shoule be 2");
        return NULL;
    }

    napi_value cards_js_arr = argv[0];
    napi_value left_cards_js_arr = argv[1];


    uint32_t i, cards_len;
    status = napi_get_array_length(env, cards_js_arr, &cards_len);
    if (status != napi_ok) return NULL;

    napi_value card_str;
    size_t card_str_len;
    for (i = 0; i < cards_len; i++) {
        status = napi_get_element(env, cards_js_arr, i, &card_str);
        if (status != napi_ok) return NULL;

        status = napi_get_value_string_utf8(env, card_str, NULL, 0, &card_str_len);
        if (status != napi_ok) return NULL;

        me->cards[i] = malloc(card_str_len + 1);
        status = napi_get_value_string_utf8(env, card_str, me->cards[i], card_str_len+1, 0);
        if (status != napi_ok) return NULL;
    }

    for ( ; i < MAX_CARDS_LEN; ++i) {
        me->cards[i] = NULL;
    }

    for (i = 0; i < MAX_HAND_CARDS_LEN; ++i) {
        me->candidate_cards[i] = NULL;
    }

    for (i = 0; i < MAX_HAND_CARDS_LEN; ++i) {
        me->score_cards[i] = NULL;
    }

    is_call_success = get_parameter_left_cards(env, left_cards_js_arr, left_cards);


    double shooting_rate = do_simulate(me, left_cards);
    napi_value shooting_rate_js;

    status = napi_create_double(env, shooting_rate, &shooting_rate_js);
    if (status != napi_ok) return NULL;

    return shooting_rate_js;
}

napi_value Init(napi_env env, napi_value exports)
{
    napi_status status;
    napi_value func;

    status = napi_create_function(env, NULL, 0, uct, NULL, &func);
    if (status != napi_ok) return NULL;

    status = napi_set_named_property(env, exports, "uct", func);
    if (status != napi_ok) return NULL;

    status = napi_create_function(env, NULL, 0, simulation, NULL, &func);
    if (status != napi_ok) return NULL;

    status = napi_set_named_property(env, exports, "simulation", func);
    if (status != napi_ok) return NULL;

    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init);
