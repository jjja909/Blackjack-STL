#include <iostream>
#include <list>
#include <map>
#include <set>
#include <stack>
#include <queue>
#include <vector>
#include <algorithm>
#include <string>
#include <iterator>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <cstdlib>
//guh
using namespace std;






//Cards for everything
class Card {
private:
    string suit;   // hearts, diamonds, clubs, spades
    string face;   // 2-10, jack, queen, king, ace
    int    value;  // Point value of the card

public:
    // construct
    Card(string s, string f, int v)
        : suit(s), face(f), value(v) {}

    // access
    string getFace()  const { return face;  }
    string getSuit()  const { return suit;  }
    int    getValue() const { return value; }

    // blank of blank like "ace" of "spades"
    string toString() const {
        return face + " of " + suit;
    }

    // + value in [] like "King of Spades [10]
    void display() const {
        cout << "  " << left << setw(18)
             << (face + " of " + suit)
             << "[" << value << "]" << endl;
    }

    // equal 
    bool operator==(const Card& other) const {
        return face == other.face && suit == other.suit;
    }

    // less than
    bool operator<(const Card& other) const {
        return value < other.value;
    }
};


//  hand manages cards for either the player or the dealer
// list<Card> as the main container and set<string> to track cards seen in the round






class Hand {
private:
    list<Card>  cards;      // the hand stored as a list
    set<string> seenCards;  // set of card names seen this round

public:
    // add a card to the hand and record it as seen
    void addCard(const Card& c) {
        cards.push_back(c);
        seenCards.insert(c.toString());
    }

    // clear all cards from the hand and reset seen set
    void clear() {
        cards.clear();
        seenCards.clear();
    }

    // check for dupes
    bool hasSeen(const Card& c) const {
        return seenCards.count(c.toString()) > 0;
    }

    // return # of cards in hand
    int size() const {
        return (int)cards.size();
    }

    // calculate hand total and ace shtuff
    int getTotal() const {
        int total    = 0;
        int aceCount = 0;

        // use for_each with lambda to add all card values
        for_each(cards.begin(), cards.end(), [&](const Card& c) {
            total += c.getValue();
            if (c.getFace() == "Ace") {
                aceCount++;
            }
        });

        // we switch aces from 11 to 1 if bad
        while (total > 21 && aceCount > 0) {
            total    -= 10;
            aceCount--;
        }

        return total;
    }

    //sort hand order
    void sortHand() {
        cards.sort([](const Card& a, const Card& b) {
            return a.getValue() < b.getValue();
        });
    }

    //  check if a card with specific value is in hand using *find_if* and an iterator
    bool hasValue(int val) const {
        auto it = find_if(cards.begin(), cards.end(),
                          [val](const Card& c) {
                              return c.getValue() == val;
                          });
        return it != cards.end();
    }
    //ace counter
    int countAces() const {
        return (int)count_if(cards.begin(), cards.end(),
                             [](const Card& c) {
                                 return c.getFace() == "Ace";
                             });
    }

    void display() const {
        for_each(cards.begin(), cards.end(), [](const Card& c) {
            c.display();
        });
    }

    // display hand and total#
    void displayWithTotal() const {
        for_each(cards.begin(), cards.end(), [](const Card& c) {
            c.display();
        });
        cout << "  Total: " << getTotal() << endl;
    }

    // display hand, but hide bc dealer
    void displayHidden() const {
        bool first = true;
        for_each(cards.begin(), cards.end(), [&first](const Card& c) {
            if (first) {
                cout << "  [Hidden Card]" << endl;
                first = false;
            } else {
                c.display();
            }
        });
    }

// dealer show hidden card
    Card getFirstCard() const {
        return cards.front();
    }

// update a copy of the card list for l8r
    list<Card> getCards() const {
        return cards;
    }
};





//everything with the deck, make, deal, shuffle, queue discard everything
class Deck {
private:
    list<Card>      cards;       // full 52-card deck as a list
    queue<Card>     dealQueue;   // deal order queue FIFO
    stack<Card>     discardPile; // discard pile stack LIFO
    map<string,int> cardValues;  // face value into point value map

    //populate the map with values
    void initCardValues() {
        cardValues["2"]     = 2;
        cardValues["3"]     = 3;
        cardValues["4"]     = 4;
        cardValues["5"]     = 5;
        cardValues["6"]     = 6;
        cardValues["7"]     = 7;
        cardValues["8"]     = 8;
        cardValues["9"]     = 9;
        cardValues["10"]    = 10;
        cardValues["Jack"]  = 10;
        cardValues["Queen"] = 10;
        cardValues["King"]  = 10;
        cardValues["Ace"]   = 11;
    }

public:
    // constructor to initialize card values and build the deck
    Deck() {
        initCardValues();
        build();
    }

    // 52 cards made and assigned
    void build() {
        cards.clear();

        list<string> suits = {"Hearts", "Diamonds", "Clubs", "Spades"};
        list<string> faces = {"2","3","4","5","6","7","8","9","10",
                               "Jack","Queen","King","Ace"};

        //loop over suits and faces so all 52 cards r made
        for (const string& s : suits) {
            for (const string& f : faces) {
                cards.push_back(Card(s, f, cardValues[f]));
            }
        }
    }

    //  shuffle the deck. random_shuffle, needs random access iterators but list iterators rnt,
    //so i copy the list into a vector (sorry D:) shuffle, and then put them back
    void shuffle() {
        // copy list into vector for random_shuffle
        vector<Card> temp;
        copy(cards.begin(), cards.end(), back_inserter(temp));

        // shuffle
        random_shuffle(temp.begin(), temp.end());
        copy(temp.begin(), temp.end(), cards.begin()); //and put back

        //transform so we can get the values back so we can display
        list<string> faceNames;
        transform(cards.begin(), cards.end(),
                  back_inserter(faceNames),
                  [](const Card& c) { return c.getFace(); });

        // clear and requeue with shuffled
        while (!dealQueue.empty()) {
            dealQueue.pop();
        }
        for (const Card& c : cards) {
            dealQueue.push(c);
        }
    }

    //  deal cards from the front of the queue
    //if the queue runs out, rebuild and reshuffle discard
    Card deal() {
        if (dealQueue.empty()) {
            build();
            shuffle();
        }

        // take a card from the front
        Card c = dealQueue.front();
        dealQueue.pop();

        // then put it away 
        discardPile.push(c);

        return c;
    }

// searching for specific card values
    bool contains(const Card& target) const {
        auto it = find(cards.begin(), cards.end(), target);
        return it != cards.end();
    }

    // return # of cards remamining in queue
    int remaining() const {
        return (int)dealQueue.size();
    }

    // return # of cards in discard
    int discarded() const {
        return (int)discardPile.size();
    }
};






// everything to do with the player
//chips bets stats etc.
class Player {
private:
    string          name;
    int             chips;
    int             currentBet;
    Hand            hand;
    map<string,int> stats;  // win/loss/tie/chips stats

//stat map with all 0s to begin
    void initStats() {
        stats["wins"]       = 0;
        stats["losses"]     = 0;
        stats["ties"]       = 0;
        stats["blackjacks"] = 0;
        stats["chipsWon"]   = 0;
        stats["chipsLost"]  = 0;
    }

public:
    //constructor set name, starting chips, bet
    Player(string n, int startChips)
        : name(n), chips(startChips), currentBet(0) {
        initStats();
    }

    string getName()  const { return name;       }
    int    getChips() const { return chips;      }
    int    getBet()   const { return currentBet; }
    Hand&  getHand()        { return hand;       }

    // check if broke
    bool isOut() const {
        return chips <= 0;
    }

    // deduct bet
    void placeBet(int amount) {
        currentBet  = amount;
        chips      -= amount;
    }

    // add winnings
    void winBet(int amount) {
        chips += amount;
    }

    // return for tie
    void returnBet() {
        chips += currentBet;
    }

    // record winnings
    void recordWin(bool blackjack = false) {
        int winnings = blackjack ? (int)(currentBet * 1.5)
                                 : currentBet;
        chips += winnings;
        stats["wins"]++;
        stats["chipsWon"] += winnings;
        if (blackjack) {
            stats["blackjacks"]++;
        }
    }

    // update lose
    void recordLoss() {
        stats["losses"]++;
        stats["chipsLost"] += currentBet;
    }

    // update tie
    void recordTie() {
        stats["ties"]++;
        chips += currentBet;
    }

    // stat display
    void displayStats() const {
        cout << "\n--- ROUND STATS ---"                        << endl;
        cout << "Wins:          " << stats.at("wins")         << endl;
        cout << "Losses:        " << stats.at("losses")       << endl;
        cout << "Ties:          " << stats.at("ties")         << endl;
        cout << "Blackjacks:    " << stats.at("blackjacks")   << endl;
        cout << "Chips won:     " << stats.at("chipsWon")     << endl;
        cout << "Chips lost:    " << stats.at("chipsLost")    << endl;
        cout << "Current chips: " << chips                    << endl;
    }

    // final stat display when its all donezo
    void displayFinalStats() const {
        cout << "\n--- FINAL STATS ---"                      << endl;
        cout << "Wins:        " << stats.at("wins")          << endl;
        cout << "Losses:      " << stats.at("losses")        << endl;
        cout << "Ties:        " << stats.at("ties")          << endl;
        cout << "Blackjacks:  " << stats.at("blackjacks")    << endl;
        cout << "Chips won:   " << stats.at("chipsWon")      << endl;
        cout << "Chips lost:  " << stats.at("chipsLost")     << endl;
        cout << "Final chips: " << chips                     << endl;
    }
};







// everything dealer
class Dealer {
private:
    Hand hand;
    bool cardHidden; // true until player finishes their turn

public:
    Dealer() : cardHidden(true) {}

    Hand&       getHand()       { return hand; }
    const Hand& getHand() const { return hand; }

    // reset for new round
    void reset() {
        hand.clear();
        cardHidden = true;
    }

    //reveal
    void revealCard() {
        cardHidden = false;
    }

    bool isHiding() const {
        return cardHidden;
    }

    // display hand
    void display() const {
        cout << "\nDealer's hand:" << endl;
        if (cardHidden) {
            hand.displayHidden(); //keep hidden if it is
        } else {
            hand.displayWithTotal();
        }
    }

    // go til 17
    bool shouldHit() const {
        return hand.getTotal() < 17;
    }
};







// black jack
class Game {
private:
    Player* player; 
    Dealer  dealer;
    Deck    deck;

    void displayHeader() const {
        cout << "\n";
        cout << "  Blackjack" << endl;
        cout << endl;
    }

    // rules in case
    void displayRules() const {
        cout << "\n--- how to play ---"                              << endl;
        cout << "Your goal is to get as close to 21 as possible"             << endl;
        cout << "      without going over (busting)."              << endl;
        cout << ""                                                  << endl;
        cout << "Card values:"                                      << endl;
        cout << "  Number cards = face value"                       << endl;
        cout << "  Jack/Queen/King = 10"                            << endl;
        cout << "  Ace = 11 (switches to 1 to avoid bust)"         << endl;
        cout << ""                                                  << endl;
        cout << "Payouts:"                                          << endl;
        cout << "  Win = 1:1 on your bet"                          << endl;
        cout << "  Blackjack = 1.5x your bet"                      << endl;
        cout << "  Tie = bet returned"                              << endl;
        cout << "-------------------\n"                             << endl;
    }

    //check for a valid bet
    int getValidBet() const {
        int bet = 0;
        while (true) {
            cout << "Place your bet (1-" << player->getChips() << "): ";
            if (cin >> bet && bet > 0 && bet <= player->getChips()) {
                break;
            }
            cin.clear();
            cin.ignore(1000, '\n');
            cout << "Invalid bet. Enter a number between 1 and "
                 << player->getChips() << "." << endl;
        }
        return bet;
    }

    //getPlayerChoice
    int getPlayerChoice() const {
        int choice = 0;
        while (choice != 1 && choice != 2) {
            cout << "\nWhat would you like to do?" << endl;
            cout << "  1. Hit"                     << endl;
            cout << "  2. Stand"                   << endl;
            cout << "Enter choice: ";
            if (!(cin >> choice) || (choice != 1 && choice != 2)) {
                cin.clear();
                cin.ignore(1000, '\n');
                cout << "Please enter 1 or 2." << endl;
                choice = 0;
            }
        }
        return choice;
    }

   //deal cards
    void dealInitialCards() {
        player->getHand().addCard(deck.deal());
        dealer.getHand().addCard(deck.deal());
        player->getHand().addCard(deck.deal());
        dealer.getHand().addCard(deck.deal());
    }

// round logic
    void resolveRound() {
        int playerTotal = player->getHand().getTotal();
        int dealerTotal = dealer.getHand().getTotal();

        cout << "\nResults:" << endl;
        cout << "  Your total:     " << playerTotal << endl;
        cout << "  Dealer total:   " << dealerTotal << endl;

        if (dealerTotal > 21) {
            cout << "  Dealer busts! You win! +"
                 << player->getBet() << " chips" << endl;
            player->recordWin();
        } else if (playerTotal > dealerTotal) {
            cout << "  You win! +"
                 << player->getBet() << " chips" << endl;
            player->recordWin();
        } else if (dealerTotal > playerTotal) {
            cout << "  Dealer wins. -"
                 << player->getBet() << " chips" << endl;
            player->recordLoss();
        } else {
            cout << "  Push! It's a tie. Bet returned." << endl;
            player->recordTie();
        }
    }

    //manages one full round of the game
    void playRound() {
        cout << "\n--- New round ---" << endl;

        // reset first
        player->getHand().clear();
        dealer.reset();

        // reshuffle at 15 cards
        if (deck.remaining() < 15) {
            cout << "(Reshuffling deck...)" << endl;
            deck.build();
            deck.shuffle();
        }

        // get and place bet
        int bet = getValidBet();
        player->placeBet(bet);

        
        cout << "\nDealing cards..." << endl;
        dealInitialCards();

        // show hands
        cout << "\nYour hand:" << endl;
        player->getHand().displayWithTotal();
        dealer.display();

        // check for 21 insta
        if (player->getHand().getTotal() == 21 &&
            player->getHand().size() == 2) {
            cout << "\nBlackjack! You win 1.5x your bet. +"
                 << (int)(bet * 1.5) << " chips" << endl;
            player->recordWin(true);
            player->displayStats();
            return;
        }

        //get player turn
        bool playerDone = false;
        while (!playerDone) {
            int choice = getPlayerChoice();

            if (choice == 1) {
                player->getHand().addCard(deck.deal());
                cout << "\nYour hand:" << endl;
                player->getHand().displayWithTotal();

                if (player->getHand().getTotal() > 21) {
                    cout << "\nBust! You went over 21." << endl;
                    cout << "You lose. -" << bet << " chips" << endl;
                    player->recordLoss();
                    player->displayStats();
                    return;
                }

                // check for 21
                if (player->getHand().getTotal() == 21) {
                    cout << "21! Standing automatically." << endl;
                    playerDone = true;
                }

            } else {
                //STAND!!
                playerDone = true;
            }
        }

        cout << "\nDealer reveals hidden card..." << endl;
        dealer.revealCard();
        cout << "\nDealer's hand:" << endl;
        dealer.getHand().displayWithTotal();

        while (dealer.shouldHit()) {
            cout << "\nDealer hits..." << endl;
            dealer.getHand().addCard(deck.deal());
            cout << "\nDealer's hand:" << endl;
            dealer.getHand().displayWithTotal();
        }

        if (!dealer.shouldHit() && dealer.getHand().getTotal() <= 21) {
            cout << "\nDealer stands." << endl;
        }

        // round logic
        resolveRound();

        // show stats
        player->displayStats();
    }

    
    
    
    
public:
    Game() : player(nullptr) {}

    ~Game() {
        delete player;
    }
    //main loop
    void run() {
        displayHeader();

        //ask to show rules
        char showRules;
        cout << "Would you like to see the rules? (y/n): ";
        cin  >> showRules;
        if (showRules == 'y' || showRules == 'Y') {
            displayRules();
        }

        // ask for da name
        string name;
        cout << "Enter your name: ";
        cin  >> name;

        player = new Player(name, 500);
        cout << "Welcome, " << name << "!"  << endl;
        cout << "Starting chips: " << player->getChips() << endl;

        deck.shuffle();

        
        char playAgain = 'y';
        while ((playAgain == 'y' || playAgain == 'Y') && !player->isOut()) {
            playRound();

        
            if (player->isOut()) {
                cout << "\nYou have run out of chips. Game over!" << endl;
                break;
            }

            cout << "\nPlay another round? (y/n): ";
            cin  >> playAgain;
        }

   //final display
        player->displayFinalStats();
        cout << "\nThanks for playing, " << player->getName()
             << "!" << endl;
    }
};


//main, start end everything
int main() {
    srand((unsigned int)time(0)); // seed random number generator
    Game game;
    game.run();
    return 0;
}