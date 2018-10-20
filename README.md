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
1. If one of the players pick a card it's not the played suit, we can know he/her has no the cards with that suit.

## References

1. http://mcts.ai/

## Others

需要关注的标志>=5张算为长牌 （louisia) 
 黑桃QKA的位置 
 是否垫了红桃 
 各个玩家绝张花色 
   一. 
 判断手牌是否可以全红，满足以下条件： (Leo) 
 红桃A，且至少有一张KQJ 
 红桃为长牌 
 黑桃QKA至少有两张 
 草花AK至少1张 
 方块AK至少1张 
 有一门花色为长牌 
     二.正常路线。 
   传牌：(Leo) 
 1、先判断黑桃 
 有黑桃Q但是黑桃少于5张：传走。 
 有黑桃KA但是总数少于5张：传走。 
 如果只有黑桃QKA就全都传走。 
   有黑桃Q但是黑桃长牌：可以留下来用来垫分给别人  
 有黑桃KA但是黑桃长牌：则判断其他花色是否有更想传走的牌，就可以不传了。 
 没有黑桃QKA的时候不传黑桃。 
 黑桃QKA都有的时候如果黑桃长牌，可以留下来垫分。 
     2. 在决定传哪些黑桃之后，以下牌传走： 
 1.红桃大牌（JQKA） 
 2.草花2 
 3.自己只有一两张的花色(除非是很小的比如2和3) 
 4.如果某一门花色有3张5以下的牌，这门花色再多大牌也可以不用传(比如红桃2345QKA的情况就不用传红桃了) 
   出牌： 
   跟出方片 （Jason) 
 a. 当判断所有玩家手中都有该花色时，就出手中最大牌 
 b. 当某位玩家手中花色绝张时: 
 b.1 如果处于安全位置就出最大牌 
 b.2 如果并不安全，就出非最大牌（比最大的小就行，不一定最小）,保证不被垫分 
 c. 自己花色绝张时，垫自己非长牌花色。 
 优先垫黑桃Q；如果黑桃Q已出（自己或者别家），其次红桃AKQ，再其次其他花色的AKQJ，然后是其他的非长牌花色；如果黑桃Q还没出，其次黑桃KA，再其次红桃AKQ，再其次其他花色的AKQJ，然后是其他的非长牌花色。 
   跟出草花 （Jason) 
 自己有草花10 
       有玩家出JQKA，就出草花10 
        没有玩家出JQKA，则按照下面abc的策略 
 自己没有草花10 
        草花10还没有出，就出小于10的最大牌 
        草花10已出，则按照下面abc的策略 
 a. 当判断所有玩家手中都有该花色时，就出手中最大牌 
 b. 当某位玩家手中花色绝张时: 
 b.1 如果处于安全位置就出最大牌 
 b.2 如果并不安全，就出非最大牌（比最大的小就行，不一定最小）,保证不被垫分 
 c. 自己花色绝张时，垫自己非长牌花色。 
 优先垫黑桃Q；如果黑桃Q已出（自己或者别家），其次红桃AKQ，再其次其他花色的AKQJ，然后是其他的非长牌花色；如果黑桃Q还没出，其次黑桃KA，再其次红桃AKQ，再其次其他花色的AKQJ，然后是其他的非长牌花色。 
       跟出红桃出非最大牌。 
 如果无论如何都是自己大，就出最大牌 
   跟出黑桃 (Jason) 
 手里没有QKA，当判断所有玩家手中都有该花色时，就出手中最大牌 
 手里没有QKA，当某位玩家手中花色绝张时，就出非最大牌 
 手里没有QKA，自己花色绝张时，首先垫自己非长牌花色的大牌：优先红桃AKQ，再其次其他花色的AKQ；长牌不垫 
 手里有Q，需要判断AK的位置，安全则出Q，不安全则出其他 
 手里有AK或其中之一，需要判断Q的位置，如果安全则出AK，不安全出其他 
 手里有Q和AK或其中之一，用A、K、J等牌收 
   领出 (Louisia) 
 出其他玩家有的花色，出较小牌（保证不是最大即可） 
 不要出自己的长套花色，如果非出不可，出小牌。 
 如果只剩自己有，坚决不出。 
 如果将黑桃AK传给了上家，领出不出黑桃（直到Q已被收） 
 如果将黑桃AK传给了下家，自己没有收到上家传来的黑桃Q，领出的话就坚决出黑桃，直到Q已被收 
 如果有人垫了红桃，且外面的红桃还没有出光，则自己用小牌领出红桃 
     三.全收路线 
 传牌 
   留住每门花色 10 以上的牌 
 长牌花色全部保留 
 红桃为非长牌时，红桃 10 以下的牌必传走。 
 黑桃至少留 1 张5以下小牌，如果没有就留一张10以下最小牌。 
 草花2 必留，如果没有就留 1 张5以下小牌，如果没有就留一张10以下最小牌。 
     出牌 
   每一轮的时候，要计算自己手中的顶大牌数量N（各个花色领出时最大的牌） 
 13-当前轮数<=N+2时，要尽可能的大，获得领出权 
   此前的策略 
   跟出方片 
 a. 方片花色第一轮出牌且已出牌中没有分，跟出最小牌，如果绝门，就贴草花10以下最小牌，没有就贴黑桃10以下最小牌；如果已出牌中有分，跟出最大牌； 
 b. 如果不是方片花色第一轮，当前已出牌没有分，则出较大牌；当前已出牌有分，则出最大牌 
   跟出草花 
 a. 草花花色第一轮出牌且已出牌中没有分，跟出最小牌，如果绝门，就贴方片10