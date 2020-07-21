#include <iostream>
#include <string>
#include <random>
#include <algorithm>
#include <windows.h>
#include <time.h>
#include <thread>

using namespace std;

//gene: {hit?,stand?,double?,split?}
//order: total more significant than dealer card
//dealer card goes A23456789T

const int popsize=1000;
const float elitism=0.15;
const float mutation=0.005;
int trials=100000;

const int shoesize=4; //keep as 4 ALWAYS!!!

int cards[]={1,2,3,4,5,6,7,8,9,10,10,10,10};

const int decklen=52*shoesize;

HANDLE hConsole;

random_device rd;
seed_seq seed{rd(),rd(),rd(),rd(),rd(),rd(),rd(),rd()};
mt19937 rng(seed);

int colours[]={160,192,176,224};

class Chromosome {
public:
    int _hard[160][4]={};
    int _soft[80][4]={};
    int _pairs[90][4]={};
    int _acepairs[10][4]={};

    double bet=1.;

    double _fitness=0;

    void populate();
    void display();
    int score(int hand[], int len);
    bool issoft(int hand[], int len);

    double test(int runs);
    double test2(int runs);
};

double Chromosome::test(int runs) {

    int deck[decklen];
    int counter=0;

    for (int i=0;i<decklen;i++)
    {
        deck[i]=cards[i%13];
    }
    shuffle(begin(deck),end(deck),rng);

    double money=0;
    int totalhands=0;

    const int handlen=18;
    int dpos=0;
    int dealerscore=0;
    int dealercard=0;
    bool dealerace=false;
    bool dealerblackjack=false;

    int hands[16*shoesize][handlen]={};
    int hpos[16*shoesize]={};
    int card=0;

    int hand=0;
    int handscore=0;
    int action[4]={};
    bool isacepair=false;
    int handcounter=1;
    int temp=0;

    int dealer[handlen]={};

    for (int run=0; run<runs; run++) {
        if (counter>(decklen/2)) {
            shuffle(begin(deck),end(deck),rng);
            counter=0;
        }

        for (int i=0;i<dpos;i++)
        {
            dealer[i]=0;
        }
        dpos=0;
        dealerscore=0;
        dealerace=false;
        dealerblackjack=false;

        while (dealerscore<17) {
            card=deck[counter];
            counter=(counter+1)%decklen;
            dealer[dpos]=card;
            dpos=dpos+1;
            dealerscore+=card;
            if (card==1) {dealerace=true;}
            if (dealerace && dealerscore>7 && dealerscore<12)
            {
                break;
            }
        }
        if (dealerace && dealerscore<12) {dealerscore+=10;}
        dealercard=dealer[0];
        if (dealerace && dealerscore==21 && dpos==2)
        {
            dealerblackjack=true;
        }

        //get first 2 cards.
        for (int i=0; i<2; i++) {
            card=deck[counter];
            counter=(counter+1)%decklen;
            hands[0][i]=card;
        }
        hpos[0]=2;

        //check for blackjack (player or dealer).
        if (score(hands[0],hpos[0])==21) {
            if (dealerscore!=21 || dpos!=2) {
                money=money+bet*1.5;
            }
            totalhands=totalhands+1;
            hands[0][0]=0;
            hands[0][1]=0;
            hpos[0]=0;
            continue;
        }

        //identify hand and play move.

        hand=0;
        handcounter=1;

        while (handcounter!=0) {
            isacepair=false;
            //find next non-empty hand.
            if (hands[hand][0]==0) {
                for (int i=0; i<16*shoesize; i++) {
                    if (hands[i][0]!=0) {
                        hand=i;
                        break;
                    }
                }
            }
            handscore=score(hands[hand],hpos[hand]);
            //identify hand.
            if (hands[hand][2]==0 && hands[hand][0]==hands[hand][1]) {
                //pair.
                if (hands[hand][0]==1) {
                    //acepair.
                    isacepair=true;
                    for (int i=0; i<4; i++) {
                        action[i]=_acepairs[dealercard-1][i];
                    }
                }
                else {
                    for (int i=0; i<4; i++) {
                        action[i]=_pairs[(handscore-4)*5+dealercard-1][i];
                    }
                }
            }
            else if (issoft(hands[hand],hpos[hand])==true) {
                //_soft.
                for (int i=0; i<4; i++) {
                    action[i]=_soft[(handscore-13)*10+dealercard-1][i];
                }
            }
            else {
                //_hard.
                for (int i=0; i<4; i++) {
                    action[i]=_hard[(handscore-5)*10+dealercard-1][i];
                }
            }
            //identify the action and do it.
            if (action[0]==1) {
                //hit.
                //card=getcard(deck,decklen);
                card=deck[counter];
                counter=(counter+1)%decklen;
                hands[hand][hpos[hand]]=card;
                hpos[hand]=hpos[hand]+1;
                handscore=score(hands[hand],hpos[hand]);

                if (handscore>21) {
                    money=money-bet;

                    //reset hand slot.
                    for (int i=0;i<hpos[hand];i++)
                    {
                        hands[hand][i]=0;
                    }
                    totalhands=totalhands+1;
                    hpos[hand]=0;
                    handcounter-=1;
                    continue;
                }
                else if (handscore==21)
                {
                    if (dealerblackjack)
                    {
                        money-=bet;
                    }
                    else if (!dealerblackjack && dealerscore!=21)
                    {
                        money+=bet;
                    }

                    for (int i=0;i<hpos[hand];i++)
                    {
                        hands[hand][i]=0;
                    }
                    totalhands+=1;
                    hpos[hand]=0;
                    handcounter-=1;
                    continue;
                }
                else {
                    continue;
                }
            }
            else if (action[1]==1) {
                //stand.
                if (dealerblackjack)
                {
                    money-=bet;
                    totalhands+=1;
                    for (int i=0;i<hpos[hand];i++)
                    {
                        hands[hand][i]=0;
                    }
                    hpos[hand]=0;
                    handcounter-=1;
                    continue;
                }

                if (dealerscore<22) {
                    if (dealerscore<handscore) {
                        money=money+bet;
                    }
                    else if (dealerscore>handscore) {
                        money=money-bet;
                    }
                }
                else if (dealerscore>21) {
                    money=money+bet;
                }
                totalhands=totalhands+1;

                //reset hand slot.
                for (int i=0;i<hpos[hand];i++)
                {
                    hands[hand][i]=0;
                }
                hpos[hand]=0;
                handcounter-=1;
                continue;
            }
            else if (action[2]==1) {
                //double.
                //card=getcard(deck,decklen);
                card=deck[counter];
                counter=(counter+1)%decklen;
                hands[hand][hpos[hand]]=card;
                hpos[hand]=hpos[hand]+1;
                handscore=score(hands[hand],hpos[hand]);

                if (dealerblackjack)
                {
                    money-=2.*bet;
                    totalhands+=1;
                    for (int i=0;i<hpos[hand];i++)
                    {
                        hands[hand][i]=0;
                    }
                    hpos[hand]=0;
                    handcounter-=1;
                    continue;
                }

                if (handscore>21)
                {
                    money-=2.*bet;
                }
                else if (handscore<22 && dealerscore>21)
                {
                    money+=2.*bet;
                }
                else if (handscore<22 && dealerscore<22)
                {
                    if (handscore<dealerscore)
                    {
                        money-=2.*bet;
                    }
                    else if (handscore>dealerscore)
                    {
                        money+=2.*bet;
                    }
                }

                totalhands=totalhands+1;

                //reset hand slot.
                for (int i=0;i<hpos[hand];i++)
                {
                    hands[hand][i]=0;
                }
                hpos[hand]=0;
                handcounter-=1;
                continue;
            }
            else if (action[3]==1) {
                //split.
                if (isacepair==true) {
                    //split aces.
                    for (int i=0; i<2; i++) {
                        //card=getcard(deck,decklen);
                        if (dealerblackjack)
                        {
                            money-=bet;
                            totalhands+=1;
                            continue;
                        }
                        card=deck[counter];
                        counter=(counter+1)%decklen;
                        if (dealerscore<22) {
                            if (dealerscore<card+11) {
                                money=money+bet;
                            }
                            else if (dealerscore>card+11) {
                                money=money-bet;
                            }
                        }
                        else if (dealerscore>21) {
                            money=money+bet;
                        }
                        totalhands=totalhands+1;
                    }

                    //reset hand slot.
                    for (int i=0;i<hpos[hand];i++)
                    {
                        hands[hand][i]=0;
                    }
                    hpos[hand]=0;
                    handcounter-=1;
                    continue;
                }
                else {
                    //non-ace pair.
                    //card=getcard(deck,decklen);
                    temp=hands[hand][0];
                    card=deck[counter];
                    counter=(counter+1)%decklen;
                    hands[hand][1]=card;
                    handscore=score(hands[hand],hpos[hand]);
                    if (handscore==21)
                    {
                        if (dealerblackjack)
                        {
                            money-=bet;
                        }
                        else if (!dealerblackjack && dealerscore!=21)
                        {
                            money+=bet;
                        }

                        totalhands+=1;
                        for (int i=0;i<hpos[hand];i++)
                        {
                            hands[hand][i]=0;
                        }
                        hpos[hand]=0;
                        handcounter-=1;
                    }


                    //find next empty hand.
                    card=temp;
                    for (int i=0; i<16*shoesize; i++) {
                        if (hands[i][0]==0) {
                            hand=i;
                            break;
                        }
                    }
                    hands[hand][0]=card;
                    card=deck[counter];
                    counter=(counter+1)%decklen;
                    hands[hand][1]=card;
                    hpos[hand]=2;
                    handcounter+=1;
                    handscore=score(hands[hand],hpos[hand]);
                    if (handscore==21)
                    {
                        if (dealerblackjack)
                        {
                            money-=bet;
                        }
                        else if (!dealerblackjack && dealerscore!=21)
                        {
                            money+=bet;
                        }

                        totalhands+=1;
                        for (int i=0;i<hpos[hand];i++)
                        {
                            hands[hand][i]=0;
                        }
                        hpos[hand]=0;
                        handcounter-=1;
                    }
                    continue;
                }
            }
        }
    }
    return 100*money/totalhands;
}

void Chromosome::populate() {
    int a[]={1,0,0};
    for (int i=0; i<160; i++) {
        //shuffle_array(a,3);
        shuffle(begin(a),end(a),rng);
        _hard[i][0]=a[0];
        _hard[i][1]=a[1];
        _hard[i][2]=a[2];
        _hard[i][3]=0;
    }
    for (int i=0; i<80; i++) {
        //shuffle_array(a,3);
        shuffle(begin(a),end(a),rng);
        _soft[i][0]=a[0];
        _soft[i][1]=a[1];
        _soft[i][2]=a[2];
        _soft[i][3]=0;
    }
    int b[]={1,0,0,0};
    for (int i=0; i<90; i++) {
        //shuffle_array(b,4);
        shuffle(begin(b),end(b),rng);
        _pairs[i][0]=b[0];
        _pairs[i][1]=b[1];
        _pairs[i][2]=b[2];
        _pairs[i][3]=b[3];
    }
    for (int i=0; i<10; i++) {
        //shuffle_array(b,4);
        shuffle(begin(b),end(b),rng);
        _acepairs[i][0]=b[0];
        _acepairs[i][1]=b[1];
        _acepairs[i][2]=b[2];
        _acepairs[i][3]=b[3];
    }
    return;
}

void Chromosome::display() {
    //hit +
    //stick 0
    //double x
    //split =
    char actions[]={'+','0','x','='};
    string header="[  ][2][3][4][5][6][7][8][9][T][A]";
    //_hard hands.
    SetConsoleTextAttribute(hConsole,15);
    cout << header << endl;
    string sidehard="05060708091011121314151617181920";
    for (int i=20; i>4; i=i-1) {
        SetConsoleTextAttribute(hConsole,15);
        cout << "[" << sidehard[(i-5)*2] << sidehard[(i-5)*2+1] << "]";
        for (int j=(i-5)*10+1; j<(i-5)*10+10; j++) {
            if (_hard[j][0]==1) {
                SetConsoleTextAttribute(hConsole,colours[0]);
                cout << "[" << actions[0] << "]";
            }
            else if (_hard[j][1]==1) {
                SetConsoleTextAttribute(hConsole,colours[1]);
                cout << "[" << actions[1] << "]";
            }
            else if (_hard[j][2]==1) {
                SetConsoleTextAttribute(hConsole,colours[2]);
                cout << "[" << actions[2] << "]";
            }
            else if (_hard[j][3]==1) {
                SetConsoleTextAttribute(hConsole,colours[3]);
                cout << "[" << actions[3] << "]";
            }
        }
        if (_hard[(i-5)*10][0]==1) {
                SetConsoleTextAttribute(hConsole,colours[0]);
                cout << "[" << actions[0] << "]";
            }
            else if (_hard[(i-5)*10][1]==1) {
                SetConsoleTextAttribute(hConsole,colours[1]);
                cout << "[" << actions[1] << "]";
            }
            else if (_hard[(i-5)*10][2]==1) {
                SetConsoleTextAttribute(hConsole,colours[2]);
                cout << "[" << actions[2] << "]";
            }
            else if (_hard[(i-5)*10][3]==1) {
                SetConsoleTextAttribute(hConsole,colours[3]);
                cout << "[" << actions[3] << "]";
            }
        cout << endl;
    }
    cout << endl;
    //_soft hands.
    SetConsoleTextAttribute(hConsole,15);
    cout << header << endl;
    string sidesoft="A2A3A4A5A6A7A8A9";
    for (int i=7; i>=0; i=i-1) {
        SetConsoleTextAttribute(hConsole,15);
        cout << "[" << sidesoft[2*i] << sidesoft[2*i+1] << "]";
        for (int j=i*10+1; j<i*10+10; j++) {
            if (_soft[j][0]==1) {
                SetConsoleTextAttribute(hConsole,colours[0]);
                cout << "[" << actions[0] << "]";
            }
            else if (_soft[j][1]==1) {
                SetConsoleTextAttribute(hConsole,colours[1]);
                cout << "[" << actions[1] << "]";
            }
            else if (_soft[j][2]==1) {
                SetConsoleTextAttribute(hConsole,colours[2]);
                cout << "[" << actions[2] << "]";
            }
            else if (_soft[j][3]==1) {
                SetConsoleTextAttribute(hConsole,colours[3]);
                cout << "[" << actions[3] << "]";
            }
        }
        if (_soft[i*10][0]==1) {
                SetConsoleTextAttribute(hConsole,colours[0]);
                cout << "[" << actions[0] << "]";
            }
            else if (_soft[i*10][1]==1) {
                SetConsoleTextAttribute(hConsole,colours[1]);
                cout << "[" << actions[1] << "]";
            }
            else if (_soft[i*10][2]==1) {
                SetConsoleTextAttribute(hConsole,colours[2]);
                cout << "[" << actions[2] << "]";
            }
            else if (_soft[i*10][3]==1) {
                SetConsoleTextAttribute(hConsole,colours[3]);
                cout << "[" << actions[3] << "]";
            }
        cout << endl;
    }
    cout << endl;
    //pair hands.
    SetConsoleTextAttribute(hConsole,15);
    cout << header << endl;
    cout << "[AA]";
    for (int j=1; j<10; j++) {
            if (_acepairs[j][0]==1) {
                SetConsoleTextAttribute(hConsole,colours[0]);
                cout << "[" << actions[0] << "]";
            }
            else if (_acepairs[j][1]==1) {
                SetConsoleTextAttribute(hConsole,colours[1]);
                cout << "[" << actions[1] << "]";
            }
            else if (_acepairs[j][2]==1) {
                SetConsoleTextAttribute(hConsole,colours[2]);
                cout << "[" << actions[2] << "]";
            }
            else if (_acepairs[j][3]==1) {
                SetConsoleTextAttribute(hConsole,colours[3]);
                cout << "[" << actions[3] << "]";
            }
    }
    if (_acepairs[0][0]==1) {
            SetConsoleTextAttribute(hConsole,colours[0]);
            cout << "[" << actions[0] << "]";
        }
        else if (_acepairs[0][1]==1) {
            SetConsoleTextAttribute(hConsole,colours[1]);
            cout << "[" << actions[1] << "]";
        }
        else if (_acepairs[0][2]==1) {
            SetConsoleTextAttribute(hConsole,colours[2]);
            cout << "[" << actions[2] << "]";
        }
        else if (_acepairs[0][3]==1) {
            SetConsoleTextAttribute(hConsole,colours[3]);
            cout << "[" << actions[3] << "]";
        }
    cout << endl;

    string sidepairs="2233445566778899TT";
    for (int i=8; i>=0; i=i-1) {
        SetConsoleTextAttribute(hConsole,15);
        cout << "[" << sidepairs[2*i] << sidepairs[2*i+1] << "]";
        for (int j=i*10+1; j<i*10+10; j++) {
            if (_pairs[j][0]==1) {
                SetConsoleTextAttribute(hConsole,colours[0]);
                cout << "[" << actions[0] << "]";
            }
            else if (_pairs[j][1]==1) {
                SetConsoleTextAttribute(hConsole,colours[1]);
                cout << "[" << actions[1] << "]";
            }
            else if (_pairs[j][2]==1) {
                SetConsoleTextAttribute(hConsole,colours[2]);
                cout << "[" << actions[2] << "]";
            }
            else if (_pairs[j][3]==1) {
                SetConsoleTextAttribute(hConsole,colours[3]);
                cout << "[" << actions[3] << "]";
            }
        }
        if (_pairs[i*10][0]==1) {
                SetConsoleTextAttribute(hConsole,colours[0]);
                cout << "[" << actions[0] << "]";
            }
            else if (_pairs[i*10][1]==1) {
                SetConsoleTextAttribute(hConsole,colours[1]);
                cout << "[" << actions[1] << "]";
            }
            else if (_pairs[i*10][2]==1) {
                SetConsoleTextAttribute(hConsole,colours[2]);
                cout << "[" << actions[2] << "]";
            }
            else if (_pairs[i*10][3]==1) {
                SetConsoleTextAttribute(hConsole,colours[3]);
                cout << "[" << actions[3] << "]";
            }
        cout << endl;
    }
    cout << endl;

    SetConsoleTextAttribute(hConsole,15);

    return;
}

int Chromosome::score(int hand[], int len) {
    int total=0;
    bool ace=false;

    for (int i=0; i<len; i++) {
        total=total+hand[i];
        if (hand[i]==1) {
            ace=true;
        }
    }
    if (ace==true && total<12) {
        total=total+10;
    }
    return total;
}

bool Chromosome::issoft(int hand[], int len) {
    int total=0;
    bool ace=false;
    bool soft=false;

    for (int i=0; i<len; i++) {
        total=total+hand[i];
        if (hand[i]==1) {
            ace=true;
        }
    }
    if (ace==true && total<12) {
        soft=true;
    }
    return soft;
}

bool comp(const Chromosome& lhs, const Chromosome& rhs) {
    return lhs._fitness<rhs._fitness;
}

Chromosome pop[popsize];
Chromosome newpop[popsize];
Chromosome topstrat;

void transfer(int x) {
    for (int i=0; i<160; i++) {
        pop[x]._hard[i][0]=newpop[x]._hard[i][0];
        pop[x]._hard[i][1]=newpop[x]._hard[i][1];
        pop[x]._hard[i][2]=newpop[x]._hard[i][2];
        pop[x]._hard[i][3]=newpop[x]._hard[i][3];
    }
    for (int i=0; i<80; i++) {
        pop[x]._soft[i][0]=newpop[x]._soft[i][0];
        pop[x]._soft[i][1]=newpop[x]._soft[i][1];
        pop[x]._soft[i][2]=newpop[x]._soft[i][2];
        pop[x]._soft[i][3]=newpop[x]._soft[i][3];
    }
    for (int i=0; i<90; i++) {
        pop[x]._pairs[i][0]=newpop[x]._pairs[i][0];
        pop[x]._pairs[i][1]=newpop[x]._pairs[i][1];
        pop[x]._pairs[i][2]=newpop[x]._pairs[i][2];
        pop[x]._pairs[i][3]=newpop[x]._pairs[i][3];
    }
    for (int i=0; i<10; i++) {
        pop[x]._acepairs[i][0]=newpop[x]._acepairs[i][0];
        pop[x]._acepairs[i][1]=newpop[x]._acepairs[i][1];
        pop[x]._acepairs[i][2]=newpop[x]._acepairs[i][2];
        pop[x]._acepairs[i][3]=newpop[x]._acepairs[i][3];
    }
    pop[x]._fitness=0;
}

void transferbest() {
    for (int i=0; i<160; i++) {
        topstrat._hard[i][0]=pop[popsize-1]._hard[i][0];
        topstrat._hard[i][1]=pop[popsize-1]._hard[i][1];
        topstrat._hard[i][2]=pop[popsize-1]._hard[i][2];
        topstrat._hard[i][3]=pop[popsize-1]._hard[i][3];
    }
    for (int i=0; i<80; i++) {
        topstrat._soft[i][0]=pop[popsize-1]._soft[i][0];
        topstrat._soft[i][1]=pop[popsize-1]._soft[i][1];
        topstrat._soft[i][2]=pop[popsize-1]._soft[i][2];
        topstrat._soft[i][3]=pop[popsize-1]._soft[i][3];
    }
    for (int i=0; i<90; i++) {
        topstrat._pairs[i][0]=pop[popsize-1]._pairs[i][0];
        topstrat._pairs[i][1]=pop[popsize-1]._pairs[i][1];
        topstrat._pairs[i][2]=pop[popsize-1]._pairs[i][2];
        topstrat._pairs[i][3]=pop[popsize-1]._pairs[i][3];
    }
    for (int i=0; i<10; i++) {
        topstrat._acepairs[i][0]=pop[popsize-1]._acepairs[i][0];
        topstrat._acepairs[i][1]=pop[popsize-1]._acepairs[i][1];
        topstrat._acepairs[i][2]=pop[popsize-1]._acepairs[i][2];
        topstrat._acepairs[i][3]=pop[popsize-1]._acepairs[i][3];
    }
    topstrat._fitness=pop[popsize-1]._fitness;
}

void makebest(int i) {
    for (int j=0;j<160;j++)
    {
        pop[i]._hard[j][0]=0;
        pop[i]._hard[j][1]=0;
        pop[i]._hard[j][2]=0;
        pop[i]._hard[j][3]=0;
    }
    for (int j=0;j<80;j++)
    {
        pop[i]._soft[j][0]=0;
        pop[i]._soft[j][1]=0;
        pop[i]._soft[j][2]=0;
        pop[i]._soft[j][3]=0;
    }
    for (int j=0;j<90;j++)
    {
        pop[i]._pairs[j][0]=0;
        pop[i]._pairs[j][1]=0;
        pop[i]._pairs[j][2]=0;
        pop[i]._pairs[j][3]=0;
    }
    for (int j=0;j<10;j++)
    {
        pop[i]._acepairs[j][0]=0;
        pop[i]._acepairs[j][1]=0;
        pop[i]._acepairs[j][2]=0;
        pop[i]._acepairs[j][3]=0;
    }

    for (int j=0; j<50; j++) {
        pop[i]._hard[j][0]=1;
    }
    for (int j=120; j<160; j++) {
        pop[i]._hard[j][1]=1;
    }
    for (int j=80; j<120; j++) {
        if (0<(j%10) && (j%10)<6) {
            pop[i]._hard[j][1]=1;
        }
        else {
            pop[i]._hard[j][0]=1;
        }
    }

    pop[i]._hard[70][0]=1;
    pop[i]._hard[71][0]=1;
    pop[i]._hard[72][0]=1;
    pop[i]._hard[73][1]=1;
    pop[i]._hard[74][1]=1;
    pop[i]._hard[75][1]=1;
    pop[i]._hard[76][0]=1;
    pop[i]._hard[77][0]=1;
    pop[i]._hard[78][0]=1;
    pop[i]._hard[79][0]=1;

    for (int j=0;j<20;j++)
    {
        pop[i]._hard[j+50][2]=1;
    }
    pop[i]._hard[50][2]=0;
    pop[i]._hard[50][0]=1;
    pop[i]._hard[59][2]=0;
    pop[i]._hard[59][0]=1;

    pop[i]._hard[42][0]=0;
    pop[i]._hard[42][2]=1;
    pop[i]._hard[43][0]=0;
    pop[i]._hard[43][2]=1;
    pop[i]._hard[44][0]=0;
    pop[i]._hard[44][2]=1;
    pop[i]._hard[45][0]=0;
    pop[i]._hard[45][2]=1;

    for (int j=0;j<20;j++)
    {
        pop[i]._soft[60+j][1]=1;
    }
    pop[i]._soft[65][1]=0;
    pop[i]._soft[65][2]=1;

    for (int j=0;j<60;j++)
    {
        pop[i]._soft[j][0]=1;
    }

    int thing[]={51,52,53,54,55,42,43,44,45,33,34,35,23,24,25,14,15,4,5};
    for (int j=0;j<19;j++)
    {
        pop[i]._soft[thing[j]][0]=0;
        pop[i]._soft[thing[j]][2]=1;
    }

    pop[i]._soft[56][0]=0;
    pop[i]._soft[56][1]=1;
    pop[i]._soft[57][0]=0;
    pop[i]._soft[57][1]=1;

    for (int j=0;j<10;j++)
    {
        pop[i]._acepairs[j][3]=1;
        pop[i]._pairs[j+80][1]=1;
    }
    for (int j=0;j<80;j++)
    {
        pop[i]._pairs[j][3]=1;
    }
    pop[i]._pairs[70][3]=0;
    pop[i]._pairs[70][1]=1;
    pop[i]._pairs[79][3]=0;
    pop[i]._pairs[79][1]=1;
    pop[i]._pairs[76][3]=0;
    pop[i]._pairs[76][1]=1;

    for (int j=31;j<39;j++)
    {
        pop[i]._pairs[j][3]=0;
        pop[i]._pairs[j][2]=1;
    }

    int things2[]={50,57,58,59,40,46,47,48,49,30,39,20,21,22,23,26,27,28,29,10,19,18,17,0,9,8,7};
    for (int j=0;j<27;j++)
    {
        pop[i]._pairs[things2[j]][3]=0;
        pop[i]._pairs[things2[j]][0]=1;
    }
}

void getfitness(int a,int b,bool show_progress)
{
    int progress=0;
    for (int i=a;i<b;i++)
    {
        pop[i]._fitness=pop[i].test(trials);
        if (show_progress && (i-a)>(b-a)*progress/10)
        {
            progress+=1;
            cout << "+";
        }
    }
    if (show_progress)
    {
        cout << endl;
    }
}

int main()
{
    hConsole=GetStdHandle(STD_OUTPUT_HANDLE);

    int elitenum=std::floor(popsize*elitism);

    uniform_int_distribution<int> unibreed(0,1);
    uniform_real_distribution<double> unimut(0.,1.);

    int popindex[popsize]={};
    //initialise population.
    for (int i=0; i<popsize; i++) {
        pop[i].populate();
        popindex[i]=i;
    }

    int gen=0;
    int totalgen=1000;
    int ad=0;
    int r=0;
    int p[2];
    int tournysize=4;
    double besttournyfitness=-999999.;
    int tournyindex=0;

    double m=0.;
    int a[3]={1,0,0};
    int b[4]={1,0,0,0};

    //best strategy.
    int topgen=0;
    topstrat._fitness=-100000000;

    //multithreading.
    const int threadcount=4;

    array<thread,threadcount> threads;
    array<int,threadcount+1> indices;

    indices[0]=0;
    indices[threadcount]=popsize;

    for (int i=1;i<threadcount;i++)
    {
        indices[i]=int(popsize*i/threadcount);
    }

    while (gen<totalgen) {
        gen=gen+1;
        if (gen>50)
        {
            trials=500000;
        }

        SetConsoleTextAttribute(hConsole,15);

        cout << "Generation: " << gen << endl;

        for (int i=0;i<threadcount-1;i++)
        {
            threads[i]=thread(getfitness,indices[i],indices[i+1],false);
        }
        threads[threadcount-1]=thread(getfitness,indices[threadcount-1],indices[threadcount],true);
        for (int i=0;i<threadcount;i++)
        {
            threads[i].join();
        }

        //sort pop in terms of _fitness.
        std::sort(pop,pop+popsize,comp);

        //show the best of the generation.
        cout << pop[popsize-1]._fitness << endl;

        pop[popsize-1].display();

        if (pop[popsize-1]._fitness>topstrat._fitness) {
            topgen=gen;
            transferbest();
            //topstrat.display();
            SetConsoleTextAttribute(hConsole,15);
        }

        //select individuals for crossover.
        ad=0;
        shuffle(begin(popindex),end(popindex),rng);
        for (int i=0; i<popsize-elitenum; i++)
        {
            for (int j=0; j<2; j++)
            {
                //tournament selection.
                besttournyfitness=-9999999.;
                if (ad+tournysize>=popsize)
                {
                    ad=0;
                    shuffle(begin(popindex),end(popindex),rng);
                }
                for (int k=ad;k<ad+tournysize;k++)
                {
                    if (pop[popindex[k]]._fitness>besttournyfitness)
                    {
                        besttournyfitness=pop[popindex[k]]._fitness;
                        tournyindex=popindex[k];
                    }
                }
                ad+=tournysize;
                p[j]=tournyindex;
            }
            //encode child and introduce mutations.
            for (int j=0; j<160; j++) {
                r=unibreed(rng);
                newpop[i]._hard[j][0]=pop[p[r]]._hard[j][0];
                newpop[i]._hard[j][1]=pop[p[r]]._hard[j][1];
                newpop[i]._hard[j][2]=pop[p[r]]._hard[j][2];
                newpop[i]._hard[j][3]=pop[p[r]]._hard[j][3];

                m=unimut(rng);
                if (m<mutation) {
                    //shuffle_array(a,3);
                    shuffle(begin(a),end(a),rng);
                    newpop[i]._hard[j][0]=a[0];
                    newpop[i]._hard[j][1]=a[1];
                    newpop[i]._hard[j][2]=a[2];
                    newpop[i]._hard[j][3]=0;
                }
            }
            for (int j=0; j<80; j++) {
                r=unibreed(rng);
                newpop[i]._soft[j][0]=pop[p[r]]._soft[j][0];
                newpop[i]._soft[j][1]=pop[p[r]]._soft[j][1];
                newpop[i]._soft[j][2]=pop[p[r]]._soft[j][2];
                newpop[i]._soft[j][3]=pop[p[r]]._soft[j][3];

                m=unimut(rng);
                if (m<mutation) {
                    //shuffle_array(a,3);
                    shuffle(begin(a),end(a),rng);
                    newpop[i]._soft[j][0]=a[0];
                    newpop[i]._soft[j][1]=a[1];
                    newpop[i]._soft[j][2]=a[2];
                    newpop[i]._soft[j][3]=0;
                }
            }
            for (int j=0; j<90; j++) {
                r=unibreed(rng);
                newpop[i]._pairs[j][0]=pop[p[r]]._pairs[j][0];
                newpop[i]._pairs[j][1]=pop[p[r]]._pairs[j][1];
                newpop[i]._pairs[j][2]=pop[p[r]]._pairs[j][2];
                newpop[i]._pairs[j][3]=pop[p[r]]._pairs[j][3];

                m=unimut(rng);
                if (m<mutation) {
                    //shuffle_array(b,4);
                    shuffle(begin(b),end(b),rng);
                    newpop[i]._pairs[j][0]=b[0];
                    newpop[i]._pairs[j][1]=b[1];
                    newpop[i]._pairs[j][2]=b[2];
                    newpop[i]._pairs[j][3]=b[3];
                }
            }
            for (int j=0; j<10; j++) {
                r=unibreed(rng);
                newpop[i]._acepairs[j][0]=pop[p[r]]._acepairs[j][0];
                newpop[i]._acepairs[j][1]=pop[p[r]]._acepairs[j][1];
                newpop[i]._acepairs[j][2]=pop[p[r]]._acepairs[j][2];
                newpop[i]._acepairs[j][3]=pop[p[r]]._acepairs[j][3];

                m=unimut(rng);
                if (m<mutation) {
                    //shuffle_array(b,4);
                    shuffle(begin(b),end(b),rng);
                    newpop[i]._acepairs[j][0]=b[0];
                    newpop[i]._acepairs[j][1]=b[1];
                    newpop[i]._acepairs[j][2]=b[2];
                    newpop[i]._acepairs[j][3]=b[3];
                }
            }
        }
        //replace old generation with new.
        for (int i=0; i<popsize-elitenum; i++) {
            transfer(i);
        }
    }
    return 0;
}
