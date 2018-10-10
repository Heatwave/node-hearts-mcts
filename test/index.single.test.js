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
    "deal_score": -28,
    "cards": ['5D', '9D', 'TD', 'QH'],
    "cards_count": 4,
    "candidate_cards": ['5D', '9D', 'TD', 'QH'],
    "score_cards": ['4H', '5H', '7H', '8H', 'TH', 'KH', 'AH', 'TC']
};

const players = [
    {
        "player_name": 'p1', "deal_score": -2, "cards_count": 4, "round_card": '', "score_cards": ["JH"], "cards": []
    },
    {
        "player_name": 'p2', "deal_score": 0, "cards_count": 4, "round_card": '', "score_cards": [], "cards": []
    },
    {
        "player_name": 'p3', "deal_score": 0, "cards_count": 4, "round_card": '', "score_cards": [], "cards": []
    }
];

const player_order = ['me', 'p1', 'p2', 'p3'];

let left_cards = [
    '2D', '2S', '6S', 'QS',
    '2H', '3H', '6H', '9H',
    '3D', '7D', 'JD', 'AC'
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
const action = mcts.uct(15000, me, players, player_order, left_cards, 0, 1);
console.log(`time spent: ${Date.now() - start}`);
console.log('action:' + action);
