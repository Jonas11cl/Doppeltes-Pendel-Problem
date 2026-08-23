#pragma once
#include "Engine.h"
#include <iostream>
#include <cmath>
#include <cstdio>
#include <random>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <deque>
#include <mutex>
#include <atomic>

struct TrainingsStatus
{
    int Generation;
    double BesteFitness;
    int UeberlebtSpiel1;
    int UeberlebtSpiel2;
};

int LimitUpdates = 7200;

TrainingsStatus aktuellerTrainingsStatus{0, 0.0, 0, 0};
std::mutex trainingsStatusMutex;

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
    double Gewichte_1[26][8];
    double Biases_1[26];
    double Gewichte_2[12][26];
    double Biases_2[12];
    double Gewichte_3[12];
    double Bias3;
};

class HidenNode1
{
public:
    double Gewichte[8];
    double Bias;
    HidenNode1() : Bias(RandomZahl(-1.0, 1.0))
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
    HidenNode2() : Bias(RandomZahl(-1.0, 1.0))
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

    int UpdatesS1 = 0;
    int UpdatesS2 = 0;

    double FitnessS1 = 0.0;
    double FitnessS2 = 0.0;

    double Summe_1[26];
    double Summe_2[12];
    double Endgewichte[12];

    NuralNetwork() : Bias(RandomZahl(-1.0, 1.0)), Fitness(0.0)
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

    void Motiren(const std::deque<double> &BesteFitness, double bestfitness, const std::deque<double> &UberlebtSpiel1, const std::deque<double> &UberlebtSpiel2)
    {
        double MuttationsWarscheilichkeit = 0.05;
        double RadMax = 0.05;
        double RadMin = -0.05;

        if (UberlebtSpiel1.size() >= 10 && UberlebtSpiel2.size() >= 10)
        {
            double ZahlS1 = 0, ZahlS2 = 0;
            for (int i = 0; i < 10; i++)
            {
                ZahlS1 += UberlebtSpiel1[i];
                ZahlS2 += UberlebtSpiel2[i];
            }

            ZahlS1 /= 10.0;
            ZahlS2 /= 10.0;

            double ZahlZ = (ZahlS1 + ZahlS2) / 2.0;

            if (ZahlZ >= LimitUpdates - 10)
            {
                return;
            }
            else if (ZahlZ >= LimitUpdates - 700)
            {
                MuttationsWarscheilichkeit = 0.01;
                RadMax = 0.01;
                RadMin = -0.01;
            }
        }

        if (MuttationsWarscheilichkeit == 0.05 && BesteFitness.size() == 50)
        {
            double alteFitness = 0;

            for (int i = 40; i < 50; i++)
            {
                alteFitness += BesteFitness[i];
            }
            alteFitness /= 10.0;

            if (alteFitness + 100 > bestfitness)
            {
                MuttationsWarscheilichkeit = 0.15;
                RadMax = 0.15;
                RadMin = -0.15;
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
                if (RandomZahl(0.0, 1.0) < MuttationsWarscheilichkeit)
                {
                    HidelLayer_1[i].Gewichte[e] += RandomZahl(RadMin, RadMax);
                }
            }
            if (RandomZahl(0.0, 1.0) < MuttationsWarscheilichkeit)
            {
                HidelLayer_1[i].Bias += RandomZahl(RadMin, RadMax);
            }
        }
        for (int i = 0; i < 12; i++)
        {
            for (int e = 0; e < 26; e++)
            {
                if (RandomZahl(0.0, 1.0) < MuttationsWarscheilichkeit)
                {
                    HidenLayer_2[i].Gewichte[e] += RandomZahl(RadMin, RadMax);
                }
            }
            if (RandomZahl(0.0, 1.0) < MuttationsWarscheilichkeit)
            {
                HidenLayer_2[i].Bias += RandomZahl(RadMin, RadMax);
            }
        }
        for (int i = 0; i < 12; i++)
        {
            if (RandomZahl(0.0, 1.0) < MuttationsWarscheilichkeit)
            {
                Endgewichte[i] += RandomZahl(RadMin, RadMax);
            }
        }
        if (RandomZahl(0.0, 1.0) < MuttationsWarscheilichkeit)
        {
            Bias += RandomZahl(RadMin, RadMax);
        }
    }
};

std::atomic<bool> training{false};
bool TrainingGestated = false;
NuralNetwork Ki[750];
NuralNetwork BesteKi[5];
MeineKlasse lokalesSpiel;

int gen = 0;
bool trainingFertig = false;
std::deque<double> BesteFitnness;
std::deque<double> Spiel_1_UL;
std::deque<double> Spiel_2_UL;

void KiGeneriren()
{
    for (int i = 0; i < 750; i++)
    {
        Ki[i] = NuralNetwork();
    }
}

extern "C" __declspec(dllexport) void StarteTraining()
{
    if (TrainingGestated == false)
    {
        TrainingGestated = true;
        KiGeneriren();
    }

    while (training == true)
    {
        gen++;
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

        // NEU: Damit Spiel 2 auch bei schweren Startwinkeln (~180°) eine faire Chance hat,
        // hochzuschwingen, bevor abgebrochen wird. Vorher fix 1500 -> jetzt an LimitUpdates gekoppelt,
        // aber nach oben gedeckelt, damit die Simulation nicht ewig läuft.
        int NichtObenLimitSpiel2 = std::min(4000, LimitUpdates / 2);

#pragma omp parallel for
        for (int Kig = 0; Kig < 750; Kig++)
        {
            Ki[Kig].Fitness = 0.0;
            Ki[Kig].FitnessS1 = 0.0;
            Ki[Kig].FitnessS2 = 0.0;
            Ki[Kig].UpdatesS1 = 0;
            Ki[Kig].UpdatesS2 = 0;

            // NEU: Fitness pro Spiel getrennt sammeln, statt sofort zu summieren
            double FitnessProSpiel[2] = {0.0, 0.0};

            for (int Spiel = 0; Spiel < 2; Spiel++)
            {
                MeineKlasse lSpiel;
                int Updates = 0;
                double LocalFitness = 0.0;

                if (Spiel == 0)
                {
                    lSpiel.PhysikZuruecksetzen(gen, Winkel_Stange_1_0, Winkel_Stange_2_0);
                }
                else
                {
                    lSpiel.PhysikZuruecksetzen(gen, Winkel_Stange_1_1, Winkel_Stange_2_1);
                }

                while (lSpiel.spielvorbei != true)
                {
                    double kraft = Ki[Kig].Denken(lSpiel.Wagen_Position, lSpiel.Wagen_Geschwindigkeit, lSpiel.Winkel_Stange_1, lSpiel.Winkel_Geschwindigkeit_1, lSpiel.Winkel_Stange_2, lSpiel.Winkel_Geschwindigkeit_2);
                    LocalFitness += lSpiel.Update(kraft, 0.01);
                    Updates++;

                    if (Updates > LimitUpdates)
                    {
                        break;
                    }
                    // NEU: Für Spiel 2 (Spiel == 1) das großzügigere Limit verwenden,
                    // damit ein noch nicht "obenstehendes" KI-Modell nicht zu früh abgebrochen wird.
                    int NichtObenLimit = (Spiel == 1) ? NichtObenLimitSpiel2 : 1500;
                    if (Updates > NichtObenLimit && lSpiel.waroben == false)
                    {
                        break;
                    }
                }

                // NEU: Pro Spiel auf "Fitness pro Zeitschritt" normieren.
                // Damit dominiert nicht automatisch das Spiel, das zufällig länger läuft
                // (z.B. weil LimitUpdates im Lauf des Trainings auf bis zu 25000 steigt).
                double NormierteFitness = LocalFitness / std::max(1, Updates);
                FitnessProSpiel[Spiel] = NormierteFitness;

                if (Spiel == 0)
                {
                    Ki[Kig].UpdatesS1 = Updates;
                    Ki[Kig].FitnessS1 = NormierteFitness;
                }
                if (Spiel == 1)
                {
                    Ki[Kig].UpdatesS2 = Updates;
                    Ki[Kig].FitnessS2 = NormierteFitness;
                }

#pragma omp critical
                {
                    if (Updates > MeistenUpdates[Spiel])
                    {
                        MeistenUpdates[Spiel] = Updates;
                    }
                }
            }

            // NEU: Das Minimum aus beiden (normierten) Spiel-Fitnesswerten nehmen statt der Summe.
            // Eine KI, die nur Spiel 1 kann, aber in Spiel 2 versagt, wird jetzt nicht mehr
            // durch die riesigen Spiel-1-Werte "reingewaschen" - sie muss in BEIDEN gut sein.
            Ki[Kig].Fitness = std::min(FitnessProSpiel[0], FitnessProSpiel[1]);
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

        std::cout << "Generation " << gen << " - Beste Fitness (min): " << BesteKi[0].Fitness
                  << " - Spiel1 Fitness: " << BesteKi[0].FitnessS1
                  << " - Spiel2 Fitness: " << BesteKi[0].FitnessS2
                  << " - Spiel 1 Updates: " << MeistenUpdates[0]
                  << " - Spiel 2 Updates: " << MeistenUpdates[1]
                  << " - Limit: " << LimitUpdates << std::endl;

        {
            std::lock_guard<std::mutex> lock(trainingsStatusMutex);
            aktuellerTrainingsStatus = {gen, BesteKi[0].Fitness, MeistenUpdates[0], MeistenUpdates[1]};
        }

        if (BesteFitnness.size() >= 50)
        {
            BesteFitnness.pop_back();
        }
        BesteFitnness.push_front(BesteKi[0].Fitness);

        if (Spiel_1_UL.size() >= 50)
        {
            Spiel_1_UL.pop_back();
        }
        Spiel_1_UL.push_front(BesteKi[0].UpdatesS1);

        if (Spiel_2_UL.size() >= 50)
        {
            Spiel_2_UL.pop_back();
        }
        Spiel_2_UL.push_front(BesteKi[0].UpdatesS2);

        if (Spiel_1_UL.size() >= 10 && Spiel_2_UL.size() >= 10)
        {
            double ZahlS1 = 0, ZahlS2 = 0;
            for (int i = 0; i < 10; i++)
            {
                ZahlS1 += Spiel_1_UL[i];
                ZahlS2 += Spiel_2_UL[i];
            }
            double ZahlZ = (ZahlS1 / 10.0 + ZahlS2 / 10.0) / 2.0;

            if (ZahlZ >= LimitUpdates - 10)
            {
                if (LimitUpdates < 25000)
                {
                    LimitUpdates += 100;
                }
            }
        }

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
                int Rand = static_cast<int>(RandomZahl(0.0, 4.999999));
                Ki[e] = BesteKi[Rand];
                Ki[e].Motiren(BesteFitnness, BesteKi[0].Fitness, Spiel_1_UL, Spiel_2_UL);
            }
        }
    }
}

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
    for (int e = 0; e < 26; e++)
    {
        KiAusgeben.Biases_1[e] = BesteKi[0].HidelLayer_1[e].Bias;
        for (int i = 0; i < 8; i++)
        {
            KiAusgeben.Gewichte_1[e][i] = BesteKi[0].HidelLayer_1[e].Gewichte[i];
        }
    }
    for (int e = 0; e < 12; e++)
    {
        KiAusgeben.Biases_2[e] = BesteKi[0].HidenLayer_2[e].Bias;
        for (int i = 0; i < 26; i++)
        {
            KiAusgeben.Gewichte_2[e][i] = BesteKi[0].HidenLayer_2[e].Gewichte[i];
        }
    }
    for (int i = 0; i < 12; i++)
    {
        KiAusgeben.Gewichte_3[i] = BesteKi[0].Endgewichte[i];
    }
    KiAusgeben.Bias3 = BesteKi[0].Bias;
    return KiAusgeben;
}

extern "C" __declspec(dllexport) void GetKiC(RetrunKi NeueKi)
{
    for (int e = 0; e < 26; e++)
    {
        BesteKi[0].HidelLayer_1[e].Bias = NeueKi.Biases_1[e];
        for (int i = 0; i < 8; i++)
        {
            BesteKi[0].HidelLayer_1[e].Gewichte[i] = NeueKi.Gewichte_1[e][i];
        }
    }

    for (int e = 0; e < 12; e++)
    {
        BesteKi[0].HidenLayer_2[e].Bias = NeueKi.Biases_2[e];
        for (int i = 0; i < 26; i++)
        {
            BesteKi[0].HidenLayer_2[e].Gewichte[i] = NeueKi.Gewichte_2[e][i];
        }
    }

    for (int i = 0; i < 12; i++)
    {
        BesteKi[0].Endgewichte[i] = NeueKi.Gewichte_3[i];
    }

    BesteKi[0].Bias = NeueKi.Bias3;
}

extern "C" __declspec(dllexport) void StarteTrainning()
{
    training = true;
}

extern "C" __declspec(dllexport) void StoppeTrainning()
{
    training = false;
}

extern "C" __declspec(dllexport) void HoleTrainingsStatus(TrainingsStatus *status)
{
    if (status == nullptr)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(trainingsStatusMutex);
    *status = aktuellerTrainingsStatus;
}

extern "C" __declspec(dllexport) void TrainingNeuStarten()
{
    training = false;
    TrainingGestated = false;
    gen = 0;
    LimitUpdates = 7200;
    BesteFitnness.clear();
    Spiel_1_UL.clear();
    Spiel_2_UL.clear();
}