#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "mcts.h"

#define ITER_MAX 60000

double do_simulate(struct stru_me *me, char *left_cards[])
{
    struct player players[3];
    init_players(players, 3);

    char *play_order[4];
    init_play_order(play_order, me, players);

    size_t iter, i;
    double shooting_count = 0.0;
    struct stru_me *cloned_me = malloc(sizeof(struct stru_me));
    struct player cloned_players[3];

    for (iter = 0; iter < ITER_MAX; ++iter) {
        clone_me(me, cloned_me);
        clone_players(players, cloned_players);

        shuffle_left_cards(left_cards);
        distribute_left_cards(left_cards, cloned_players);
        reset_play_order_on_start(play_order, cloned_me, cloned_players);

        char *first_player_name = play_order[0];

        if (strcmp(first_player_name, cloned_me->name) == 0) {
            cloned_me->round_card = strdup("2C");
            cloned_me->cards_count = 12;
            remove_card_from_cards(cloned_me->cards, "2C");
        } else {
            for (i = 0; i < 3; ++i) {
                struct player *p = &cloned_players[i];
                if (strcmp(first_player_name, p->name) == 0) {
                    p->round_card = strdup("2C");
                    p->cards_count = 12;
                    remove_card_from_cards(p->cards, "2C");
                }
            }
        }

        while (cloned_me->cards_count > 0) {
            do_move(NULL, cloned_me, cloned_players, play_order);
        }

        update_score_based_on_score_cards(cloned_me, cloned_players);

        if (cloned_me->deal_score > 0)
            shooting_count += 1.0;

        clean_cloned_me(cloned_me);
        clean_cloned_players(cloned_players);
    }

    free(cloned_me);
    for (i = 0; i < 4; ++i)
        if (play_order[i] != NULL)
            free(play_order[i]);

    // printf("shooting_count: %f\n", shooting_count);

    return shooting_count / (double)(ITER_MAX);
}

void init_players(struct player players[], size_t players_len)
{
    size_t i, j;
    char pname[10] = { 'p', '\0' };

    for (i = 0; i < players_len; ++i) {
        struct player *p = &players[i];
        pname[1] = (char)(i + 1 + '0');
        pname[2] = '\0';
        p->name = strdup(pname);
        p->deal_score = 0;
        p->cards_count = 13;
        p->round_card = "";
        for (j = 0; j < MAX_HAND_CARDS_LEN; ++j)
            p->cards[j] = NULL;
        for (j = 0; j < MAX_HAND_CARDS_LEN; ++j)
            p->score_cards[j] = NULL;
    }
}

void init_play_order(char *order[], struct stru_me *me, struct player players[])
{
    size_t i;
    struct player *p = players;
    for (i = 0; i < 4; ++i) {
        if (i == 0)
            order[i] = strdup(me->name);
        else
            order[i] = strdup((p++)->name);
    }
}

void reset_play_order_on_start(char *order[], struct stru_me *me, struct player players[])
{
    size_t i, j;
    char *first_player_name;
    struct player *p = players;


    for (i = 0; i < 4; ++i) {
        if (i == 0) {
            for (j = 0; j < MAX_HAND_CARDS_LEN; ++j) {
                if (me->cards[j] != NULL && strcmp(me->cards[j], "2C") == 0) {
                    first_player_name = me->name;
                    break;
                }
            }
        } else {
            for (j = 0; j < MAX_HAND_CARDS_LEN; ++j) {
                if (p->cards[j] != NULL && strcmp(p->cards[j], "2C") == 0) {
                    first_player_name = p->name;
                    break;
                }
            }
            ++p;
        }
    }

    char *last_order_name;
    while (strcmp(order[0], first_player_name) != 0) {
        last_order_name = strdup(order[3]);
        free(order[3]);
        for (i = 3; i > 0; --i) {
            order[i] = order[i-1];
        }
        order[0] = strdup(last_order_name);

        free(last_order_name);
    }
}

void remove_card_from_cards(char *cards[], char *card)
{
    size_t i, j;

    for (i = 0; i < MAX_HAND_CARDS_LEN; ++i) {
        if (cards[i] != NULL && strcmp(cards[i], card) == 0) {
            for (j = i; j + 1 < MAX_HAND_CARDS_LEN; ++j) {
                cards[j] = cards[j+1];
            }
            break;
        }
    }
}
