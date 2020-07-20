# Blackjack Genetic Algorithm

This is a genetic algorithm which can generate a strategy for playing blackjack based on the first 2 cards dealt to the player as well as the dealer's face card.
If the algorithm runs for long enough it should eventually converge upon the basic strategy that many people use.

Just run the executable file to see it work - coloured output is present although this only works in Windows terminal.

# Example output

![image](https://user-images.githubusercontent.com/53403691/87956867-71740700-caa7-11ea-9161-31ee188ec27e.png)

The number is one hundred times the percentage gain/loss of each strategy. For this particular case, -1397.75 indicates that the player would expect to lose 13.9775% of their starting money.

At the start of each row is each starting total or combination of cards which the player could receive and they are grouped in blocks - the top block is hard totals, the middle is soft totals (one Ace), and the bottom block is pairs.
The columns correspond to the card which the dealer has on show.

Key:

- Green (+)  - Hit
- Red (0)    - Stick
- Blue (x)   - Double
- Yellow (=) - Split
