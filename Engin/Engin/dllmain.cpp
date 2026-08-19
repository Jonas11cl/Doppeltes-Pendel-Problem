#include "Engine.h"
#include <iostream>
#include <cmath>
#include <cstdio>
#include <random>
#include <cmath>
#include <unordered_map>
double RandomZahl(double Anfang, double Ende)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<double> distrib(Anfang, Ende);
    return distrib(gen);
}
class HidenNode1
{
public:
    double Gewichte[8];
    double Bias;
    HidenNode1()
        : Bias(RandomZahl(-1.0, 1.0))
    {
        for (int i = 0; i < 8; ++i)
        {
            Gewichte[i] = RandomZahl(-1.0, 1.0);
        }
    }
};
class HidenNode2
{
public:
    double Gewichte[26];
    double Bias;
    HidenNode2()
        : Bias(RandomZahl(-1.0, 1.0))
    {
        for (int i = 0; i < 26; ++i)
        {
            Gewichte[i] = RandomZahl(-1.0, 1.0);
        }
    }
};
bool Training = true;
class NuralNetwork
{
public:
    HidenNode1 HidelLayer_1[26];
    HidenNode2 HidenLayer_2[12];
    double Bias;
    double Fitness;
    double Summe_1[26];
    double Summe_2[12];
    double MittelGewcihte[26];
    double Endgewichte[12];

    NuralNetwork()
        : Bias(RandomZahl(-1.0, 1.0)), Fitness(0.0)
    {
        for (int i = 0; i < 12; ++i)
        {
            Endgewichte[i] = RandomZahl(-1.0, 1.0);
        }
        for (int i = 0; i < 26; ++i)
        {
            MittelGewcihte[i] = RandomZahl(-1.0, 1.0);
        }
    }

    double Denken(double WagenX, double WagenTempo, double winkel1, double winkelTempo1, double winkel2, double winkelTempo2)
    {
        double Endsumme = 0;
        double SinWinkel_1 = sin(winkel1);
        double SinWinkel_2 = sin(winkel2);
        double CosWinkel_1 = cos(winkel1);
        double CosWinkel_2 = cos(winkel2);
        winkelTempo1 = winkelTempo1 * 0.1;
        winkelTempo2 = winkelTempo2 * 0.1;
        for (int i = 0; i < 26; i++)
        {
            double Summe = (WagenX * HidelLayer_1[i].Gewichte[0]) + (WagenTempo * HidelLayer_1[i].Gewichte[1]) + (SinWinkel_1 * HidelLayer_1[i].Gewichte[2]) + (winkelTempo1 * HidelLayer_1[i].Gewichte[3]) + (SinWinkel_2 * HidelLayer_1[i].Gewichte[4]) + (winkelTempo2 * HidelLayer_1[i].Gewichte[5]) + (CosWinkel_1 * HidelLayer_1[i].Gewichte[6]) + (CosWinkel_2 * HidelLayer_1[i].Gewichte[7]) + HidelLayer_1[i].Bias;
            Summe = std::tanh(Summe);
            this->Summe_1[i] = Summe;
        }
        for (int i = 0; i < 12; i++)
        {
            double AkkSumme = 0;
            for (int e = 0; e < 26; e++)
            {
                AkkSumme += Summe_1[e] * HidenLayer_2[i].Gewichte[e];
            }
            AkkSumme = std::tanh(AkkSumme);
            Summe_2[i] = AkkSumme;
        }
        for (int i = 0; i < 12; i++)
        {
            Endsumme += Summe_2[i] * Endgewichte[i];
        }

        Endsumme = 10.0 * std::tanh(Endsumme);
        return Endsumme;
    }
    void Motiren()
    {
        double NeueKiWarscheilichkeit = 0.1;
        double Mb = RandomZahl(0.0, 1.0);
        if (Mb < NeueKiWarscheilichkeit)
        {
            *this = NuralNetwork();
            return;
        }

        double MuttationsWarscheilichkeit = 0.05;
        for (int i = 0; i < 26; i++)
        {
            for (int e = 0; e < 8; e++)
            {
                double RandZahl = RandomZahl(0.0, 1.0);
                if (RandZahl < MuttationsWarscheilichkeit)
                {
                    double Muttatiosrate = RandomZahl(-0.05, 0.05);
                    HidelLayer_1[i].Gewichte[e] += Muttatiosrate;
                }
            }
            double RandZahl = RandomZahl(0.0, 1.0);
            if (RandZahl < MuttationsWarscheilichkeit)
            {
                double Muttatiosrate = RandomZahl(-0.05, 0.05);
                HidelLayer_1[i].Bias += Muttatiosrate;
            }
        }
        for (int i = 0; i < 12; i++)
        {
            for (int e = 0; e < 26; e++)
            {
                double RandZahl = RandomZahl(0.0, 1.0);
                if (RandZahl < MuttationsWarscheilichkeit)
                {
                    double Muttatiosrate = RandomZahl(-0.05, 0.05);
                    HidenLayer_2[i].Gewichte[e] += Muttatiosrate;
                }
            }
            double RandZahl = RandomZahl(0.0, 1.0);
            if (RandZahl < MuttationsWarscheilichkeit)
            {
                double Muttatiosrate = RandomZahl(-0.05, 0.05);
                HidenLayer_2[i].Bias += Muttatiosrate;
            }
        }
        for (int i = 0; i < 12; i++)
        {
            double RandZahl = RandomZahl(0.0, 1.0);
            if (RandZahl < MuttationsWarscheilichkeit)
            {
                double Muttatiosrate = RandomZahl(-0.05, 0.05);
                Endgewichte[i] += Muttatiosrate;
            }
        }
        double RandZahl = RandomZahl(0.0, 1.0);
        if (RandZahl < MuttationsWarscheilichkeit)
        {
            double Muttatiosrate = RandomZahl(-0.05, 0.05);
            Bias += Muttatiosrate;
        }
    }
};

NuralNetwork Ki[750];
NuralNetwork BesteKi[5];
int main()
{
    for (int i = 0; i < 750; i++)
    {
        Ki[i] = NuralNetwork();
    }
    bool trainingFertig = false;
    for (int gen = 0; gen < 2000; gen++)
    {
        double besteFitness = -INFINITY;
        int besterIndex = 0;
        int MeistenUpdates = -1;
#pragma omp parallel for
        for (int Kig = 0; Kig < 750; Kig++)
        {
            Ki[Kig].Fitness = 0.0;
            // for (int Spiel = 0; Spiel < 2; Spiel++)
            {
                MeineKlasse lokalesSpiel;
                int Updates = 0;
                lokalesSpiel.PhysikZuruecksetzen(gen, 0);

                while (lokalesSpiel.spielvorbei != true)
                {
                    double kraft = Ki[Kig].Denken(lokalesSpiel.Wagen_Position, lokalesSpiel.Wagen_Geschwindigkeit, lokalesSpiel.Winkel_Stange_1, lokalesSpiel.Winkel_Geschwindigkeit_1, lokalesSpiel.Winkel_Stange_2, lokalesSpiel.Winkel_Geschwindigkeit_2);
                    Ki[Kig].Fitness += lokalesSpiel.Update(kraft, 0.01);
                    Updates++;
                    if (Updates > 7200)
                    {
                        break;
                    }
                }
                if (Updates > MeistenUpdates)
                {
                    MeistenUpdates = Updates;
                }
            }
        }

        std::vector<std::pair<int, double>> ListeKis;
        ListeKis.reserve(750);
        for (int e = 0; e < 750; e++)
        {
            ListeKis.emplace_back(e, Ki[e].Fitness);
        }
        std::sort(ListeKis.begin(), ListeKis.end(), [](const auto &a, const auto &b)
                  { return a.second > b.second; });

        if (ListeKis.size() > 5)
        {
            ListeKis.resize(5);
        }
        for (int e = 0; e < 5; e++)
        {
            int index = ListeKis[e].first;
            BesteKi[e] = Ki[index];
        }

        std::cout << "Generation " << gen << " - Beste Fitness: " << BesteKi[0].Fitness << " - Updates Überlebet:" << MeistenUpdates << std::endl;

        for (int e = 0; e < 750; e++)
        {
            for (int f = 0; f < 5; f++)
            {
                Ki[f] = BesteKi[f];
            }

            if (e >= 5)
            {
                int Rand = RandomZahl(0, 4);

                Ki[e] = BesteKi[Rand];
                Ki[e].Motiren();
            }
        }
    }
    return 0;
}

extern "C" __declspec(dllexport) void StarteTraining()
{
    for (int i = 0; i < 750; i++)
    {
        Ki[i] = NuralNetwork();
    }
    bool trainingFertig = false;
    for (int gen = 0; gen < 2000; gen++)
    {
        double besteFitness = -INFINITY;
        int besterIndex = 0;
        int MeistenUpdates = -1;
#pragma omp parallel for
        for (int Kig = 0; Kig < 750; Kig++)
        {
            Ki[Kig].Fitness = 0.0;
            // for (int Spiel = 0; Spiel < 2; Spiel++)
            {
                MeineKlasse lokalesSpiel;
                int Updates = 0;
                lokalesSpiel.PhysikZuruecksetzen(gen, 0);

                while (lokalesSpiel.spielvorbei != true)
                {
                    double kraft = Ki[Kig].Denken(lokalesSpiel.Wagen_Position, lokalesSpiel.Wagen_Geschwindigkeit, lokalesSpiel.Winkel_Stange_1, lokalesSpiel.Winkel_Geschwindigkeit_1, lokalesSpiel.Winkel_Stange_2, lokalesSpiel.Winkel_Geschwindigkeit_2);
                    Ki[Kig].Fitness += lokalesSpiel.Update(kraft, 0.01);
                    Updates++;
                    if (Updates > 7200)
                    {
                        break;
                    }
                }
                if (Updates > MeistenUpdates)
                {
                    MeistenUpdates = Updates;
                }
            }
        }

        std::vector<std::pair<int, double>> ListeKis;
        ListeKis.reserve(750);
        for (int e = 0; e < 750; e++)
        {
            ListeKis.emplace_back(e, Ki[e].Fitness);
        }
        std::sort(ListeKis.begin(), ListeKis.end(), [](const auto &a, const auto &b)
                  { return a.second > b.second; });

        if (ListeKis.size() > 5)
        {
            ListeKis.resize(5);
        }
        for (int e = 0; e < 5; e++)
        {
            int index = ListeKis[e].first;
            BesteKi[e] = Ki[index];
        }

        std::cout << "Generation " << gen << " - Beste Fitness: " << BesteKi[0].Fitness << " - Updates Überlebet:" << MeistenUpdates << std::endl;

        for (int e = 0; e < 750; e++)
        {
            for (int f = 0; f < 5; f++)
            {
                Ki[f] = BesteKi[f];
            }

            if (e >= 5)
            {
                int Rand = RandomZahl(0, 4);

                Ki[e] = BesteKi[Rand];
                Ki[e].Motiren();
            }
        }
    }
}
MeineKlasse lokalesSpiel;

extern "C" __declspec(dllexport) void ResetPhysik()
{
    lokalesSpiel.PhysikZuruecksetzen(10000, 0);
}
extern "C" __declspec(dllexport) void MachEinenLiveSchritt()
{
    double Krfat = BesteKi[0].Denken(lokalesSpiel.Wagen_Position, lokalesSpiel.Wagen_Geschwindigkeit, lokalesSpiel.Winkel_Stange_1, lokalesSpiel.Winkel_Geschwindigkeit_1, lokalesSpiel.Winkel_Stange_2, lokalesSpiel.Winkel_Geschwindigkeit_2);
    lokalesSpiel.Update(Krfat, 0.01);
}
extern "C" __declspec(dllexport) double HoleWagenPosition()
{
    return lokalesSpiel.Wagen_Position;
}
extern "C" __declspec(dllexport) double HoleWinkel1()
{
    return lokalesSpiel.Winkel_Stange_1;
}
extern "C" __declspec(dllexport) double HoleWinkel2()
{
    return lokalesSpiel.Winkel_Stange_2;
}