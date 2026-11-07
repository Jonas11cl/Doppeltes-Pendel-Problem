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
    int UeberlebtSpiel3;
    double Logwarscheilichkeit[3][7200];
    double CKiFittness[3][7200];
    double CFittness[3][7200];
    double CFehlerTick[3][7200];
     double CNormalFehlerTick[3][7200];
    double CFehler;
    double CRechenfehler;
    double CKraft[3][7200]
};

int LimitUpdates = 7200;
double HinzuKraft = 0;

TrainingsStatus aktuellerTrainingsStatus{0, 0.0, 0, 0, 0};
std::mutex trainingsStatusMutex;

double RandomZahl(double Anfang, double Ende)
{
    static std::random_device rd;
    thread_local static std::mt19937 gen(rd());
    std::uniform_real_distribution<double> distrib(Anfang, Ende);
    return distrib(gen);
}
double Normal(double mittelwert, double unsicherheit)
{
    std::random_device rd;
    std::mt19937 gen(rd());

    std::normal_distribution<double> d(mittelwert, unsicherheit);

    double zufallszahl = d(gen);
    return zufallszahl;
}
double Normalwarscheinelichkeit(double gezogene_kraft, double unsicherheit, double mittelwert)
{
    const double ln_2pi = std::log(2.0 * 3.14159265358979323846);

    double term1 = -std::log(unsicherheit);
    double term2 = -0.5 * ln_2pi;
    double term3 = -std::pow(gezogene_kraft - mittelwert, 2) / (2.0 * std::pow(unsicherheit, 2));

    double log_wahrscheinlichkeitsdichte = term1 + term2 + term3;

    std::cout << "Natürlicher Logarithmus der Wahrscheinlichkeitsdichte: "
              << log_wahrscheinlichkeitsdichte << std::endl;

    return log_wahrscheinlichkeitsdichte;
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


class KibufferCritic
{
    public: 
    double Gewichte_1[26][8];
    double Biases_1[26];
    double Gewichte_2[12][26];
    double Biases_2[12];
    double Gewichte_3[12];
    double Bias3;
}

class KibufferActor
{
    public: 
    double Gewichte_1[26][8];
    double Biases_1[26];
    double Gewichte_2[12][26];
    double Biases_2[12];
    double Gewichte_3[12];
    double Bias3;
}

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
    int UpdatesS3 = 0;

    double FitnessS1 = 0.0;
    double FitnessS2 = 0.0;
    double FitnessS3 = 0.0;

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

    void Motiren(const std::deque<double> &BesteFitness, double bestfitness, const std::deque<double> &UberlebtSpiel1, const std::deque<double> &UberlebtSpiel2, const std::deque<double> &UberlebtSpiel3)
    {
        double MuttationsWarscheilichkeit = 0.05;
        double RadMax = 0.05;
        double RadMin = -0.05;

        if (UberlebtSpiel1.size() >= 10 && UberlebtSpiel2.size() >= 10 && UberlebtSpiel3.size() >= 10)
        {
            double ZahlS1 = 0, ZahlS2 = 0, ZahlS3 = 0;
            for (int i = 0; i < 10; i++)
            {
                ZahlS1 += UberlebtSpiel1[i];
                ZahlS2 += UberlebtSpiel2[i];
                ZahlS3 += UberlebtSpiel3[i];
            }

            ZahlS1 /= 10.0;
            ZahlS2 /= 10.0;
            ZahlS3 /= 10.0;

            double ZahlZ = std::min({ZahlS1, ZahlS2, ZahlS3});

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

class NuralNetwork_Critic
{
public:
    HidenNode1 HidelLayer_1[26];
    HidenNode2 HidenLayer_2[12];
    double Bias;
    double Fitness;

    int UpdatesS1 = 0;
    int UpdatesS2 = 0;
    int UpdatesS3 = 0;

    double FitnessS1 = 0.0;
    double FitnessS2 = 0.0;
    double FitnessS3 = 0.0;

    double Summe_1[26];
    double Summe_2[12];
    double Endgewichte[12];

    NuralNetwork_Critic() : Bias(RandomZahl(-1.0, 1.0)), Fitness(0.0)
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
        return Endsumme;
    }

    void Motiren(const std::deque<double> &BesteFitness, double bestfitness, const std::deque<double> &UberlebtSpiel1, const std::deque<double> &UberlebtSpiel2, const std::deque<double> &UberlebtSpiel3)
    {
        double MuttationsWarscheilichkeit = 0.05;
        double RadMax = 0.05;
        double RadMin = -0.05;

        if (UberlebtSpiel1.size() >= 10 && UberlebtSpiel2.size() >= 10 && UberlebtSpiel3.size() >= 10)
        {
            double ZahlS1 = 0, ZahlS2 = 0, ZahlS3 = 0;
            for (int i = 0; i < 10; i++)
            {
                ZahlS1 += UberlebtSpiel1[i];
                ZahlS2 += UberlebtSpiel2[i];
                ZahlS3 += UberlebtSpiel3[i];
            }

            ZahlS1 /= 10.0;
            ZahlS2 /= 10.0;
            ZahlS3 /= 10.0;

            double ZahlZ = std::min({ZahlS1, ZahlS2, ZahlS3});

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
            *this = NuralNetwork_Critic();
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
NuralNetwork Ki;
NuralNetwork_Critic Critic;
MeineKlasse lokalesSpiel;

int gen = 0;
bool trainingFertig = false;
int GenerationenSeitLetzterErhoehung = 0;
std::deque<double> BesteFitnness;
std::deque<double> Spiel_1_UL;
std::deque<double> Spiel_2_UL;
std::deque<double> Spiel_3_UL;

extern "C" __declspec(dllexport) void StarteTraining()
{
    if (TrainingGestated == false)
    {
        TrainingGestated = true;
        Ki = NuralNetwork();
        Critic = NuralNetwork_Critic();
    }

    while (training == true)
    {
        gen++;
        double Winkel_Stange_1_0 = RandomZahl(-0.08, 0.08);
        double Winkel_Stange_2_0 = RandomZahl(-0.08, 0.08);
        double Winkel_Stange_1_1;
        double Winkel_Stange_2_1;
        double Winkel_Stange_1_2 = RandomZahl(-0.08, 0.08);
        double Winkel_Stange_2_2 = RandomZahl(-0.08, 0.08);
        if (lokalesSpiel.Unsicherheitsfaktor > 0.1)
        {
            lokalesSpiel.Unsicherheitsfaktor -= 0.01;
        }

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
        int MeistenUpdates[3] = {-1, -1, -1};

        int NichtObenLimitSpiel2 = std::min(4000, LimitUpdates / 2);

#pragma omp parallel for

        Ki.Fitness = 0.0;
        Ki.FitnessS1 = 0.0;
        Ki.FitnessS2 = 0.0;
        Ki.FitnessS3 = 0.0;
        Ki.UpdatesS1 = 0;
        Ki.UpdatesS2 = 0;
        Ki.UpdatesS3 = 0;

        double FitnessProSpiel[3] = {0.0, 0.0, 0.0};

        for (int Spiel = 0; Spiel < 3; Spiel++)
        {
            MeineKlasse lSpiel;
            int Updates = 0;
            double LocalFitness = 0.0;

            int NachsterStupsIn = static_cast<int>(RandomZahl(100.0, 400.0));
            double AktuelleStupsKraft = 0.0;
            int StupsRestdauer = 0;

            if (Spiel == 0)
            {
                lSpiel.PhysikZuruecksetzen(gen, Winkel_Stange_1_0, Winkel_Stange_2_0);
            }
            else if (Spiel == 1)
            {
                lSpiel.PhysikZuruecksetzen(gen, Winkel_Stange_1_1, Winkel_Stange_2_1);
            }
            else
            {
                lSpiel.PhysikZuruecksetzen(gen, Winkel_Stange_1_2, Winkel_Stange_2_2);
            }

            while (lSpiel.spielvorbei != true)
            {
                double kraftKi = Ki.Denken(lSpiel.Wagen_Position, lSpiel.Wagen_Geschwindigkeit, lSpiel.Winkel_Stange_1, lSpiel.Winkel_Geschwindigkeit_1, lSpiel.Winkel_Stange_2, lSpiel.Winkel_Geschwindigkeit_2);
                double kraft = Normal(kraftKi, lokalesSpiel.Unsicherheitsfaktor);
                double normalwarscheinlichkeit = Normalwarscheinelichkeit(kraft, lokalesSpiel.Unsicherheitsfaktor, kraftKi);
                aktuellerTrainingsStatus.Logwarscheilichkeit[Spiel][Updates] = normalwarscheinlichkeit;
                double KiFittnness = Critic.Denken(lSpiel.Wagen_Position, lSpiel.Wagen_Geschwindigkeit, lSpiel.Winkel_Stange_1, lSpiel.Winkel_Geschwindigkeit_1, lSpiel.Winkel_Stange_2, lSpiel.Winkel_Geschwindigkeit_2);
                aktuellerTrainingsStatus.CKiFittness[Spiel][Updates] = KiFittnness;
                aktuellerTrainingsStatus.CKraft[Spiel][Updates] = kraft;
                if (Spiel == 2)
                {
                    if (StupsRestdauer > 0)
                    {
                        kraft += AktuelleStupsKraft;
                        StupsRestdauer--;
                    }
                    else
                    {
                        NachsterStupsIn--;
                        if (NachsterStupsIn <= 0)
                        {
                            double Richtung = (RandomZahl(0.0, 1.0) < 0.5) ? -1.0 : 1.0;
                            AktuelleStupsKraft = Richtung * RandomZahl(3.0, 6.0);
                            StupsRestdauer = static_cast<int>(RandomZahl(5.0, 15.0));
                            NachsterStupsIn = static_cast<int>(RandomZahl(150.0, 400.0));
                        }
                    }
                }

                double AkkFittnes = lSpiel.Update(kraft, 0.01, Updates, Spiel);
                LocalFitness += AkkFittnes;
                aktuellerTrainingsStatus.CFittness[Spiel][Updates] = AkkFittnes;

                Updates++;

                if (Updates > LimitUpdates)
                {
                    break;
                }

                int NichtObenLimit = (Spiel == 1) ? NichtObenLimitSpiel2 : 1500;
                if (Updates > NichtObenLimit && lSpiel.waroben == false)
                {
                    break;
                }
            }

            double NormierteFitness = LocalFitness / std::max(1, Updates);
            FitnessProSpiel[Spiel] = NormierteFitness;

            if (Spiel == 0)
            {
                Ki.UpdatesS1 = Updates;
                Ki.FitnessS1 = NormierteFitness;
            }
            if (Spiel == 1)
            {
                Ki.UpdatesS2 = Updates;
                Ki.FitnessS2 = NormierteFitness;
            }
            if (Spiel == 2)
            {
                Ki.UpdatesS3 = Updates;
                Ki.FitnessS3 = NormierteFitness;
            }

#pragma omp critical
        }

        Ki.Fitness = std::min({FitnessProSpiel[0], FitnessProSpiel[1], FitnessProSpiel[2]});
    }

    std::cout << "Generation " << gen << " - Beste Fitness (min): " << Ki.Fitness
              << " - Spiel1 Fitness: " << Ki.FitnessS1
              << " - Spiel2 Fitness: " << Ki.FitnessS2
              << " - Spiel3(Stups) Fitness: " << Ki.FitnessS3
              << " - Updates Spiel1: " << Ki.UpdatesS1
              << " - Updates Spiel2: " << Ki.UpdatesS2
              << " - Updates Spiel3: " << Ki.UpdatesS3
              << " - Limit: " << LimitUpdates << std::endl;

    {
        std::lock_guard<std::mutex> lock(trainingsStatusMutex);
        aktuellerTrainingsStatus = {gen, Ki.Fitness, Ki.UpdatesS1, Ki.UpdatesS2, Ki.UpdatesS3};

        if (BesteFitnness.size() >= 50)
        {
            BesteFitnness.pop_back();
        }
        BesteFitnness.push_front(Ki.Fitness);

        if (Spiel_1_UL.size() >= 50)
        {
            Spiel_1_UL.pop_back();
        }
        Spiel_1_UL.push_front(Ki.UpdatesS1);

        if (Spiel_2_UL.size() >= 50)
        {
            Spiel_2_UL.pop_back();
        }
        Spiel_2_UL.push_front(Ki.UpdatesS2);

        if (Spiel_3_UL.size() >= 50)
        {
            Spiel_3_UL.pop_back();
        }
        Spiel_3_UL.push_front(Ki.UpdatesS3);

        if (Spiel_1_UL.size() >= 10 && Spiel_2_UL.size() >= 10 && Spiel_3_UL.size() >= 10)
        {
            double ZahlS1 = 0, ZahlS2 = 0, ZahlS3 = 0;
            for (int i = 0; i < 10; i++)
            {
                ZahlS1 += Spiel_1_UL[i];
                ZahlS2 += Spiel_2_UL[i];
                ZahlS3 += Spiel_3_UL[i];
            }
            ZahlS1 /= 10.0;
            ZahlS2 /= 10.0;
            ZahlS3 /= 10.0;

            double ZahlZ = std::min({ZahlS1, ZahlS2, ZahlS3});

            GenerationenSeitLetzterErhoehung++;

            const double SchwellenProzentLimitErhoehen = 0.75;
            const int CooldownGenerationen = 15;

            if (ZahlZ >= LimitUpdates * SchwellenProzentLimitErhoehen &&
                GenerationenSeitLetzterErhoehung >= CooldownGenerationen)
            {
                if (LimitUpdates < 25000)
                {
                    LimitUpdates += 100;
                    GenerationenSeitLetzterErhoehung = 0;
                }
            }
        }

        if (gen == 500 || gen == 1200)
        {
            BesteFitnness.clear();
            Spiel_1_UL.clear();
            Spiel_2_UL.clear();
            Spiel_3_UL.clear();
        }

        int Rand = static_cast<int>(RandomZahl(0.0, 4.999999));

        Ki.Motiren(BesteFitnness, Ki.Fitness, Spiel_1_UL, Spiel_2_UL, Spiel_3_UL);
        double Durchschitt = 0;
        for (int ge = 0; ge <= 2; ge++)
        {
            double Zukunft = 0;
            int Updates = 0;
            if (ge == 0)
            {
                Updates = aktuellerTrainingsStatus.UeberlebtSpiel1;
            }
            else if (ge == 1)
            {
                Updates = aktuellerTrainingsStatus.UeberlebtSpiel2;
            }
            else
            {
                Updates = aktuellerTrainingsStatus.UeberlebtSpiel3;
            }
            for (int up = 0; up < Updates; up++)
            {
               
                Zukunft = aktuellerTrainingsStatus.CFittness[ge][Updates-1- up] + (0.99 * Zukunft);
                aktuellerTrainingsStatus.CFehlerTick[ge][Updates-1- up] = Zukunft - aktuellerTrainingsStatus.CKiFittness[ge][Updates-1- up];
            }
        }
        
        double SpielFehler[3] = {0,0,0};
        double Abweichung = 0;
        for (int ge = 0; ge <= 2; ge++)
        {
            int Updates;
            if (ge == 0)
            {
                Updates = aktuellerTrainingsStatus.UeberlebtSpiel1;
            }
            else if (ge == 1)
            {
                Updates = aktuellerTrainingsStatus.UeberlebtSpiel2;
            }
            else
            {
                Updates = aktuellerTrainingsStatus.UeberlebtSpiel3;
            }
            for (int up = 0; up < Updates; up++)
            {
                double Fehler = aktuellerTrainingsStatus.CFehlerTick[ge][up];
                Durchschitt += aktuellerTrainingsStatus.CFehlerTick[ge][up];
                SpielFehler[ge] += Fehler;
                aktuellerTrainingsStatus
            }
        }
          Durchschitt = Durchschitt /(aktuellerTrainingsStatus.UeberlebtSpiel1 + aktuellerTrainingsStatus.UeberlebtSpiel2 + aktuellerTrainingsStatus.UeberlebtSpiel3);
         for (int ge = 0; ge <= 2; ge++)
        {
            int Updates = 0;
            if (ge == 0)
            {
                Updates = aktuellerTrainingsStatus.UeberlebtSpiel1;
            }
            else if (ge == 1)
            {
                Updates = aktuellerTrainingsStatus.UeberlebtSpiel2;
            }
            else
            {
                Updates = aktuellerTrainingsStatus.UeberlebtSpiel3;
            }
            for (int up = 0; up < Updates; up++)
            {
              double HAbweichung = Durchschitt - aktuellerTrainingsStatus.CFehlerTick[ge][up];
              Abweichung += (HAbweichung * HAbweichung);
            }
            
        }
       Abweichung =  Abweichung/(aktuellerTrainingsStatus.UeberlebtSpiel1 + aktuellerTrainingsStatus.UeberlebtSpiel2 + aktuellerTrainingsStatus.UeberlebtSpiel3);
            Abweichung = std::sqrt(Abweichung);
        for (int ge = 0; ge <= 2; ge++)
        {
            int Updates =0;
            if (ge == 0)
            {
                Updates = aktuellerTrainingsStatus.UeberlebtSpiel1;
            }
            else if (ge == 1)
            {
                Updates = aktuellerTrainingsStatus.UeberlebtSpiel2;
            }
            else
            {
                Updates = aktuellerTrainingsStatus.UeberlebtSpiel3;
            }
            for (int up = 0; up < Updates; up++)
            {
              aktuellerTrainingsStatus.CNormalFehlerTick[ge][up] = (aktuellerTrainingsStatus.CFehlerTick[ge][up] - Durchschitt) /(Abweichung+0.00000001);
            }
        }
        aktuellerTrainingsStatus.CFehler = SpielFehler[0] + SpielFehler[1] + SpielFehler[2];

        for (int ge = 0; ge <= 2; ge++)
        {
            int Updates =0;
            if (ge == 0)
            {
                Updates = aktuellerTrainingsStatus.UeberlebtSpiel1;
            }
            else if (ge == 1)
            {
                Updates = aktuellerTrainingsStatus.UeberlebtSpiel2;
            }
            else
            {
                Updates = aktuellerTrainingsStatus.UeberlebtSpiel3;
            }
            for (int up = 0; up < Updates; up++)
            {
                
              
            }
        }
    }
}

extern "C" __declspec(dllexport) void ResetPhysik()
{
    lokalesSpiel.PhysikZuruecksetzen(10000, 3.0, 3.0);
}

extern "C" __declspec(dllexport) void MachEinenLiveSchritt()
{
    double Krfat = Ki.Denken(lokalesSpiel.Wagen_Position, lokalesSpiel.Wagen_Geschwindigkeit, lokalesSpiel.Winkel_Stange_1, lokalesSpiel.Winkel_Geschwindigkeit_1, lokalesSpiel.Winkel_Stange_2, lokalesSpiel.Winkel_Geschwindigkeit_2);
    lokalesSpiel.Update(Krfat + HinzuKraft, 0.01);
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
        KiAusgeben.Biases_1[e] = Ki.HidelLayer_1[e].Bias;
        for (int i = 0; i < 8; i++)
        {
            KiAusgeben.Gewichte_1[e][i] = Ki.HidelLayer_1[e].Gewichte[i];
        }
    }
    for (int e = 0; e < 12; e++)
    {
        KiAusgeben.Biases_2[e] = Ki.HidenLayer_2[e].Bias;
        for (int i = 0; i < 26; i++)
        {
            KiAusgeben.Gewichte_2[e][i] = Ki.HidenLayer_2[e].Gewichte[i];
        }
    }
    for (int i = 0; i < 12; i++)
    {
        KiAusgeben.Gewichte_3[i] = Ki.Endgewichte[i];
    }
    KiAusgeben.Bias3 = Ki.Bias;
    return KiAusgeben;
}

extern "C" __declspec(dllexport) void GetKiC(RetrunKi NeueKi)
{
    for (int e = 0; e < 26; e++)
    {
        Ki.HidelLayer_1[e].Bias = NeueKi.Biases_1[e];
        for (int i = 0; i < 8; i++)
        {
            Ki.HidelLayer_1[e].Gewichte[i] = NeueKi.Gewichte_1[e][i];
        }
    }

    for (int e = 0; e < 12; e++)
    {
        Ki.HidenLayer_2[e].Bias = NeueKi.Biases_2[e];
        for (int i = 0; i < 26; i++)
        {
            Ki.HidenLayer_2[e].Gewichte[i] = NeueKi.Gewichte_2[e][i];
        }
    }

    for (int i = 0; i < 12; i++)
    {
        Ki.Endgewichte[i] = NeueKi.Gewichte_3[i];
    }

    Ki.Bias = NeueKi.Bias3;
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
    GenerationenSeitLetzterErhoehung = 0;
    BesteFitnness.clear();
    Spiel_1_UL.clear();
    Spiel_2_UL.clear();
    Spiel_3_UL.clear();
}

extern "C" __declspec(dllexport) void KraftInput(double HKraft)
{
    HinzuKraft = HKraft;
}