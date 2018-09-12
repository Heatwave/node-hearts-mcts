const mcts = require('./index.js');

const all_pokers = [
    '2H', '2S', '2C', '2D',
    '3H', '3S', '3C', '3D',
    '4H', '4S', '4C', '4D',
    '5H', '5S', '5C', '5D',
    '6H', '6S', '6C', '6D',
    '7H', '7S', '7C', '7D',
    '8H', '8S', '8C', '8D',
    '9H', '9S', '9C', '9D',
    'TH', 'TS', 'TC', 'TD',
    'JH', 'JS', 'JC', 'JD',
    'QH', 'QS', 'QC', 'QD',
    'KH', 'KS', 'KC', 'KD',
    'AH', 'AS', 'AC', 'AD'
];

const me = {
    "player_name": 'me',
    "deal_score": 0,
    "cards": ['AS', 'KS', '9S', '8S'],
    "cards_count": 4,
    "candidate_cards": ['AS', 'KS', '9S', '8S']
};

const players = [
    {
        "player_name": 'p1',
        "deal_score": 0,
        "cards_count": 4,
        "round_card": ''
    },
    {
        "player_name": 'p2',
        "deal_score": 0,
        "cards_count": 4,
        "round_card": ''
    },
    {
        "player_name": 'p3',
        "deal_score": 0,
        "cards_count": 3,
        "round_card": '5S'
    }
];

const player_order = ['p3', 'me', 'p1', 'p2'];

const left_cards = all_pokers.filter(value => {
    if (me.cards.indexOf(value) >= 0)
        return false;
    for (p of players) {
        if (value === p.round_card)
            return false;
    }

    return true;
});

console.log("left_cards: " + left_cards);

/*
var r = {};

for (var i = 0; i < 50; i++) {
    const action = mcts(20000, me, players, player_order, left_cards);
    if (action in r)
        r[action] += 1;
    else
        r[action] = 1;
}

console.log(r);
*/

var start = Date.now();
const action = mcts(20000, me, players, player_order, left_cards);
console.log(`time spent: ${Date.now() - start}`);
console.log(action);

