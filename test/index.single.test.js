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
    "deal_score": -14,
    "cards": ["2D", "9D", "TD", "4S", "8S", "QS"],
    "cards_count": 6,
    "candidate_cards": ["2D", "9D", "TD", "4S", "8S", "QS"],
    "score_cards": ["TC", "2H", '6H', '7H', '8H', '9H', 'TH', 'AH']
};

const players = [
    {
        "player_name": 'p1', "deal_score": -1, "cards_count": 5, "round_card": '3H', "score_cards": ["KH"]
    },
    {
        "player_name": 'p2', "deal_score": 0, "cards_count": 6, "round_card": '', "score_cards": []
    },
    {
        "player_name": 'p3', "deal_score": 0, "cards_count": 6, "round_card": '', "score_cards": []
    }
];

const player_order = ['p1', 'me', 'p2', 'p3'];

let left_cards = [
    '3C', '3D', '4H', '4C', '4D', '5H', '5D', '6D', '7D', '8D', '9C', 'JH', 'JC', 'JD', 'QH', 'QC', 'QD'
];

// left_cards = left_cards.filter(value => {
//     if (me.cards.indexOf(value) >= 0)
//         return false;
//     if (me.score_cards.indexOf(value) !== -1)
//         return false;
//     for (p of players) {
//         if (value === p.round_card)
//             return false;
//         if (p.score_cards.indexOf(value) !== -1)
//             return false;
//     }

//     return true;
// });

var start = Date.now();
const action = mcts.uct(20000, me, players, player_order, left_cards, 0);
console.log(`time spent: ${Date.now() - start}`);
console.log('action:' + action);
