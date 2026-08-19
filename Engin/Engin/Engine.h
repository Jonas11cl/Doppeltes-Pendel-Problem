#pragma once
#include "Engine.h"
#include <iostream>
#include <cmath>
#include <cstdio>
#include <random>
#include <cmath>
#include <unordered_map>

class MeineKlasse
{
public:
    double Wagen_Position = 0.0;
    double Wagen_Geschwindigkeit = 0.0;
    double Wagen_Beschleunigung = 0.0;

    double Winkel_Stange_1 = 0.2;
    double Winkel_Stange_2 = 0.2;
    double Winkel_Geschwindigkeit_1 = 0.0;
    double Winkel_Geschwindigkeit_2 = 0.0;
    double Winkel_Beschleunigung_1 = 0.0;
    double Winkel_Beschleunigung_2 = 0.0;

    double Schubkraft = 0.0;
    double DeltaTime = 0.02;

    double Schwerkraft = 9.81;
    double Masse_Wagen = 1.0;
    double Masse_Stange_1 = 0.1;
    double Laenge_Stange_1 = 0.5;
    double Masse_Stange_2 = 0.1;
    double Laenge_Stange_2 = 0.5;
    double Reibung = 0.05;
    bool spielvorbei = false;
    double Zielenergie_1 = Masse_Stange_1 * Schwerkraft * Laenge_Stange_1;
    double Zielenergie_2 = Masse_Stange_1 * Schwerkraft * (Laenge_Stange_1 + Laenge_Stange_2);
    double GesammtZielEnergie = Zielenergie_1 + Zielenergie_2;
    double RandomZahlR(double Anfang, double Ende)
    {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<double> distrib(Anfang, Ende);
        return distrib(gen);
    }
    void PhysikZuruecksetzen(int Generation, int Spiel)
    {
        /* if (Generation < 200)
         {*/
        Winkel_Stange_1 = RandomZahlR(-0.05, 0.05); // Startet minimal links oder rechts
        Winkel_Stange_2 = RandomZahlR(-0.05, 0.05);
        /* }*/
        /*       else if (Generation < 500)
               {
                   Winkel_Stange_1 = 1.5708;
                   Winkel_Stange_2 = 1.5708;
               }
               else
               {

                   if (Spiel == 1)
                   {
                       Winkel_Stange_1 = 0.3;
                       Winkel_Stange_2 = 0.3;
                   }
                   else
                   {
                       Winkel_Stange_1 = 3.142;
                       Winkel_Stange_2 = 3.142;
                   }
               }*/
        Wagen_Position = 0.0;
        Wagen_Geschwindigkeit = 0.0;
        Winkel_Geschwindigkeit_1 = 0.0;
        Winkel_Geschwindigkeit_2 = 0.0;
        spielvorbei = false;
    }

    double Update(double Schubkraft, double Zeitschritt)
    {
        double Fittness = 0;
        double Winkeldifference = Winkel_Stange_1 - Winkel_Stange_2;
        double FligkraftW1 = pow(Winkel_Geschwindigkeit_1, 2.0);
        double FligkraftW2 = pow(Winkel_Geschwindigkeit_2, 2.0);
        double B1 = Schubkraft + (Masse_Stange_1 + Masse_Stange_2) * Laenge_Stange_1 * FligkraftW1 * sin(Winkel_Stange_1) + Masse_Stange_2 * Laenge_Stange_2 * FligkraftW2 * sin(Winkel_Stange_2);
        double B2 = (Masse_Stange_1 + Masse_Stange_2) * Schwerkraft * Laenge_Stange_1 * sin(Winkel_Stange_1) - Masse_Stange_2 * Laenge_Stange_1 * Laenge_Stange_2 * FligkraftW2 * sin(Winkeldifference);
        double B3 = Masse_Stange_2 * Schwerkraft * Laenge_Stange_2 * sin(Winkel_Stange_2) + Masse_Stange_2 * Laenge_Stange_1 * Laenge_Stange_2 * FligkraftW1 * sin(Winkeldifference);
        double MatrixA[3][3];
        MatrixA[0][0] = Masse_Wagen + Masse_Stange_1 + Masse_Stange_2;
        MatrixA[0][1] = (Masse_Stange_1 + Masse_Stange_2) * Laenge_Stange_1 * cos(Winkel_Stange_1);
        MatrixA[0][2] = Masse_Stange_2 * Laenge_Stange_2 * cos(Winkel_Stange_2);
        MatrixA[1][0] = MatrixA[0][1];
        MatrixA[1][1] = (Masse_Stange_1 + Masse_Stange_2) * pow(Laenge_Stange_1, 2.0);
        MatrixA[1][2] = Masse_Stange_2 * Laenge_Stange_1 * Laenge_Stange_2 * cos(Winkeldifference);
        MatrixA[2][0] = MatrixA[0][2];
        MatrixA[2][1] = MatrixA[1][2];
        MatrixA[2][2] = Masse_Stange_2 * pow(Laenge_Stange_2, 2.0);
        Geschwindigkeitbrechenen(MatrixA, B1, B2, B3);
        Wagen_Geschwindigkeit = Wagen_Geschwindigkeit + (Wagen_Beschleunigung * Zeitschritt);
        Winkel_Geschwindigkeit_1 = Winkel_Geschwindigkeit_1 + (Winkel_Beschleunigung_1 * Zeitschritt);
        Winkel_Geschwindigkeit_2 = Winkel_Geschwindigkeit_2 + (Winkel_Beschleunigung_2 * Zeitschritt);
        Wagen_Position = Wagen_Position + (Wagen_Geschwindigkeit * Zeitschritt);
        Winkel_Stange_1 = Winkel_Stange_1 + (Winkel_Geschwindigkeit_1 * Zeitschritt);
        Winkel_Stange_2 = Winkel_Stange_2 + (Winkel_Geschwindigkeit_2 * Zeitschritt);
        while (Winkel_Stange_1 > 3.14159265)
            Winkel_Stange_1 -= 6.2831853;
        while (Winkel_Stange_1 < -3.14159265)
            Winkel_Stange_1 += 6.2831853;

        while (Winkel_Stange_2 > 3.14159265)
            Winkel_Stange_2 -= 6.2831853;
        while (Winkel_Stange_2 < -3.14159265)
            Winkel_Stange_2 += 6.2831853;
        if (Wagen_Position < -2.4 || Wagen_Position > 2.4)
        {
            Wagen_Position = std::max(-2.4, std::min(2.4, Wagen_Position));
            spielvorbei = true;
            Fittness = -1000;
        }
        else
        {
            Fittness = FittnessBerechen();
        }

        return Fittness;
    }

    void Geschwindigkeitbrechenen(double MA[3][3], double B1, double B2, double B3)
    {
        double det = MA[0][0] * (MA[1][1] * MA[2][2] - MA[1][2] * MA[2][1]) - MA[0][1] * (MA[1][0] * MA[2][2] - MA[1][2] * MA[2][0]) + MA[0][2] * (MA[1][0] * MA[2][1] - MA[1][1] * MA[2][0]);

        if (std::fabs(det) < 1e-9)
        {
            Wagen_Beschleunigung = 0.0;
            Winkel_Beschleunigung_1 = 0.0;
            Winkel_Beschleunigung_2 = 0.0;
            return;
        }

        double det0 = B1 * (MA[1][1] * MA[2][2] - MA[1][2] * MA[2][1]) - MA[0][1] * (B2 * MA[2][2] - MA[1][2] * B3) + MA[0][2] * (B2 * MA[2][1] - MA[1][1] * B3);

        double det1 = MA[0][0] * (B2 * MA[2][2] - MA[1][2] * B3) - B1 * (MA[1][0] * MA[2][2] - MA[1][2] * MA[2][0]) + MA[0][2] * (MA[1][0] * B3 - B2 * MA[2][0]);

        double det2 = MA[0][0] * (MA[1][1] * B3 - B2 * MA[2][1]) - MA[0][1] * (MA[1][0] * B3 - B2 * MA[2][0]) + B1 * (MA[1][0] * MA[2][1] - MA[1][1] * MA[2][0]);

        Wagen_Beschleunigung = det0 / det;
        Winkel_Beschleunigung_1 = det1 / det;
        Winkel_Beschleunigung_2 = det2 / det;
    }
    double FittnessBerechen()
    {
        if (cos(Winkel_Stange_1) > 0.85 && cos(Winkel_Stange_2) > 0.85)
        {
            double WinkelFeher_1 = 7.5 * (1.0 - cos(Winkel_Stange_1));
            double WinkelFeher_2 = 7.5 * (1.0 - cos(Winkel_Stange_2));
            double Winkelgeschwindigkeit = 0.3 * (abs(Winkel_Geschwindigkeit_1) + abs(Winkel_Geschwindigkeit_2));
            double WagenAbweichung = 1.5 * abs(Wagen_Position);
            double Fehler = WinkelFeher_1 + WinkelFeher_2 + Winkelgeschwindigkeit + WagenAbweichung + (0.5 * abs(Wagen_Geschwindigkeit));
            return 5.0 * (1.0 / (1.0 + Fehler)) + 2.0;
        }
        else
        {
            double Lageengerie_1 = Masse_Stange_1 * Schwerkraft * (Laenge_Stange_1 * cos(Winkel_Stange_1));
            double Lagenergie_2 = Masse_Stange_2 * Schwerkraft * ((Laenge_Stange_1 * cos(Winkel_Stange_1)) + (Laenge_Stange_2 * cos(Winkel_Stange_2)));
            double Lageenergie = Lageengerie_1 + Lagenergie_2;
            double Bewegungsenergie_1 = 0.5 * Masse_Stange_1 * pow((Laenge_Stange_1 * Winkel_Geschwindigkeit_1), 2.0);
            double Bewegungsenergie_2 = 0.5 * Masse_Stange_2 * pow((Laenge_Stange_2 * Winkel_Geschwindigkeit_2), 2.0);
            double Bewegumgsenergie = Bewegungsenergie_1 + Bewegungsenergie_2;
            double Energie = Bewegumgsenergie + Lageenergie;

            double Fehler = abs(GesammtZielEnergie - Energie);
            return 1.0 / (1.0 + Fehler);
        }
    }
};
