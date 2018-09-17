#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <ctype.h>

#include "uct.h"


char *do_uct(int32_t itermax, struct stru_me me, struct player players[], char *player_order[], char* left_cards[])
{
    char *result_action = "2C";
    int32_t i;

    struct node rootnode;
    init_rootnode(&rootnode, &me);

    struct node *action_node;
    struct node *new_child_node;

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

        untried_moves_count = get_untried_moves_count(action_node->untried_moves, MAX_CARDS_LEN);

        child_nodes_count = 0;
        for (j = 0; j < MAX_CHILDREN_LEN; ++j) {
            if (action_node->children[j] != NULL)
                ++child_nodes_count;
        }

        while (untried_moves_count == 0 && child_nodes_count != 0) {
            new_child_node = uct_select_child(action_node);
            action_node = new_child_node;
            selected_move = strdup(new_child_node->move);
            do_move(selected_move, &cloned_me, cloned_players, cloned_player_order);

            untried_moves_count = get_untried_moves_count(action_node->untried_moves, MAX_CARDS_LEN);
            child_nodes_count = 0;
            for (j = 0; j < MAX_CHILDREN_LEN; ++j) {
                if (action_node->children[j] != NULL)
                    ++child_nodes_count;
            }
        }

        // Expand
        untried_moves_count = get_untried_moves_count(action_node->untried_moves, MAX_CARDS_LEN);

        if (untried_moves_count > 0) {
            random_index = rand() % untried_moves_count;
            selected_move = action_node->untried_moves[random_index];
            do_move(selected_move, &cloned_me, cloned_players, cloned_player_order);
            new_child_node = node_add_child(selected_move, action_node, &cloned_me);
            action_node = new_child_node;
        }

        untried_moves_count = get_untried_moves_count(action_node->untried_moves, MAX_CARDS_LEN);

        // Rollout
        while (cloned_me.cards_count > 0) {
            do_move(NULL, &cloned_me, cloned_players, cloned_player_order);
        }

        assert(cloned_players[0].cards_count == 0);
        assert(cloned_players[1].cards_count == 0);
        assert(cloned_players[2].cards_count == 0);

        // Backpropagate
        while (action_node != NULL) {
            update_node_with_result(action_node, &cloned_me, cloned_players);
            action_node = action_node->parent;
        }
    }

    /*
    for (i = 0; i < MAX_CHILDREN_LEN; i++) {
        if (rootnode.children[i] != NULL)
            printf("move: %s, wins: %lf, visits: %d\t", rootnode.children[i]->move, rootnode.children[i]->wins, rootnode.children[i]->visits);
    }
    printf("\n");
    */

    size_t best_visits = 0;
    result_action = strdup(rootnode.children[0]->move);
    for (i = 0; i < MAX_CHILDREN_LEN; ++i) {
        if (rootnode.children[i] != NULL && rootnode.children[i]->move != NULL
        && rootnode.children[i]->visits > best_visits) {
            best_visits = rootnode.children[i]->visits;
            result_action = strdup(rootnode.children[i]->move);
        }
    }

    clean_nodes_mem(&rootnode);

    return result_action;
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
}

void init_rootnode(struct node *rootnode, struct stru_me *me)
{
    rootnode->move = NULL;
    rootnode->parent = NULL;

    uint32_t i;
    for (i = 0; i < sizeof(rootnode->children) / sizeof(rootnode->children[0]); i++) {
        rootnode->children[i] = NULL;
    }

    rootnode->wins = 0.0;
    rootnode->visits = 0;

    char **pp = rootnode->untried_moves;
    size_t moves_count = 0;
    for (i = 0; i < MAX_CARDS_LEN; i++) {
        if (me->candidate_cards[i] != NULL) {
            *pp = malloc(strlen(me->candidate_cards[i])+1);
            strcpy(*pp++, me->candidate_cards[i]);
            ++moves_count;
        }
    }

    while ((pp - rootnode->untried_moves) < MAX_CARDS_LEN) {
        *pp = NULL;
        ++pp;
    }
    assert((pp - rootnode->untried_moves) == MAX_CARDS_LEN);
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
        if (arr[i] != NULL && strlen(arr[i]) == 2) {
            ++count;
        }
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

void update_node_with_result(struct node *action_node, struct stru_me *cloned_me, struct player players[])
{
    action_node->visits += 1;

    size_t ranking = 0;
    double win = 0.0;
    size_t i;
    int is_shooting_the_moon = 1;
    for (i = 0; i < 3; ++i) {
        if (players[i].deal_score != 0)
            is_shooting_the_moon = 0;
        if (cloned_me->deal_score < players[i].deal_score)
            ++ranking;
    }

    if (ranking == 0)
        win = 1;
    else if (ranking == 1)
        win = 0.75;
    else if (ranking == 2)
        win = 0.25;

    if (is_shooting_the_moon == 1)
        win = 1;

    action_node->wins += win;
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

        // update played_suit based on first played card
        if (i == 0 && played_suit == 0) {
            played_suit = (*(pp-1))[1];
        }
    }

    assert((pp - cur_round_cards) == 4);

    if (played_suit == 0) {
        played_suit = cur_round_cards[0][1];
    }

    size_t hearts_count = 0;
    for (i = 0; i < 4; i++) {
        if (cur_round_cards[i][1] == 'H')
            ++hearts_count;
    }

    int32_t round_score = hearts_count * (-1);

    for (i = 0; i < 4; i++) {
        // TODO: add TC to double the score
        if (strcmp(cur_round_cards[i], "QS") == 0)
            round_score += -13;
    }

    size_t winner_index = 0;
    char max_rank = 0;

    for (i = 0; i < 4; i++) {
        if (cur_round_cards[i][1] != played_suit)
            continue;
        if (rankcmp(cur_round_cards[i][0], max_rank) > 0) {
            max_rank = cur_round_cards[i][0];
            winner_index = i;
        }
    }

    char *winner_name = cloned_player_order[winner_index];
    if (strcmp(winner_name, cloned_me->name) == 0) {
        cloned_me->deal_score += round_score;
    } else {
        for (i = 0; i < 3; i++) {
            if (strcmp(winner_name, cloned_players[i].name) == 0) {
                cloned_players[i].deal_score += round_score;
                break;
            }
        }
    }

    // adjust cloned_player_order
    char *last_order_name;
    while (strcmp(cloned_player_order[0], winner_name) != 0) {
        last_order_name = strdup(cloned_player_order[3]);
        for (i = 3; i > 0; --i) {
            cloned_player_order[i] = cloned_player_order[i-1];
        }
        cloned_player_order[0] = strdup(last_order_name);
    }

    cloned_me->round_card = "";
    for (i = 0; i < 3; i++)
        cloned_players[i].round_card = "";
}

int rankcmp(char a, char b)
{
    if (isdigit(a))
        a = a - '0';
    else if (a == 'T')
        a = 10;
    else if (a == 'J')
        a = 11;
    else if (a == 'Q')
        a = 12;
    else if (a == 'K')
        a = 13;
    else if (a == 'A')
        a = 14;
    else
        a = 0;

    if (isdigit(b))
        b = b - '0';
    else if (b == 'T')
        b = 10;
    else if (b == 'J')
        b = 11;
    else if (b == 'Q')
        b = 12;
    else if (b == 'K')
        b = 13;
    else if (b == 'A')
        b = 14;
    else
        b = 0;

    if (a > b)
        return 1;
    else if (a < b)
        return -1;
    else
        return 0;
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

    return suit;
}

void pick_card_me(struct stru_me *cloned_me, char played_suit, char *selected_move)
{
    if (cloned_me->round_card != NULL && strlen(cloned_me->round_card) > 1)
        return;

    size_t able_to_played_cards_count = 0, i, random_index;
    char *able_to_played_cards[64];
    char **pp2able_to_played_cards = able_to_played_cards;

    if (selected_move == NULL) {
        if (played_suit == 0) {
            for (i = 0; i < MAX_CARDS_LEN; i++) {
                if (cloned_me->cards[i] != NULL && strlen(cloned_me->cards[i]) == 2) {
                    ++able_to_played_cards_count;
                    *pp2able_to_played_cards++ = strdup(cloned_me->cards[i]);
                }
            }
        } else {
            for (i = 0; i < MAX_CARDS_LEN; i++) {
                if (cloned_me->cards[i] != NULL && strlen(cloned_me->cards[i]) == 2
                && cloned_me->cards[i][1] == played_suit) {
                    ++able_to_played_cards_count;
                    *pp2able_to_played_cards++ = strdup(cloned_me->cards[i]);
                }
            }

            if (able_to_played_cards_count == 0) {
                for (i = 0; i < MAX_CARDS_LEN; i++) {
                    if (cloned_me->cards[i] != NULL && strlen(cloned_me->cards[i]) == 2) {
                        ++able_to_played_cards_count;
                        *pp2able_to_played_cards++ = strdup(cloned_me->cards[i]);
                    }
                }
            }
        }
        random_index = rand() % able_to_played_cards_count;
        selected_move = strdup(able_to_played_cards[random_index]);
    }

    cloned_me->round_card = strdup(selected_move);

    char **pp = cloned_me->cards;
    while (strcmp(selected_move, *pp++) != 0)
        ;

    --pp;

    while (*(pp+1) != NULL && strlen(*(pp+1)) > 1) {
        *pp = *(pp+1);
        ++pp;
    }

    *pp = NULL;

    cloned_me->cards_count -= 1;
}

void pick_card_player(struct player *cur_player, char played_suit)
{
    if (cur_player->round_card != NULL && strlen(cur_player->round_card) > 1)
        return;

    char *able_to_played_cards[64];
    char **pp = able_to_played_cards;
    size_t i;

    if (played_suit != 0) {
        for (i = 0; i < MAX_CARDS_LEN; i++) {
            if (cur_player->cards[i] != NULL && strlen(cur_player->cards[i]) > 1
            && cur_player->cards[i][1] == played_suit)
                *pp++ = strdup(cur_player->cards[i]);
        }
    } else {
        for (i = 0; i < MAX_CARDS_LEN; i++) {
            if (cur_player->cards[i] != NULL && strlen(cur_player->cards[i]) > 1)
                *pp++ = strdup(cur_player->cards[i]);
        }
    }

    while ((pp - able_to_played_cards) < 64) {
        *pp++ = NULL;
    }

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

    --pp;

    while (*(pp+1) != NULL && strlen(*(pp+1)) == 2) {
        *pp = *(pp+1);
        ++pp;
    }

    *pp = NULL;

    cur_player->cards_count -= 1;
}

struct node *uct_select_child(const struct node *n)
{
    struct node *selected;
    size_t i;
    double uct_value, best_value = 0.0;
    double C = 1.0 / sqrt(2.0);
    for (i = 0; i < sizeof(n->children) / sizeof(n->children[0]); i++) {
        if (n->children[i] == NULL)
            continue;


        struct node *c = n->children[i];

        // Upper Confidence Bounds formula
        // UCB = wins / visits + C * sqrt(2 * ln(total_visits) / visits)
        uct_value = c->wins / c->visits + C * sqrt(2 * log(n->visits) / c->visits);

        if (uct_value > best_value) {
            selected = c;
            best_value = uct_value;
        }
    }

    return selected;
}

struct node *node_add_child(char *selected_move, struct node *action_node, struct stru_me *me)
{
    size_t child_index = 0;
    while (action_node->children[child_index] != NULL)
        ++child_index;

    assert(child_index < MAX_CHILDREN_LEN);

    // remove selected_move from rootnode untried_moves
    char **pp = action_node->untried_moves;
    while (strcmp(selected_move, *pp++) != 0)
        ;

    --pp;

    while (*(pp+1) != NULL && strlen(*(pp+1)) == 2) {
        *pp = *(pp+1);
        ++pp;
    }

    *pp = NULL;

    struct node *new_child = malloc(sizeof(struct node));
    init_childnode(new_child, selected_move, action_node, me);

    action_node->children[child_index] = new_child;

    return new_child;
}

void init_childnode(struct node *child, char *move, struct node *parent, struct stru_me *me)
{
    child->move = strdup(move);
    child->parent = parent;
    child->wins = 0.0;
    child->visits = 0;

    size_t i;
    for (i = 0; i < MAX_CHILDREN_LEN; i++) {
        child->children[i] = NULL;
    }

    for (i = 0; i < MAX_CARDS_LEN; i++) {
        if (parent->untried_moves[i] != NULL && strlen(parent->untried_moves[i]) == 2)
            child->untried_moves[i] = strdup(parent->untried_moves[i]);
        else
            child->untried_moves[i] = NULL;
    }
}

void clean_nodes_mem(struct node *rootnode)
{
    if (rootnode->move != NULL)
        free(rootnode->move);

    size_t i;

    for (i = 0; i < MAX_CARDS_LEN; ++i)
        if (rootnode->untried_moves[i] != NULL)
            free(rootnode->untried_moves[i]);

    for (i = 0; i < MAX_CHILDREN_LEN; ++i)
        if (rootnode->children[i] != NULL)
            clean_nodes_mem(rootnode->children[i]);
}
