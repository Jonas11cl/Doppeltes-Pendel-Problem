#pragma once
class MeineKlasse
{
public:
    double Wagen_Position = 0.0;
    double Wagen_Geschwindigkeit = 0.0;
    double Wagen_Beschleunigung = 0.0;

    double Winkel_Stange = 0.05;
    double Winkel_Geschwindigkeit = 0.0;
    double Winkel_Beschleunigung = 0.0;

    double Schubkraft = 0.0;
    double DeltaTime = 0.02;

    double Schwerkraft = 9.81;
    double Masse_Wagen = 1.0;
    double Masse_Stange = 0.1;
    double Laenge_Stange = 0.5;
    double Reibung = 0.05;
    bool spielvorbei = false;
};
