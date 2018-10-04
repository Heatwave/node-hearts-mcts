const mcts = require('../index.js');

function getRandomInt(max) {
    return Math.floor(Math.random() * Math.floor(max));
}

const all_pokers = [
    '2H', '2S', '2C', '2D', '3H', '3S', '3C', '3D', '4H', '4S', '4C', '4D',
    '5H', '5S', '5C', '5D', '6H', '6S', '6C', '6D', '7H', '7S', '7C', '7D',
    '8H', '8S', '8C', '8D', '9H', '9S', '9C', '9D', 'TH', 'TS', 'TC', 'TD',
    'JH', 'JS', 'JC', 'JD', 'QH', 'QS', 'QC', 'QD', 'KH', 'KS', 'KC', 'KD',
    'AH', 'AS', 'AC', 'AD'
];

const me = {
    "player_name": 'P1001',
    "deal_score": 0,
    "cards": ["TS", "KS", "9S", "6S", "2S", "JH", "7H", "QC", "TC", "9C", "5C", "JD", "6D"],
    "cards_count": 13,
    "candidate_cards": [],
    "score_cards": [],
    "left_cards": []
};

function start() {
    me.cards = [];

    let pokers = JSON.parse(JSON.stringify(all_pokers));
    let i = 0;
    for (; i < 13; ++i) {
        const randomIndex = getRandomInt(pokers.length);
        me.cards.push(pokers[randomIndex]);
        pokers.splice(pokers.indexOf(pokers[randomIndex]), 1);
    }

    me.left_cards = all_pokers.filter(value => {
        if (me.cards.indexOf(value) !== -1)
            return false;
        else
            return true;
    });

    let start = Date.now();
    const shooting_rate = mcts.simulation(me.cards, me.left_cards);
    // console.log('time spent: ', Date.now() - start);
    console.log(shooting_rate);
    if (shooting_rate > 0.01)
        console.log(me.cards);
}

let i = 100;
while (i-- > 0) {
    start();
}
