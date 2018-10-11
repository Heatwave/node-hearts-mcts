'use strict';

const assert = require('assert');

const mcts = require('../index.js');

const MCTS_ITERMAX = 15000;

function cloneObj(obj) {
    return JSON.parse(JSON.stringify(obj));
}

const all_pokers = [
    '2H', '2S', '2C', '2D', '3H', '3S', '3C', '3D', '4H', '4S', '4C', '4D',
    '5H', '5S', '5C', '5D', '6H', '6S', '6C', '6D', '7H', '7S', '7C', '7D',
    '8H', '8S', '8C', '8D', '9H', '9S', '9C', '9D', 'TH', 'TS', 'TC', 'TD',
    'JH', 'JS', 'JC', 'JD', 'QH', 'QS', 'QC', 'QD', 'KH', 'KS', 'KC', 'KD',
    'AH', 'AS', 'AC', 'AD'
];

function getRandomInt(max) {
    return Math.floor(Math.random() * Math.floor(max));
}

function fisher_yates(arr) {
    if (!Array.isArray(arr) || arr.length < 1)
        return;

    let len = arr.length;
    let temp, i;

    while (--len) {
        i = getRandomInt(len);
        temp = arr[len];
        arr[len] = arr[i];
        arr[i] = temp;
    }
}

function initMe(me, pokers, index) {
    me.deal_score = 0;
    me.cards = pokers.slice(index, index + 13);
    me.cards_count = 13;
    me.round_card = '';
    me.candidate_cards = [];
    me.left_cards = pokers.slice(index + 13);
}

function initPlayer(p, pokers, index) {
    p.deal_score = 0;
    p.cards = pokers.slice(index, index + 13);
    p.cards_count = 13;
    p.round_card = '';
    p.candidate_cards = [];
}

function updateCandidateCards(p, suit, isHeartBroken) {
    p.candidate_cards = [];

    if (!suit) {
        if (isHeartBroken)
            p.candidate_cards = cloneObj(p.cards);
        else {
            p.candidate_cards = p.cards.filter(value => {
                if (value[1] === 'H')
                    return false;
                else
                    return true;
            });
            if (p.candidate_cards.length === 0)
                p.candidate_cards = cloneObj(p.cards);
        }
        return;
    }

    let candidates = p.cards.filter(value => {
        if (value[1] === suit)
            return true;
        else
            return false;
    });

    if (candidates.length !== 0) {
        if (isHeartBroken) {
            p.candidate_cards = candidates;
        } else {
            candidates = candidates.filter(value => {
                if (value[1] === 'H')
                    return false;
                else
                    return true;
            });
            p.candidate_cards = candidates;
        }
    } else {
        if (isHeartBroken) {
            p.candidate_cards = cloneObj(p.cards);
        } else {
            candidates = p.cards.filter(value => {
                if (value[1] === 'H')
                    return false;
                else
                    return true;
            });
            if (candidates.length !== 0)
                p.candidate_cards = candidates;
            else {
                p.candidate_cards = cloneObj(p.cards);
            }
        }
    }

    assert(p.candidate_cards.length > 0);
}

function pickCardMe(me, players, order) {
    let action;
    if (me.candidate_cards.length === 1)
        action = me.candidate_cards[0];
    else {
        action = mcts.uct(MCTS_ITERMAX, me, players, order, me.left_cards, 0, 0);
        assert(action.length === 2);
    }

    me.round_card = action;
    me.cards.splice(me.cards.indexOf(action), 1);
    me.cards_count -= 1;
}

function pickCardPlayer(player) {
    player.round_card = player.candidate_cards[getRandomInt(player.candidate_cards.length)];
    player.cards.splice(player.cards.indexOf(player.round_card), 1);
    player.cards_count -= 1;
}

function rankCmp(a, b) {
    if (a === 'T')
        a = 10;
    else if (a === 'J')
        a = 11;
    else if (a === 'Q')
        a = 12;
    else if (a === 'K')
        a = 13;
    else if (a === 'A')
        a = 14;


    if (b === 'T')
        b = 10;
    else if (b === 'J')
        b = 11;
    else if (b === 'Q')
        b = 12;
    else if (b === 'K')
        b = 13;
    else if (b === 'A')
        b = 14;

    assert(!isNaN(parseInt(a, 10)));
    assert(!isNaN(parseInt(b, 10)));

    if (a > b)
        return 1;
    else if (a < b)
        return -1;
    else
        return 0;
}

function getWinnerIndex(roundCards) {
    assert(roundCards.length === 4);
    const suit = roundCards[0][1];
    const cards = roundCards.filter(value => {
        if (value[1] !== suit)
            return false;
        else
            return true;
    });
    let index = 0;
    let maxRank = cards[index][0];
    for (let i = 0; i < cards.length; i++) {
        const value = cards[i];
        if (rankCmp(value[0], maxRank) > 0) {
            index = i;
            maxRank = value[0];
        }
    }

    return index;
}

function updateLeftCards(me, roundCards) {
    me.left_cards = me.left_cards.filter(value => {
        if (roundCards.indexOf(value) !== -1)
            return false;
        else
            return true;
    })
}

function updateScoreCards(player, roundCards) {
    roundCards.forEach(value => {
        if (value[1] === 'H' || value === 'QS' || value === 'TC')
            player.score_cards.push(value);
    });
}

function getScoreFromCards(scoreCards) {
    let score = 0;
    scoreCards.forEach(card => {
        if (card[1] === 'H')
            score += -1;
        else if (card === 'QS')
            score += -13;
    });

    if (score === -26)
        score = 104;


    if (scoreCards.indexOf('TC') !== -1)
        score *= 2;
    return score;
}

let total_scores = {
    me: 0,
    p1: 0,
    p2: 0,
    p3: 0
};

function start() {
    let pokers = cloneObj(all_pokers);
    fisher_yates(pokers);

    let name2playersMapping = {};

    let me = {
        'player_name': 'me',
        'deal_score': 0,
        'cards': [],
        'cards_count': 13,
        'round_card': '',
        'candidate_cards': [],
        'score_cards': [],
        'left_cards': []
    };
    name2playersMapping.me = me;

    let players = [
        {
            'player_name': 'p1', 'deal_score': 0, 'cards': [], 'cards_count': 13, 'round_card': '', 'candidate_cards': [], 'score_cards': [], "suits_status": []
        },
        {
            'player_name': 'p2', 'deal_score': 0, 'cards': [], 'cards_count': 13, 'round_card': '', 'candidate_cards': [], 'score_cards': [], "suits_status": []
        },
        {
            'player_name': 'p3', 'deal_score': 0, 'cards': [], 'cards_count': 13, 'round_card': '', 'candidate_cards': [], 'score_cards': [], "suits_status": []
        }
    ];
    players.forEach(value => {
        name2playersMapping[value.player_name] = value;
    });

    let player_order = ['me', 'p1', 'p2', 'p3'];

    let pokersIndex = 0;

    initMe(me, pokers, pokersIndex);
    assert(me.cards.length == 13);

    players.forEach(player => {
        pokersIndex += 13;
        initPlayer(player, pokers, pokersIndex);
        assert(player.cards.length === 13);
    });

    let name_with_2c = '';
    if (me.cards.indexOf('2C') !== -1) {
        name_with_2c = me.player_name;
        me.round_card = '2C';
        me.cards.splice(me.cards.indexOf('2C'), 1);
        me.cards_count -= 1;
    }
    players.forEach(p => {
        if (p.cards.indexOf('2C') !== -1) {
            name_with_2c = p.player_name;
            p.round_card = '2C';
            p.cards.splice(p.cards.indexOf('2C'), 1);
            p.cards_count -= 1;
        }
    });

    while (true) {
        if (player_order[0] === name_with_2c)
            break
        else {
            player_order.unshift(player_order.pop());
        }
    }

    let roundCount = 13;

    let playedSuit = 'C';

    let isHeartBroken = false;

    let roundCards = ['2C'];

    updateLeftCards(me, ['2C']);
    while (roundCount-- > 0) {
        player_order.forEach(name => {
            let player = name2playersMapping[name];
            if (player.round_card !== '')
                return;
            updateCandidateCards(player, playedSuit, isHeartBroken);
            if (name === me.player_name)
                pickCardMe(me, players, player_order);
            else
                pickCardPlayer(player);
            roundCards.push(player.round_card);
            updateLeftCards(me, roundCards);

            if (playedSuit === '')
                playedSuit = player.round_card[1];

            if (!isHeartBroken) {
                for (const card of roundCards) {
                    if (card[1] === 'H') {
                        isHeartBroken = true;
                        break;
                    }
                }
            }
        });

        let winnerIndex = getWinnerIndex(roundCards);
        let winnerName = player_order[winnerIndex];

        while (true) {
            if (player_order[0] === winnerName)
                break
            else {
                player_order.unshift(player_order.pop());
            }
        }

        updateScoreCards(name2playersMapping[winnerName], roundCards);

        updateLeftCards(me, roundCards);

        player_order.forEach(name => {
            let player = name2playersMapping[name];
            player.round_card = '';
        });
        roundCards = [];
        playedSuit = '';
    }

    me.deal_score = getScoreFromCards(me.score_cards);
    for (const p of players) {
        p.deal_score = getScoreFromCards(p.score_cards);
    }

    let me_rank = 4;
    let scores = {
        'me': me.deal_score
    };
    for (const p of players) {
        total_scores[p.player_name] += p.deal_score;
        scores[p.player_name] = p.deal_score;
        if (me.deal_score > p.deal_score)
            me_rank -= 1;
    }
    total_scores.me += me.deal_score;
    console.log(scores, me_rank);
    return me_rank;
}

let i = 100;
let rank = 4;
let result = [0, 0, 0, 0];
while (--i) {
    console.log('i:', i);
    rank = start();
    result[rank-1]++;
}
console.log(result);
console.log(total_scores);