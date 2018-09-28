# node-hearts-mcts
Node.js native addon. Use Monte Carlo tree Search to play Hearts.


## Hearts game rules

wiki: https://en.wikipedia.org/wiki/Hearts

### Terminology

1. Cards
    1. A card is composed of 2 characters.
    1. The first character indicates the rank: 2~9, T(10), J(J), Q(Q), K(K), A(Ace).
    1. The second character indicates the suit: H(eart), S(pade), C(lub), D(iamond).
    1. Example:
        * 3H -> 3♥
        * TS -> 10♠
        * QC -> Q♣
        * AD -> A♦

1. Scoring
    1. ♥[2~9, J, Q, K, A] -1
    1. ♠Q (QS) -13

### special rules:

1. ♣10 (TC): Doubles the score of the player in this deal
1. Hearts Exposed:
    1. When the ♥A (AH) is exposed in a deal, the points of all ♥ cards are doubled in this deal.

## TODO

1. Use Monte Carlo simulation to rollout games to get the Shooting the Moon rate, to adjust passed cards, exposed card and MCTS Shooting the Moon 'wins'.
1. Add the passed cards to other players' cards, so we can know what cards of opponent has.
1. Add exposed card.
1. Add Hearts broken.

## References

1. http://mcts.ai/
