# Blackjack Genetic Algorithm

This is a genetic algorithm which can generate a strategy for playing blackjack based on the first 2 cards dealt to the player as well as the dealer's face card.
If the algorithm runs for long enough it should eventually converge upon the basic strategy that many people use.

Just run the executable file to see it work - coloured output is present although this only works in Windows terminal.

# Example output

Games are simulated with 4 decks, dealer hits on soft 17 and there is no insurance or surrender. Blackjack payout is 3:2.

![image](https://user-images.githubusercontent.com/53403691/88113485-c5671480-cba9-11ea-9751-17ee5815c8b1.png)

The number at the top is the percentage gain/loss of each strategy. It can vary between runs but increasing the number of hands simulated will reduce this effect.

At the start of each row is each starting total or combination of cards which the player could receive and they are grouped in blocks - the top block is hard totals, the middle is soft totals (one Ace), and the bottom block is pairs.
The columns correspond to the card which the dealer has on show.

Key:

- Green (+)  - Hit
- Red (0)    - Stick
- Blue (x)   - Double
- Yellow (=) - Split

# Genetic Algorithm Default Settings

These can be changed within the source code if desired.

- Population size: 1000
- Elitism rate: 0.15
- Mutation rate: 0.005
- Number of hands played: 100000

Fitness is checked by simulating many hands of blackjack (100000 by default). After generation 50, this number is increased to 500000 in order to reduce variation so that more refined changes to the strategy can be made.

Tournament selection is used, with a tournament size of 4.

Crossover is a simple uniform crossover. For each cell in the strategy grid of a child, the action is one of the parent's actions, chosen with equal probability.

Each cell of a child strategy has a small chance to be mutated which assigns it a random action.

Elite individuals are propogated directly to the next generation without mutation.
