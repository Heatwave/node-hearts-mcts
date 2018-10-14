#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "mcts.h"

#define ITER_MAX 50000

static int show_detail = 0;

double do_simulation(struct stru_me *me, char *left_cards[])
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

        update_score_based_on_score_cards(cloned_me, cloned_players, 0);

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

int32_t do_simulate(int32_t itermax, struct stru_me *me, struct player players[], char *player_order[], int is_AH_exposed)
{
    int32_t i;
    size_t j;
    int32_t total_score = 0;

    struct stru_me *cloned_me = malloc(sizeof(struct stru_me));
    struct player cloned_players[3];
    char *cloned_player_order[4];

    for (i = 0; i < itermax; ++i) {
        copy_me(me, cloned_me);
        copy_players(players, cloned_players);
        for (j = 0; j < 4; ++j)
            cloned_player_order[j] = strdup(player_order[j]);

        shuffle_left_cards(me->left_cards);
        distribute_left_cards(me->left_cards, cloned_players);

        while (cloned_me->cards_count > 0) {
            play_game(cloned_me, cloned_players, cloned_player_order);
        }

        update_deal_score_based_on_score_cards(cloned_me, cloned_players, is_AH_exposed);

        if (show_detail)
            printf("me score: %d\n", cloned_me->deal_score);
        total_score += cloned_me->deal_score;
        
        free_cloned_mem(cloned_me, cloned_players, cloned_player_order);
    }

    return total_score;
}

void copy_me(struct stru_me *me, struct stru_me *cloned_me)
{
    cloned_me->name = strdup(me->name);
    cloned_me->deal_score = me->deal_score;
    cloned_me->cards_count = me->cards_count;
    
    if (me->round_card != NULL)
        cloned_me->round_card = strdup(me->round_card);
    else
        cloned_me->round_card = NULL;

    size_t i;
    for (i = 0; i < MAX_HAND_CARDS_LEN; ++i) {
        if (me->cards[i] != NULL)
            cloned_me->cards[i] = strdup(me->cards[i]);
        else
            cloned_me->cards[i] = NULL;

        if (me->candidate_cards[i] != NULL)
            cloned_me->candidate_cards[i] = strdup(me->candidate_cards[i]);
        else
            cloned_me->candidate_cards[i] = NULL;

        if (me->score_cards[i] != NULL)
            cloned_me->score_cards[i] = strdup(me->score_cards[i]);
        else
            cloned_me->score_cards[i] = NULL;
    }
}

void copy_players(struct player players[], struct player cloned_players[])
{
    size_t i, j;
    for (i = 0; i < 3; ++i) {
        cloned_players[i].name = strdup(players[i].name);
        cloned_players[i].number = (int)i;
        cloned_players[i].deal_score = players[i].deal_score;
        cloned_players[i].cards_count = players[i].cards_count;
        if (players[i].round_card != NULL)
            cloned_players[i].round_card = strdup(players[i].round_card);
        else
            cloned_players[i].round_card = NULL;
        for (j = 0; j < MAX_HAND_CARDS_LEN; ++j) {
            if (players[i].cards[j] != NULL)
                cloned_players[i].cards[j] = strdup(players[i].cards[j]);
            else
                cloned_players[i].cards[j] = NULL;

            if (players[i].score_cards[j] != NULL)
                cloned_players[i].score_cards[j] = strdup(players[i].score_cards[j]);
            else
                cloned_players[i].score_cards[j] = NULL;

            cloned_players[i].candidate_cards[j] = NULL;
        }
    }
}

void free_cloned_mem(struct stru_me *cloned_me, struct player cloned_players[], char *cloned_player_order[])
{
    if (cloned_me->name != NULL)
        free(cloned_me->name);
    if (cloned_me->round_card != NULL)
        free(cloned_me->round_card);

    size_t i, j;
    for (i = 0; i < MAX_HAND_CARDS_LEN; ++i) {

        if (cloned_me->cards[i] != NULL)
            free(cloned_me->cards[i]);
        if (cloned_me->candidate_cards[i] != NULL)
            free(cloned_me->candidate_cards[i]);
        if (cloned_me->score_cards[i] != NULL)
            free(cloned_me->score_cards[i]);
    }

    for (i = 0; i < 3; ++i) {
        if (cloned_players[i].name != NULL)
            free(cloned_players[i].name);
        if (cloned_players[i].round_card != NULL)
            free(cloned_players[i].round_card);
        for (j = 0; j < MAX_HAND_CARDS_LEN; ++j) {
            if (cloned_players[i].cards[j] != NULL)
                free(cloned_players[i].cards[j]);
            if (cloned_players[i].score_cards[j] != NULL)
                free(cloned_players[i].score_cards[j]);
        }
    }

    for (i = 0; i < 4; ++i)
        free(cloned_player_order[i]);
}

void play_game(struct stru_me *cloned_me, struct player cloned_players[], char *cloned_player_order[])
{
    size_t i, j;

    char played_suit = -1;
    char *first_player_name = cloned_player_order[0];
    if (strcmp(first_player_name, cloned_me->name) == 0 && cloned_me->round_card != NULL)
        played_suit = cloned_me->round_card[1];
    for (i = 0; i < 3; ++i) {
        if (strcmp(first_player_name, cloned_players[i].name) == 0 && cloned_players[i].round_card != NULL) {
            played_suit = cloned_players[i].round_card[1];
            break;
        }
    }

    int is_heart_broken = 0;
    if (cloned_me->round_card != NULL && cloned_me->round_card[1] == 'H')
        is_heart_broken = 1;
    for (i = 0; i < MAX_HAND_CARDS_LEN; ++i)
        if (is_heart_broken == 0 && cloned_me->score_cards[i] != NULL && cloned_me->score_cards[i][1] == 'H')
            is_heart_broken = 1;
    for (i = 0; i < 3; ++i) {
        if (is_heart_broken == 0 && cloned_players[i].round_card != NULL && cloned_players[i].round_card[1] == 'H')
            is_heart_broken = 1;
        for (j = 0; j < MAX_HAND_CARDS_LEN; ++j)
            if (is_heart_broken == 0 && cloned_players[i].score_cards[j] != NULL && cloned_players[i].score_cards[j][1] == 'H')
                is_heart_broken = 1;
    }

    char *current_round_cards[4];
    char **pp = current_round_cards;

    char *played_card = NULL;

    for (i = 0; i < 4; ++i) {
        char *current_player_name = cloned_player_order[i];

        if (strcmp(current_player_name, cloned_me->name) == 0) {
            if (cloned_me->round_card != NULL && strlen(cloned_me->round_card) == 2) {
                played_card = cloned_me->round_card;
            } else {
                update_me_candidate_cards(cloned_me, played_suit, is_heart_broken);
                assert(cloned_me->candidate_cards[0] != NULL);
                played_card = choose_played_card_me(cloned_me, current_round_cards, i);
            }
            assert(played_card != NULL && strlen(played_card) == 2);
            *pp++ = strdup(played_card);
            play_card_me(cloned_me, played_card);
        } else {
            for (j = 0; j < 3; ++j) {
                if (strcmp(current_player_name, cloned_players[j].name) == 0) {
                    if (cloned_players[j].round_card != NULL && strlen(cloned_players[j].round_card) == 2) {
                        played_card = cloned_players[j].round_card;
                        *pp++ = strdup(played_card);
                        cloned_players[j].round_card = NULL;
                        break;
                    } else {
                        update_player_candidate_cards(&cloned_players[j], played_suit, is_heart_broken);
                        assert(cloned_players[j].candidate_cards[0] != NULL);
                        played_card = choose_played_card_player(&cloned_players[j], current_round_cards, i);
                    }
                    assert(played_card != NULL && strlen(played_card) == 2);
                    *pp++ = strdup(played_card);
                    play_card_player(&cloned_players[j], played_card);
                    break;
                }
            }
        }

        assert((pp - current_round_cards) - 1 == (long)i);

        if (!is_heart_broken && played_card[1] == 'H')
            is_heart_broken = 1;

        if (i == 0)
            played_suit = played_card[1];
    }

    int winner_order_index = 0;
    char *winned_card = current_round_cards[winner_order_index];
    for (i = 1; i < 4; ++i) {
        if (current_round_cards[i][1] != played_suit)
            continue;
        if (rankcmp(current_round_cards[i][0], winned_card[0]) > 0) {
            winner_order_index = i;
            winned_card = current_round_cards[i];
        }
    }

    char *winner_name = cloned_player_order[winner_order_index];

    if (show_detail)
        printf("winned_card: %s, winner_order_index: %d, winner_name: %s\n", winned_card, winner_order_index, winner_name);

    if (strcmp(winner_name, cloned_me->name) == 0) {
        insert_score_cards(cloned_me->score_cards, current_round_cards);
    } else {
        for (i = 0; i < MAX_HAND_CARDS_LEN; ++i) {
            if (strcmp(winner_name, cloned_players[i].name) == 0) {
                insert_score_cards(cloned_players[i].score_cards, current_round_cards);
                break;
            }
        }
    }

    // adjust_cloned_player_order
    char *last_order_name = NULL;
    while (strcmp(cloned_player_order[0], winner_name) != 0) {
        last_order_name = strdup(cloned_player_order[3]);
        free(cloned_player_order[3]);
        for (i = 3; i > 0; --i) {
            cloned_player_order[i] = cloned_player_order[i-1];
        }
        cloned_player_order[0] = last_order_name;
    }

    for (i = 0; i < 4; ++i)
        if (current_round_cards[i] != NULL)
            free(current_round_cards[i]);
}

void update_me_candidate_cards(struct stru_me *cloned_me, char played_suit, int is_heart_broken)
{
    size_t i;
    for (i = 0; i < MAX_HAND_CARDS_LEN; ++i) {
        if (cloned_me->candidate_cards[i] != NULL)
            free(cloned_me->candidate_cards[i]);
        cloned_me->candidate_cards[i] = NULL;
    }

    if (played_suit == -1) {
        if (is_heart_broken) {
            for (i = 0; i < MAX_HAND_CARDS_LEN; ++i) {
                if (cloned_me->cards[i] != NULL)
                    cloned_me->candidate_cards[i] = strdup(cloned_me->cards[i]);
            }
        } else {
            char **pp = cloned_me->candidate_cards;
            for (i = 0; i < MAX_HAND_CARDS_LEN; ++i) {
                if (cloned_me->cards[i] != NULL && cloned_me->cards[i][1] != 'H')
                    *pp++ = strdup(cloned_me->cards[i]);
            }
            if (pp - cloned_me->candidate_cards == 0) {
                for (i = 0; i < MAX_HAND_CARDS_LEN; ++i)
                    if (cloned_me->cards[i] != NULL)
                        cloned_me->candidate_cards[i] = strdup(cloned_me->cards[i]);
            }
        }
    } else {
        char **pp = cloned_me->candidate_cards;
        for (i = 0; i < MAX_HAND_CARDS_LEN; ++i) {
            if (cloned_me->cards[i] != NULL && cloned_me->cards[i][1] == played_suit)
                *pp++= strdup(cloned_me->cards[i]);
        }
        if (pp - cloned_me->candidate_cards == 0) {
            for (i = 0; i < MAX_HAND_CARDS_LEN; ++i)
                if (cloned_me->cards[i] != NULL)
                    cloned_me->candidate_cards[i] = strdup(cloned_me->cards[i]);
        }
    }

    // printf("me cards: ");
    // for (i = 0; i < MAX_HAND_CARDS_LEN; ++i)
    //     if (cloned_me->cards[i] != NULL)
    //         printf("%s ", cloned_me->cards[i]);
    // printf("\n");
    // printf("played_suit: %d, is_heart_broken: %d\n", played_suit, is_heart_broken);
    assert(cloned_me->candidate_cards[0] != NULL);
}

void update_player_candidate_cards(struct player *p, char played_suit, int is_heart_broken)
{
    size_t i;
    for (i = 0; i < MAX_HAND_CARDS_LEN; ++i) {
        if (p->candidate_cards[i] != NULL)
            free(p->candidate_cards[i]);
        p->candidate_cards[i] = NULL;
    }

    if (played_suit == -1) {
        if (is_heart_broken) {
            for (i = 0; i < MAX_HAND_CARDS_LEN; ++i)
                if (p->cards[i] != NULL)
                    p->candidate_cards[i] = strdup(p->cards[i]);
        } else {
            char **pp = p->candidate_cards;
            for (i = 0; i < MAX_HAND_CARDS_LEN; ++i)
                if (p->cards[i] != NULL && p->cards[i][1] != 'H')
                    *pp++ = strdup(p->cards[i]);
            if (pp - p->candidate_cards == 0)
                for (i = 0; i < MAX_HAND_CARDS_LEN; ++i)
                    if (p->cards[i] != NULL)
                        p->candidate_cards[i] = strdup(p->cards[i]);
        }
    } else {
        char **pp = p->candidate_cards;
        for (i = 0; i < MAX_HAND_CARDS_LEN; ++i)
            if (p->cards[i] != NULL && p->cards[i][1] == played_suit)
                *pp++ = strdup(p->cards[i]);
        if (pp - p->candidate_cards == 0)
            for (i = 0; i < MAX_HAND_CARDS_LEN; ++i)
                if (p->cards[i] != NULL)
                    p->candidate_cards[i] = strdup(p->cards[i]);
    }
    assert(p->candidate_cards[0] != NULL);

    // printf("%s candidate cards: ", p->name);
    // for (i = 0; i < MAX_HAND_CARDS_LEN; ++i)
    //     if (p->candidate_cards[i] != NULL)
    //         printf("%s ", p->candidate_cards[i]);
    // printf("\n");
    // printf("%s cards: ", p->name);
    // for (i = 0; i < MAX_HAND_CARDS_LEN; ++i)
    //     if (p->cards[i] != NULL)
    //         printf("%s ", p->cards[i]);
    // printf("\n");
}

char *choose_played_card_me(struct stru_me *cloned_me, char *current_round_cards[], int current_round_cards_len)
{
    size_t i;
    int candidate_cards_count = 0;
    for (i = 0; i < MAX_HAND_CARDS_LEN; ++i)
        if (cloned_me->candidate_cards[i] != NULL)
            ++candidate_cards_count;

    return cloned_me->candidate_cards[rand() % candidate_cards_count];
}

char *choose_played_card_player(struct player *p, char *current_round_cards[], int current_round_cards_len)
{
    size_t i;
    int candidate_cards_count = 0;
    for (i = 0; i < MAX_HAND_CARDS_LEN; ++i)
        if (p->candidate_cards[i] != NULL)
            ++candidate_cards_count;

    return p->candidate_cards[rand() % candidate_cards_count];
}

void play_card_me(struct stru_me *cloned_me, char *played_card)
{
    cloned_me->cards_count -= 1;

    char **pp = cloned_me->cards;
    while (strcmp(played_card, *pp++) != 0)
        ;
    --pp;
    
    while ((*pp = *(pp+1)) != NULL) {
        ++pp;
    }

    cloned_me->round_card = NULL;
}

void play_card_player(struct player *p, char *played_card)
{
    p->cards_count -= 1;

    char **pp = p->cards;
    // printf("%s\n", played_card);
    
    // for (size_t i = 0; i < MAX_HAND_CARDS_LEN; ++i)
    // {
    //     if (p->cards[i] != NULL)
    //         printf("%s ", p->cards[i]);
    // }
    // printf("\n");

    while (strcmp(played_card, *pp) != 0)
        ++pp;
    
    while ((*pp = *(pp+1)) != NULL) {
        ++pp;
    }

    p->round_card = NULL;
}

void insert_score_cards(char *score_cards[], char *current_round_cards[])
{
    char **pp = score_cards;
    while (*pp != NULL)
        ++pp;

    size_t i;
    for (i = 0; i < 4; ++i) {
        if (current_round_cards[i][1] == 'H' || strcmp(current_round_cards[i], "QS") == 0 || strcmp(current_round_cards[i], "TC") == 0)
            *pp++ = strdup(current_round_cards[i]);
    }
}

void update_deal_score_based_on_score_cards(struct stru_me *me, struct player players[], int is_AH_exposed)
{
    size_t i, j;

    size_t score_cards_len = 0;
    for (i = 0; i < MAX_HAND_CARDS_LEN; ++i)
        if (me->score_cards[i] != NULL)
            ++score_cards_len;

    for (i = 0; i < 3; ++i)
        for (j = 0; j < MAX_HAND_CARDS_LEN; ++j)
            if (players[i].score_cards[j] != NULL)
                ++score_cards_len;

    assert(score_cards_len == 15);

    int32_t score = 0;
    int has_tc = 0;
    for (i = 0; i < MAX_HAND_CARDS_LEN; ++i) {
        if (me->score_cards[i] != NULL) {
            if (me->score_cards[i][1] == 'H') {
                if (is_AH_exposed)
                    score += -2;
                else
                    score += -1;
            }
            if (strcmp(me->score_cards[i], "QS") == 0)
                score += -13;
            if (strcmp(me->score_cards[i], "TC") == 0)
                has_tc = 1;
        }
    }
    if (score >= -26)
        score = 0;
    if (has_tc)
        score *= 2;
    me->deal_score = score;

    for (i = 0; i < 3; ++i) {
        score = 0;
        has_tc = 0;
        for (j = 0; j < MAX_HAND_CARDS_LEN; ++j) {
            if (players[i].score_cards[j] != NULL) {
                if (players[i].score_cards[j][1] == 'H') {
                    if (is_AH_exposed)
                        score += -2;
                    else
                        score += -1;
                }
                if (strcmp(players[i].score_cards[j], "QS") == 0)
                    score += -13;
                if (strcmp(players[i].score_cards[j], "TC") == 0)
                    has_tc = 1;
            }
        }
        if (score >= -26) {
            me->deal_score = -26;
            score = 0;
        }
        if (has_tc)
            score *= 2;
        players[i].deal_score = score;
    }
}
