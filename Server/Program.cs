using System;
using System.Collections.Concurrent;
using System.Runtime.InteropServices;
using System.Text.Json;
using System.Threading;
using System.Threading.Tasks;
using Fleck;

public class ClientPaket
{
    public int Version { get; set; }
    public string Inhalt { get; set; }
}

public class ClientMessage
{
    public string Aktion { get; set; }
    public string Inhalt { get; set; }
}

public class DatenPaket
{
    public int Version { get; set; }
    public double WagenX { get; set; }
    public double Winkel1 { get; set; }
    public double Winkel2 { get; set; }
}

public class TrainingsStatusPaket
{
    public int Version { get; set; }
    public int Generation { get; set; }
    public double BesteFitness { get; set; }
    public int UeberlebtSpiel1 { get; set; }
    public int UeberlebtSpiel2 { get; set; }
}

public class KiPaket
{
    public int Version { get; set; }
    public string KiDaten { get; set; }
}

[StructLayout(LayoutKind.Sequential)]
public struct RetrunKi
{
    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 208, ArraySubType = UnmanagedType.R8)]
    public double[] Gewichte_1;

    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 312, ArraySubType = UnmanagedType.R8)]
    public double[] Gewichte_2;

    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 12, ArraySubType = UnmanagedType.R8)]
    public double[] Gewichte_3;

    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 26, ArraySubType = UnmanagedType.R8)]
    public double[] Bias1;

    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 12, ArraySubType = UnmanagedType.R8)]
    public double[] Bias2;

    public double Bias3;
};

[StructLayout(LayoutKind.Sequential)]
public struct TrainingsStatus
{
    public int Generation;
    public double BesteFitness;
    public int UeberlebtSpiel1;
    public int UeberlebtSpiel2;
}

class Program
{
    // WICHTIG: CallingConvention.Cdecl überall hinzugefügt, um Stack-Korruption zu verhindern!
    [DllImport("Engin.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern void StarteTraining();

    [DllImport("Engin.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern void MachEinenLiveSchritt();

    [DllImport("Engin.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern double HoleWagenPosition();

    [DllImport("Engin.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern double HoleWinkel1();

    [DllImport("Engin.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern double HoleWinkel2();

    [DllImport("Engin.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern RetrunKi HoleKi();

    [DllImport("Engin.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern void ResetPhysik();

    [DllImport("Engin.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern void GetKiC(RetrunKi NeueKi);

    [DllImport("Engin.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern void StoppeTrainning();

    [DllImport("Engin.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern void StarteTrainning();

    [DllImport("Engin.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern void TrainingNeuStarten();

    [DllImport("Engin.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern void HoleTrainingsStatus(out TrainingsStatus status);

    private static readonly SemaphoreSlim ersterClientTor = new SemaphoreSlim(0);
    private static int torSchonOffen = 0;
    private static bool Running = true;
    private static bool TrainingAktive = false;
    private static CancellationTokenSource? trainingStatusAbbruch;
    private static Task? trainingTask;

    public static async Task Main()
    {
        var clients = new ConcurrentDictionary<IWebSocketConnection, byte>();
        var server = new WebSocketServer("ws://127.0.0.1:8080");

        server.Start(socket =>
        {
            socket.OnOpen = () =>
            {
                clients[socket] = 0;
                Console.WriteLine("Client verbunden.");

                if (Interlocked.Exchange(ref torSchonOffen, 1) == 0)
                {
                    ersterClientTor.Release();
                }
            };

            socket.OnClose = () =>
            {
                clients.TryRemove(socket, out _);
                Console.WriteLine("Client getrennt.");
            };

            socket.OnError = exception =>
            {
                Console.WriteLine("WebSocket-Fehler: " + exception.Message);
            };

            socket.OnMessage = async message =>
            {
                var NewMesssage = JsonSerializer.Deserialize<ClientMessage>(message);

                if (NewMesssage.Aktion == "Restart")
                {
                    Running = false;
                    await Task.Delay(50); // Warten bis Physikschleife garantiert beendet ist
                    ResetPhysik();
                    Running = true;
                    _ = updateLoop(clients); // Task im Hintergrund ausführen (nicht blockieren!)
                }
                else if (NewMesssage.Aktion == "Upload")
                {
                    var options = new JsonSerializerOptions { IncludeFields = true };
                    RetrunKi uploadedKi = JsonSerializer.Deserialize<RetrunKi>(
                        NewMesssage.Inhalt,
                        options
                    );
                    if (
                        uploadedKi.Gewichte_1?.Length != 208
                        || uploadedKi.Gewichte_2?.Length != 312
                        || uploadedKi.Gewichte_3?.Length != 12
                        || uploadedKi.Bias1?.Length != 26
                        || uploadedKi.Bias2?.Length != 12
                    )
                    {
                        throw new JsonException("Ungültige KI-Daten: Array-Längen stimmen nicht.");
                    }

                    GetKiC(uploadedKi);
                    ResetPhysik();
                }
                else if (NewMesssage.Aktion == "Download")
                {
                    GetKi(clients);
                }
                else if (NewMesssage.Aktion == "Starte Training")
                {
                    if (TrainingAktive != true)
                    {
                        Running = false;
                        TrainingAktive = true;
                        await Task.Delay(50);

                        StarteTrainning();

                        // Startet die eigentliche Trainingsschleife
                        trainingTask = Task.Run(() => StarteTraining());

                        trainingStatusAbbruch?.Cancel();
                        trainingStatusAbbruch = new CancellationTokenSource();
                        _ = SendeTrainingsStatus(clients, trainingStatusAbbruch.Token);
                    }
                }
                else if (NewMesssage.Aktion == "Stoppe Training")
                {
                    if (TrainingAktive != false)
                    {
                        TrainingAktive = false;
                        StoppeTrainning();
                        if (trainingTask != null)
                        {
                            await trainingTask;
                            trainingTask = null;
                        }
                        trainingStatusAbbruch?.Cancel();
                        Running = true;
                        _ = updateLoop(clients);
                    }
                }
                else if (NewMesssage.Aktion == "Training Neustarten")
                {
                    if (TrainingAktive)
                    {
                        TrainingAktive = false;
                        StoppeTrainning();
                        if (trainingTask != null)
                        {
                            await trainingTask;
                            trainingTask = null;
                        }
                    }

                    TrainingNeuStarten();
                    Running = false;
                    TrainingAktive = true;
                    StarteTrainning();
                    trainingTask = Task.Run(() => StarteTraining());

                    trainingStatusAbbruch?.Cancel();
                    trainingStatusAbbruch = new CancellationTokenSource();
                    _ = SendeTrainingsStatus(clients, trainingStatusAbbruch.Token);
                }
            };
        });

        Console.WriteLine("Warte auf Client...");
        trainingStatusAbbruch?.Cancel();

        _ = updateLoop(clients);

        await Task.Delay(Timeout.Infinite);
    }

    public static void GetKi(ConcurrentDictionary<IWebSocketConnection, byte> clients)
    {
        RetrunKi NewKi = HoleKi();
        var options = new JsonSerializerOptions { WriteIndented = true, IncludeFields = true };
        string jsonString = JsonSerializer.Serialize(NewKi, options);
        Console.WriteLine(jsonString);
        var Ki = new KiPaket { Version = 2, KiDaten = jsonString };

        string json = JsonSerializer.Serialize(Ki);
        foreach (var client in clients.Keys)
        {
            if (!client.IsAvailable)
            {
                clients.TryRemove(client, out _);
                continue;
            }

            try
            {
                client.Send(json);
                Console.WriteLine("Gesendet an Client."); // Habe den großen JSON-String im Log ausgeblendet der Übersicht halber
            }
            catch (Exception ex)
            {
                Console.WriteLine("Fehler beim Senden an Client: " + ex.Message);
                clients.TryRemove(client, out _);
            }
        }
    }

    public static async Task SendeTrainingsStatus(
        ConcurrentDictionary<IWebSocketConnection, byte> clients,
        CancellationToken abbruch
    )
    {
        int letzteGeneration = -1;
        try
        {
            while (!abbruch.IsCancellationRequested)
            {
                HoleTrainingsStatus(out TrainingsStatus status);
                if (status.Generation != letzteGeneration)
                {
                    letzteGeneration = status.Generation;
                    var paket = new TrainingsStatusPaket
                    {
                        Version = 3,
                        Generation = status.Generation,
                        BesteFitness = status.BesteFitness,
                        UeberlebtSpiel1 = status.UeberlebtSpiel1,
                        UeberlebtSpiel2 = status.UeberlebtSpiel2,
                    };
                    string json = JsonSerializer.Serialize(paket);

                    foreach (var client in clients.Keys)
                    {
                        if (!client.IsAvailable)
                        {
                            clients.TryRemove(client, out _);
                            continue;
                        }

                        try
                        {
                            client.Send(json);
                        }
                        catch (Exception ex)
                        {
                            Console.WriteLine(
                                "Fehler beim Senden des Trainingsstatus: " + ex.Message
                            );
                            clients.TryRemove(client, out _);
                        }
                    }
                }

                await Task.Delay(100, abbruch);
            }
        }
        catch (OperationCanceledException) { }
        catch (Exception ex)
        {
            Console.WriteLine("Fehler beim Lesen des Trainingsstatus: " + ex.Message);
        }
    }

    public static async Task updateLoop(ConcurrentDictionary<IWebSocketConnection, byte> clients)
    {
        if (torSchonOffen == 0)
        {
            await ersterClientTor.WaitAsync();
        }

        ResetPhysik();
        Console.WriteLine("Update-Loop (Physik) gestartet.");

        while (Running == true)
        {
            MachEinenLiveSchritt();
            double WagenX = HoleWagenPosition();
            double Winkel1 = HoleWinkel1();
            double Winkel2 = HoleWinkel2();

            var paket = new DatenPaket
            {
                Version = 1,
                WagenX = WagenX,
                Winkel1 = Winkel1,
                Winkel2 = Winkel2,
            };
            string json = JsonSerializer.Serialize(paket);

            foreach (var client in clients.Keys)
            {
                if (!client.IsAvailable)
                {
                    clients.TryRemove(client, out _);
                    continue;
                }

                try
                {
                    client.Send(json);
                }
                catch (Exception ex)
                {
                    Console.WriteLine("Fehler beim Senden an Client: " + ex.Message);
                    clients.TryRemove(client, out _);
                }
            }

            // 8 Millisekunden Wartezeit für ca. 120 Updates pro Sekunde
            await Task.Delay(8);
        }

        Console.WriteLine("Update-Loop (Physik) beendet.");
    }
}
