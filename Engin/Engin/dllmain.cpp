#include "Engine.h"
#include <iostream>
#include <cmath>
#include <cstdio>
#include <random>
#include <cmath>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <deque>
double RandomZahl(double Anfang, double Ende)
{
    static std::random_device rd;
    thread_local static std::mt19937 gen(rd());
    std::uniform_real_distribution<double> distrib(Anfang, Ende);
    return distrib(gen);
}
class RetrunKi
{
public:
    double Gewichte_1[8];
    double Gewichte_2[26];
    double Gewichte_3[12];
    double Bias1;
    double Bias2;
    double Bias3;
};
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
    double Endgewichte[12];

    NuralNetwork()
        : Bias(RandomZahl(-1.0, 1.0)), Fitness(0.0)
    {
        for (int i = 0; i < 12; ++i)
        {
            Endgewichte[i] = RandomZahl(-1.0, 1.0);
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
            AkkSumme += HidenLayer_2[i].Bias;
            AkkSumme = std::tanh(AkkSumme);
            Summe_2[i] = AkkSumme;
        }
        for (int i = 0; i < 12; i++)
        {
            Endsumme += Summe_2[i] * Endgewichte[i];
        }
        Endsumme += Bias;

        Endsumme = 10.0 * std::tanh(Endsumme);
        return Endsumme;
    }
    void Motiren(std::deque<double> BesteFitness, double bestfitness, std::deque<double> UberlebtSpiel1, std::deque<double> UberlebtSpiel2)
    {
        double MuttationsWarscheilichkeit = 0.05;
        double RadMax = 0.05;
        double RadMin = -0.05;
        if (sizeof(BesteFitness) >= 50)
        {
            if (sizeof(UberlebtSpiel1) >= 50 && sizeof(UberlebtSpiel1) >= 50)
            {
                double UberlbesArray1[10];
                double UberlbesArray2[10];

                auto StadrdpunktS1 = UberlebtSpiel1.end() - 10;
                auto StadrdpunktS2 = UberlebtSpiel1.end() - 10;
                std::copy(StadrdpunktS1, UberlebtSpiel1.end(), UberlbesArray1);
                std::copy(StadrdpunktS2, UberlebtSpiel2.end(), UberlbesArray2);
                double Rundungsarry[10];
                auto Stadtpunkt = BesteFitness.end() - 10;
                std::copy(Stadtpunkt, BesteFitness.end(), Rundungsarry);
                double Zahl = 0;
                double ZahlS1 = 0;
                double ZahlS2 = 0;
                for (int i = 0; i < 10; i++)
                {
                    Zahl += Rundungsarry[i];
                    ZahlS1 += UberlbesArray1[i];
                    ZahlS2 += UberlbesArray2[i];
                }
                double ZahlZ = (ZahlS1 + ZahlS2) / 2;
                Zahl = Zahl / 10;
                if (Zahl + 100 > bestfitness && ZahlZ < 6500)
                {
                    MuttationsWarscheilichkeit = 0.15;
                    RadMax = 0.15;
                    RadMin = -0.15;
                }
                else if (ZahlZ > 7200)
                {
                    MuttationsWarscheilichkeit = 0.01;
                    RadMax = 0.01;
                    RadMin = -0.01;
                }
            }
        }
        double NeueKiWarscheilichkeit = 0.1;
        double Mb = RandomZahl(0.0, 1.0);
        if (Mb < NeueKiWarscheilichkeit)
        {
            *this = NuralNetwork();
            return;
        }

        for (int i = 0; i < 26; i++)
        {
            for (int e = 0; e < 8; e++)
            {
                double RandZahl = RandomZahl(0.0, 1.0);
                if (RandZahl < MuttationsWarscheilichkeit)
                {
                    double Muttatiosrate = RandomZahl(RadMin, RadMax);
                    HidelLayer_1[i].Gewichte[e] += Muttatiosrate;
                }
            }
            double RandZahl = RandomZahl(0.0, 1.0);
            if (RandZahl < MuttationsWarscheilichkeit)
            {
                double Muttatiosrate = RandomZahl(RadMin, RadMax);
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
                    double Muttatiosrate = RandomZahl(RadMin, RadMax);
                    HidenLayer_2[i].Gewichte[e] += Muttatiosrate;
                }
            }
            double RandZahl = RandomZahl(0.0, 1.0);
            if (RandZahl < MuttationsWarscheilichkeit)
            {
                double Muttatiosrate = RandomZahl(RadMin, RadMax);
                HidenLayer_2[i].Bias += Muttatiosrate;
            }
        }
        for (int i = 0; i < 12; i++)
        {
            double RandZahl = RandomZahl(0.0, 1.0);
            if (RandZahl < MuttationsWarscheilichkeit)
            {
                double Muttatiosrate = RandomZahl(RadMin, RadMax);
                Endgewichte[i] += Muttatiosrate;
            }
        }
        double RandZahl = RandomZahl(0.0, 1.0);
        if (RandZahl < MuttationsWarscheilichkeit)
        {
            double Muttatiosrate = RandomZahl(RadMin, RadMax);
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
    std::deque<double> BesteFitnness;
    std::deque<double> Spiel_1_UL;
    std::deque<double> Spiel_2_UL;
    for (int gen = 0; gen < 7500; gen++)
    {

        double Winkel_Stange_1_0 = RandomZahl(-0.08, 0.08);
        double Winkel_Stange_2_0 = RandomZahl(-0.08, 0.08);
        double Winkel_Stange_1_1;
        double Winkel_Stange_2_1;
        if (gen < 500)
        {

            Winkel_Stange_1_1 = RandomZahl(-0.8, 0.8);
            Winkel_Stange_2_1 = RandomZahl(-0.8, 0.8);
        }
        else if (gen < 1200)
        {

            Winkel_Stange_1_1 = 1.57 + RandomZahl(-0.15, 0.15);
            Winkel_Stange_2_1 = 1.57 + RandomZahl(-0.15, 0.15);
        }
        else
        {

            Winkel_Stange_1_1 = 3.14159 + RandomZahl(-0.1, 0.1);
            Winkel_Stange_2_1 = 3.14159 + RandomZahl(-0.1, 0.1);
        }

        double besteFitness = -INFINITY;
        int besterIndex = 0;
        int MeistenUpdates[2] = {-1, -1};
#pragma omp parallel for
        for (int Kig = 0; Kig < 750; Kig++)
        {
            Ki[Kig].Fitness = 0.0;
            for (int Spiel = 0; Spiel < 2; Spiel++)
            {
                MeineKlasse lokalesSpiel;
                int Updates = 0;
                if (Spiel == 0)
                {
                    lokalesSpiel.PhysikZuruecksetzen(gen, Winkel_Stange_1_0, Winkel_Stange_2_0);
                }
                else
                {
                    lokalesSpiel.PhysikZuruecksetzen(gen, Winkel_Stange_1_1, Winkel_Stange_2_1);
                }

                while (lokalesSpiel.spielvorbei != true)
                {
                    double kraft = Ki[Kig].Denken(lokalesSpiel.Wagen_Position, lokalesSpiel.Wagen_Geschwindigkeit, lokalesSpiel.Winkel_Stange_1, lokalesSpiel.Winkel_Geschwindigkeit_1, lokalesSpiel.Winkel_Stange_2, lokalesSpiel.Winkel_Geschwindigkeit_2);
                    Ki[Kig].Fitness += lokalesSpiel.Update(kraft, 0.01);
                    Updates++;
                    if (Updates > 7200)
                    {
                        break;
                    }
                    if (Updates > 1500 && lokalesSpiel.waroben == false)
                    {
                        break;
                    }
                }
                if (Updates > MeistenUpdates[Spiel])
                {
                    MeistenUpdates[Spiel] = Updates;
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

        std::cout << "Generation " << gen << " - Beste Fitness: " << BesteKi[0].Fitness << " - Updates Uberlebet Spiel 1:" << MeistenUpdates[0] << " - Updates Uberlebet Spiel 2:" << MeistenUpdates[1] << std::endl;
        if (BesteFitnness.size() >= 50)
        {
            BesteFitnness.pop_back();
        }
        BesteFitnness.push_front(BesteKi[0].Fitness);
        if (Spiel_1_UL.size() >= 50)
        {
            Spiel_1_UL.pop_back();
        }
        Spiel_1_UL.push_front(MeistenUpdates[0]);

        if (Spiel_2_UL.size() >= 50)
        {
            Spiel_2_UL.pop_back();
        }
        Spiel_2_UL.push_front(MeistenUpdates[1]);

        if (gen == 500 || gen == 1200)
        {
            BesteFitnness.clear();
            Spiel_1_UL.clear();
            Spiel_2_UL.clear();
        }
        for (int f = 0; f < 5; f++)
        {
            Ki[f] = BesteKi[f];
        }
        for (int e = 0; e < 750; e++)
        {
            if (e >= 5)
            {
                int Rand = RandomZahl(0, 4);

                Ki[e] = BesteKi[Rand];
                Ki[e].Motiren(BesteFitnness, BesteKi[0].Fitness, Spiel_1_UL, Spiel_2_UL);
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
    std::deque<double> BesteFitnness;
    std::deque<double> Spiel_1_UL;
    std::deque<double> Spiel_2_UL;
    for (int gen = 0; gen < 7500; gen++)
    {

        double Winkel_Stange_1_0 = RandomZahl(-0.08, 0.08);
        double Winkel_Stange_2_0 = RandomZahl(-0.08, 0.08);
        double Winkel_Stange_1_1;
        double Winkel_Stange_2_1;
        if (gen < 500)
        {

            Winkel_Stange_1_1 = RandomZahl(-0.8, 0.8);
            Winkel_Stange_2_1 = RandomZahl(-0.8, 0.8);
        }
        else if (gen < 1200)
        {

            Winkel_Stange_1_1 = 1.57 + RandomZahl(-0.15, 0.15);
            Winkel_Stange_2_1 = 1.57 + RandomZahl(-0.15, 0.15);
        }
        else
        {

            Winkel_Stange_1_1 = 3.14159 + RandomZahl(-0.1, 0.1);
            Winkel_Stange_2_1 = 3.14159 + RandomZahl(-0.1, 0.1);
        }

        double besteFitness = -INFINITY;
        int besterIndex = 0;
        int MeistenUpdates[2] = {-1, -1};
#pragma omp parallel for
        for (int Kig = 0; Kig < 750; Kig++)
        {
            Ki[Kig].Fitness = 0.0;
            for (int Spiel = 0; Spiel < 2; Spiel++)
            {
                MeineKlasse lokalesSpiel;
                int Updates = 0;
                if (Spiel == 0)
                {
                    lokalesSpiel.PhysikZuruecksetzen(gen, Winkel_Stange_1_0, Winkel_Stange_2_0);
                }
                else
                {
                    lokalesSpiel.PhysikZuruecksetzen(gen, Winkel_Stange_1_1, Winkel_Stange_2_1);
                }

                while (lokalesSpiel.spielvorbei != true)
                {
                    double kraft = Ki[Kig].Denken(lokalesSpiel.Wagen_Position, lokalesSpiel.Wagen_Geschwindigkeit, lokalesSpiel.Winkel_Stange_1, lokalesSpiel.Winkel_Geschwindigkeit_1, lokalesSpiel.Winkel_Stange_2, lokalesSpiel.Winkel_Geschwindigkeit_2);
                    Ki[Kig].Fitness += lokalesSpiel.Update(kraft, 0.01);
                    Updates++;
                    if (Updates > 7200)
                    {
                        break;
                    }
                    if (Updates > 1500 && lokalesSpiel.waroben == false)
                    {
                        break;
                    }
                }
                if (Updates > MeistenUpdates[Spiel])
                {
                    MeistenUpdates[Spiel] = Updates;
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

        std::cout << "Generation " << gen << " - Beste Fitness: " << BesteKi[0].Fitness << " - Updates Uberlebet Spiel 1:" << MeistenUpdates[0] << " - Updates Uberlebet Spiel 2:" << MeistenUpdates[1] << std::endl;
        if (BesteFitnness.size() >= 50)
        {
            BesteFitnness.pop_back();
        }
        BesteFitnness.push_front(BesteKi[0].Fitness);
        if (Spiel_1_UL.size() >= 50)
        {
            Spiel_1_UL.pop_back();
        }
        Spiel_1_UL.push_front(MeistenUpdates[0]);

        if (Spiel_2_UL.size() >= 50)
        {
            Spiel_2_UL.pop_back();
        }
        Spiel_2_UL.push_front(MeistenUpdates[1]);

        if (gen == 500 || gen == 1200)
        {
            BesteFitnness.clear();
            Spiel_1_UL.clear();
            Spiel_2_UL.clear();
        }
        for (int f = 0; f < 5; f++)
        {
            Ki[f] = BesteKi[f];
        }
        for (int e = 0; e < 750; e++)
        {
            if (e >= 5)
            {
                int Rand = RandomZahl(0, 4);

                Ki[e] = BesteKi[Rand];
                Ki[e].Motiren(BesteFitnness, BesteKi[0].Fitness, Spiel_1_UL, Spiel_2_UL);
            }
        }
    }
}
MeineKlasse lokalesSpiel;

extern "C" __declspec(dllexport) void ResetPhysik()
{
    lokalesSpiel.PhysikZuruecksetzen(10000, 3.14159, 3.14159);
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

extern "C" __declspec(dllexport) RetrunKi HoleKi()
{
    RetrunKi KiAusgeben;
    for (int i = 0; i < 8; i++)
    {
        KiAusgeben.Gewichte_1[i] = BesteKi->HidelLayer_1->Gewichte[i];
    }
    for (int i = 0; i < 26; i++)
    {
        KiAusgeben.Gewichte_2[i] = BesteKi->HidenLayer_2->Gewichte[i];
    }
    for (int i = 0; i < 12; i++)
    {
        KiAusgeben.Gewichte_3[i] = BesteKi->Endgewichte[i];
    }
    KiAusgeben.Bias1 = BesteKi->HidelLayer_1->Bias;
    KiAusgeben.Bias2 = BesteKi->HidenLayer_2->Bias;
    KiAusgeben.Bias3 = BesteKi->Bias;
    return KiAusgeben;
}

extern "C" __declspec(dllexport) void GetKiC(RetrunKi NeueKi)
{
    BesteKi->HidelLayer_1->Bias = NeueKi.Bias1;
    BesteKi->HidenLayer_2->Bias = NeueKi.Bias2;
    BesteKi->Bias = NeueKi.Bias3;
    for (int i = 0; i < 8; i++)
    {
        BesteKi->HidelLayer_1->Gewichte[i] = NeueKi.Gewichte_1[i];
    }
    for (int i = 0; i < 26; i++)
    {
        BesteKi->HidenLayer_2->Gewichte[i] = NeueKi.Gewichte_2[i];
    }
    for (int i = 0; i < 12; i++)
    {
        BesteKi->Endgewichte[i] = NeueKi.Gewichte_3[i];
    }
}