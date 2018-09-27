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
    "cards": ["AS", "QS", "JS", "8H", "6H", "3H", "AC", "QC", "6C", "3C", "9D", "5D", "2D"],
    "cards_count": 13,
    "candidate_cards": [],
    "score_cards": [],
    "left_cards": []
};

me.left_cards = all_pokers.filter(value => {
    if (me.cards.indexOf(value) !== -1)
        return false;
    else
        return true;
});

const shooting_rate = mcts.simulation(me.cards, me.left_cards);
console.log(shooting_rate);
