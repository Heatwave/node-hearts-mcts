const mcts = require('../index.js');

const all_pokers = [
    '2H', '2S', '2C', '2D', '3H', '3S', '3C', '3D', '4H', '4S', '4C', '4D',
    '5H', '5S', '5C', '5D', '6H', '6S', '6C', '6D', '7H', '7S', '7C', '7D',
    '8H', '8S', '8C', '8D', '9H', '9S', '9C', '9D', 'TH', 'TS', 'TC', 'TD',
    'JH', 'JS', 'JC', 'JD', 'QH', 'QS', 'QC', 'QD', 'KH', 'KS', 'KC', 'KD',
    'AH', 'AS', 'AC', 'AD'
];

const me = {
    "player_name": 'stan',
    "deal_score": 0,
    "cards": ["QS", "TS", "6S", "4S", "2S", "KH", "QH", "9H", "7C", "AD", "7D", "6D"],
    "left_cards": [
        "2H", "2D", "3H", "3S", "3C", "3D", "4H", "4C", "4D", "5H", "5S", "5D", "6H", "7H", "7S", "8H", "8S", "8C", "8D", "9S", "9C", "9D", "TH", "TC", "JH", "JS", "JC", "JD", "QC", "QD", "KS", "KC", "KD", "AH", "AS", "AC"
    ],
    "round_card": '',
    "cards_count": 12,
    "candidate_cards": ["QS","TS","6S","4S","2S","7C","AD","7D","6D"],
    "score_cards": []
};

const players = [
    {
        "player_name": 'player2', "deal_score": 0, "cards_count": 12, "round_card": '', "score_cards": [], "cards": [], "suits_status": [0, 1, 0, 1]
    },
    {
        "player_name": 'player3', "deal_score": 0, "cards_count": 12, "round_card": '', "score_cards": [], "cards": [], "suits_status": [1, 0, 0, 0]
    },
    {
        "player_name": 'player4', "deal_score": 0, "cards_count": 12, "round_card": '', "score_cards": [], "cards": [], "suits_status": [0, 0, 1, 1]
    }
];

const player_order = ['stan', 'player2', 'player3', 'player4'];

let total_scores = {};
let score = 0;

for (const card of me.candidate_cards) {
    me.round_card = card;
    score = mcts.simulate(2000, me, players, player_order, 1);
    total_scores[card] = score;
}

console.log(total_scores);

let best_card;
let best_score;
for (const card in total_scores) {
    if (total_scores.hasOwnProperty(card)) {
        const score = total_scores[card];
        if (!best_score || best_score < score) {
            best_score = score;
            best_card = card;
        }
    }
}

console.log('choosed card: ', best_card);
