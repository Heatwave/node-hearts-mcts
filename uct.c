#include <node_api.h>
#include <stdio.h>
#include <stdlib.h>

napi_value uct();
napi_value Init(napi_env, napi_value exports);

struct stru_me {
    char *name;
    int32_t deal_score;
    int32_t cards_count;
    char *cards[64];
    char *candidate_cards[64];
};

int get_parameter_me(napi_env env, napi_value me_js_obj, struct stru_me *me);

napi_value uct(napi_env env, napi_callback_info info)
{
    napi_status status;
    int is_call_success;

    napi_value argv[5];
    size_t argc = 5;

    int32_t itermax;
    struct stru_me me;

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

    napi_value action_js;

    char *action = "7C";

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

    return 0;
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
