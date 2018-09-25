const mcts = require('../index.js');

const all_pokers = [
    '2H', '2S', '2C', '2D', '3H', '3S', '3C', '3D', '4H', '4S', '4C', '4D',
    '5H', '5S', '5C', '5D', '6H', '6S', '6C', '6D', '7H', '7S', '7C', '7D',
    '8H', '8S', '8C', '8D', '9H', '9S', '9C', '9D', 'TH', 'TS', 'TC', 'TD',
    'JH', 'JS', 'JC', 'JD', 'QH', 'QS', 'QC', 'QD', 'KH', 'KS', 'KC', 'KD',
    'AH', 'AS', 'AC', 'AD'
];

const me = {
    "player_name": 'me',
    "deal_score": 0,
    "cards": ["KS", "QS", "TS", "8S", "3S", "9H", "5H", "2H", "8C", "5C", "9D", "7D", "3D"],
    "cards_count": 13,
    "candidate_cards": ['8C', '5C'],
    "score_cards": []
};

const players = [
    {
        "player_name": 'p1', "deal_score": 0, "cards_count": 12, "round_card": '2C', "score_cards": []
    },
    {
        "player_name": 'p2', "deal_score": 0, "cards_count": 12, "round_card": '6C', "score_cards": []
    },
    {
        "player_name": 'p3', "deal_score": 0, "cards_count": 12, "round_card": '9C', "score_cards": []
    }
];

const player_order = ['p1', 'me', 'p2', 'p3'];

const left_cards = all_pokers.filter(value => {
    if (me.cards.indexOf(value) >= 0)
        return false;
    for (p of players) {
        if (value === p.round_card)
            return false;
    }

    return true;
});

var start = Date.now();
const action = mcts.uct(15000, me, players, player_order, left_cards);
console.log(`time spent: ${Date.now() - start}`);
console.log('action:' + action);
