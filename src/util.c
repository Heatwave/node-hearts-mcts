#include <assert.h>
#include <ctype.h>
#include <stdlib.h>

#include "mcts.h"


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
