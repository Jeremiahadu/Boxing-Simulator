
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <random>
#include <set>
#include <time.h>
#include <vector>

using namespace std;

int weightClasses[17] = {
    105, 108, 112, 115, 118, 122, 126, 130, 135,
    140, 147, 154, 160, 168, 175, 200, 201  // 201 represents heavyweight
};

struct Record
{
    int wins = 0;
    int loses = 0;
    int knockouts = 0;
};

class Boxer
{
public:
    Boxer() {}
    Boxer(int weightClass, int wt, int ID, int abilitySc) :
        wtClass(weightClass),
        weight(wt),
        id(ID),
        abilityScore(abilitySc)
    {
        record.wins = 0;
        record.loses = 0;
        record.knockouts = 0;
    }
    int id;
    int abilityScore;
    Record record;
    int wtClass;
    int weight;
};

class Gym
{
public:
    Gym(int Name, int numBoxers):name(Name), numberOfBoxers(numBoxers)
    {
        boxers.resize(numberOfBoxers);
        set<int> wtClasses;
        srand(time(0));

        std::random_device rd;
        std::mt19937 e2(rd());

        normal_distribution<double> distributionAbility(100, 15);
        for (int i = 0; i < numberOfBoxers; ++i)
        {
            int cl = 0;
            int wt = 0;
            while (wtClasses.find(cl) != wtClasses.end() || cl < 1)
            {
                wt = rand() % 97 + 105;  // 105 to 201 lbs
                for (int j = 1; j < 17; ++j)
                {
                    if (wt > weightClasses[j - 1] && wt <= weightClasses[j])
                    {
                        cl = j;
                        break;
                    }
                }
            }
            int abilityScore = max(static_cast<int>(round<int>(distributionAbility(e2))), 0);
            boxers[i] = Boxer(cl, wt, name + i + 1, abilityScore);
            wtClasses.insert(cl);
        }
    };

    int getBoxer(int wtClass)
    {
        for (int i = 0; i < numberOfBoxers; ++i)
            if (boxers[i].wtClass == wtClass)
                return i;
        return -1;
    }

    float totalGymScore()
    {
        int wins = 0;
        int loses = 0;
        for (int i = 0; i < numberOfBoxers; ++i)
        {
            wins += boxers[i].record.wins;
            loses += boxers[i].record.loses;
        }

        return float(wins) / (wins + loses);
    }

    int name;
    int numberOfBoxers;
    vector<Boxer> boxers;
};

class BoxingLeague
{
public:
    BoxingLeague()
    {
        std::random_device rd;
        std::mt19937 e2(rd());

        for (int gymName = 100; gymName < 900; gymName += 100)
        {
            int numberOfBoxers = 16;  // One boxer per weight class
            gyms.push_back(Gym(gymName, numberOfBoxers));
        }

        for (int i = 0; i < 17; ++i)
            for (int j = 0; j < 8; ++j)
                for (int k = 0; k < 8; ++k)
                    advantageMatrix[i][j][k] = 0;
    }

    void simulateSeason()
    {
        for (int i = 0; i < 8; ++i)
            for (int j = i + 1; j < 8; ++j)
                gymsMatch(i, j);
    }

    int bestGym()
    {
        int bestGym = -1;
        for (int i = 0; i < 8; ++i)
        {
            if (gyms[i].totalGymScore() > bestScore)
            {
                bestScore = gyms[i].totalGymScore();
                bestGym = (i + 1) * 100;
            }
        }

        return bestGym;
    }

    int boxersMatch(int boxer1, int boxer2)
    {
        if (boxer1 % 100 < 1 || boxer2 % 100 < 1)
            return 0;
        Boxer& b1 = gyms[boxer1 / 100 - 1].boxers[boxer1 % 100 - 1];
        Boxer& b2 = gyms[boxer2 / 100 - 1].boxers[boxer2 % 100 - 1];

        float sigma = max(abs(b1.abilityScore - b2.abilityScore) / 3.f, 15.f);

        std::random_device rd;
        std::mt19937 e2(rd());

        std::normal_distribution<double> distribution1(b1.abilityScore, sigma);
        std::normal_distribution<double> distribution2(b2.abilityScore, sigma);
        float score1 = distribution1(e2);
        float score2 = distribution2(e2);

        if (score1 > score2)
        {
            b1.record.wins++;
            b2.record.loses++;
            if (score1 - score2 > 30) // Knockout
                b1.record.knockouts++;
            advantageMatrix[b1.wtClass][boxer1 / 100 - 1][boxer2 / 100 - 1]++;
            advantageMatrix[b1.wtClass][boxer2 / 100 - 1][boxer1 / 100 - 1]--;
            return 1;
        }
        else
        {
            b1.record.loses++;
            b2.record.wins++;
            if (score2 - score1 > 30) // Knockout
                b2.record.knockouts++;
            advantageMatrix[b1.wtClass][boxer1 / 100 - 1][boxer2 / 100 - 1]--;
            advantageMatrix[b1.wtClass][boxer2 / 100 - 1][boxer1 / 100 - 1]++;
            return -1;
        }
    }

    void gymsMatch(int gym1, int gym2)
    {
        for (int i = 1; i < 17; ++i)
        {
            int b1 = 100 * (gym1 + 1) + gyms[gym1].getBoxer(i) + 1;
            int b2 = 100 * (gym2 + 1) + gyms[gym2].getBoxer(i) + 1;
            boxersMatch(b1, b2);
        }
    }

    vector<Gym> gyms;
    float bestScore;
    int advantageMatrix[17][8][8];
};

struct Comp {
    Comp(BoxingLeague& league) { boxingLeague = league; }
    bool operator()(Boxer b1, Boxer b2)
    {
        if (b1.record.wins < b2.record.wins)
            return true;
        if (b2.record.wins < b1.record.wins)
            return false;

        if (boxingLeague.advantageMatrix[b1.wtClass][b1.id / 100][b2.id / 100] < 0)
            return true;
        else
            return false;
    }
    BoxingLeague boxingLeague;
};

class Match
{
public:
    Match(Boxer& b1, Boxer& b2) : boxer1(b1), boxer2(b2)
    {
        float sigma = max(abs(boxer1.abilityScore - boxer2.abilityScore) / 3.f, 15.f);

        std::random_device rd;
        std::mt19937 e2(rd());

        std::normal_distribution<double> distribution1(boxer1.abilityScore, sigma);
        std::normal_distribution<double> distribution2(boxer2.abilityScore, sigma);
        float score1 = distribution1(e2);
        float score2 = distribution2(e2);
        if (score1 > score2)
        {
            winner = boxer1;
            loser = boxer2;
            isKnockout = (score1 - score2 > 30);
        }
        else
        {
            winner = boxer2;
            loser = boxer1;
            isKnockout = (score2 - score1 > 30);
        }
    }
    Boxer boxer1;
    Boxer boxer2;
    Boxer winner;
    Boxer loser;
    bool isKnockout;
};

class Tournament
{
public:
    Tournament(BoxingLeague& league, int weightClass): boxingLeague(&league), wtClass(weightClass)
    {
        for (int i = 0; i < 8; ++i)
        {
            int id = (*boxingLeague).gyms[i].getBoxer(weightClass);
            if (id > -1)
                participants.push_back((*boxingLeague).gyms[i].boxers[id % 100]);
        }
        seed();
    }

    void seed()
    {
        Comp comp(*boxingLeague);
        sort(participants.begin(), participants.end(), comp);
    }

    void simulate()
    {
        matches.push_back(Match(participants[0], participants[7]));
        matches.push_back(Match(participants[4], participants[3]));
        matches.push_back(Match(participants[2], participants[5]));
        matches.push_back(Match(participants[1], participants[6]));
        matches.push_back(Match(matches[0].winner, matches[1].winner));
        matches.push_back(Match(matches[2].winner, matches[3].winner));
        matches.push_back(Match(matches[0].loser, matches[1].loser));
        matches.push_back(Match(matches[2].loser, matches[3].loser));
        matches.push_back(Match(matches[4].loser, matches[7].winner));
        matches.push_back(Match(matches[5].loser, matches[6].winner));
        matches.push_back(Match(matches[6].loser, matches[7].loser));
        matches.push_back(Match(matches[8].loser, matches[9].loser));
        matches.push_back(Match(matches[9].winner, matches[10].winner));
        matches.push_back(Match(matches[5].winner, matches[6].winner));
    }

    void printWinner()
    {
        cout << "And the winner in the weight class " << weightClasses[wtClass] << " lbs is boxer number " << matches[13].winner.id;
        if (matches[13].isKnockout)
            cout << " by knockout!";
        cout << endl;
    }

    vector<Boxer> participants;
    vector<Match> matches;
    BoxingLeague* boxingLeague;
    int wtClass;
};

int main()
{
    BoxingLeague league;
    for (int i = 0; i < 2; ++i)
        league.simulateSeason();

    cout << "League winner: the gym of " << league.bestGym() << " with the score of ";
    cout.precision(3);
    cout << league.bestScore << "! Congrats!" << endl;

    for (int i = 1; i < 17; ++i)
    {
        Tournament tournament(league, i);
        tournament.simulate();
        tournament.printWinner();
    }
    return 0;
}