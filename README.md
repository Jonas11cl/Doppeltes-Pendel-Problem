# Doppeltes-Pendel-Problem

In diesem Repository sehen sie meinen Versuch eine KI zu bauen die versucht ein Doppeltes Pendel Aufzuschwingen und zu Stabilisieren. 

Das Projekt besteht aus 3 Teilen auf die wir später noch genauer eingehen werden.
1. Die KI selber plus Physics-Engine in C++ Geschieben und in eine aufrufbare .dll verpackt. 
2. Ein Server in C# Geschieben der die .dll aufruft und ihre daten an das Frontend weiter gibt.
3. Das Frontend in Typscript, Html und Css Geschieben was die finale KI anzeigt.


# 1. C++ KI plus Physics-Engine
Als ersten schritt im Projekt habe ich die Physics-Engine Geschieben, den ohne die klappt nichts,
sie funktioniert in dem man die Update funktioniert aufruft, dort werden die Kräfte die auf die einzelnen Pendeln und den Wagen wirken, Das beinhaltet die Kräfte die die Pendel und der Wagen Gegenseitig auf ein andere ausüben, die Schwerkraft, die Reibung und Der Input der KI.


Die KI Hat 6 Inputs Jeweils für die Pendel Winkel und Winkel Geschwindigkeit und für die Wage, die Position und die Geschwindigkeit. drauf folgen 2 Hidden-Layers mit 26 und 12 Hidden Nodes. und am enden einen Output der eine Kraft auf den Wagen ausübt.


