using System;
using System.Collections.Concurrent;
using System.Dynamic;
using System.Runtime.InteropServices;
using System.Text.Json;
using System.Threading;
using System.Threading.Tasks;
using Fleck;

public class ClientPaket
{
    public int Version { get; set; }
    public string Inhalt { get; set; } // Die eigentlichen Daten vom Client
}

public class ClientMessage
{
    public string Aktion { get; set; }
    public string Inhalt { get; set; } // Die eigentlichen Daten vom Client
}

public class DatenPaket
{
    public int Version { get; set; }
    public double WagenX { get; set; }
    public double Winkel1 { get; set; }
    public double Winkel2 { get; set; }
}

public class KiPaket
{
    public int Version { get; set; }
    public string KiDaten { get; set; }
}

[StructLayout(LayoutKind.Sequential)]
public struct RetrunKi
{
    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 8)]
    public double[] Gewichte_1;

    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 26)]
    public double[] Gewichte_2;

    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 12)]
    public double[] Gewichte_3;

    public double Bias1;
    public double Bias2;
    public double Bias3;
};

class Program
{
    [DllImport("Engin.dll")]
    public static extern void StarteTraining();

    [DllImport("Engin.dll")]
    public static extern void MachEinenLiveSchritt();

    [DllImport("Engin.dll")]
    public static extern double HoleWagenPosition();

    [DllImport("Engin.dll")]
    public static extern double HoleWinkel1();

    [DllImport("Engin.dll")]
    public static extern double HoleWinkel2();

    [DllImport("Engin.dll")]
    public static extern RetrunKi HoleKi();

    [DllImport("Engin.dll")]
    public static extern void ResetPhysik();

    [DllImport("Engin.dll")]
    public static extern void GetKiC(RetrunKi NeueKi);

    private static readonly SemaphoreSlim ersterClientTor = new SemaphoreSlim(0);
    private static int torSchonOffen = 0;
    public static bool Running = true;

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
                    ResetPhysik();
                    Running = true;
                    await updateLoop(clients);
                }
                else if (NewMesssage.Aktion == "Upload")
                {
                    var options = new JsonSerializerOptions { IncludeFields = true };
                    RetrunKi uploadedKi = JsonSerializer.Deserialize<RetrunKi>(
                        NewMesssage.Inhalt,
                        options
                    );
                    if (
                        uploadedKi.Gewichte_1?.Length != 8
                        || uploadedKi.Gewichte_2?.Length != 26
                        || uploadedKi.Gewichte_3?.Length != 12
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
            };
        });
        StarteTraining();

        Console.WriteLine("Warte auf Client");
        await updateLoop(clients);
    }

    public static async void GetKi(ConcurrentDictionary<IWebSocketConnection, byte> clients)
    {
        RetrunKi NewKi;

        NewKi = HoleKi();
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
                Console.WriteLine("Gesendet an Client: " + json);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Fehler beim Senden an Client: " + ex.Message);
                clients.TryRemove(client, out _);
            }
        }
    }

    public static async Task updateLoop(ConcurrentDictionary<IWebSocketConnection, byte> clients)
    {
        // Blockiert hier, bis OnOpen das Tor freigibt
        await ersterClientTor.WaitAsync();
        ResetPhysik();
        Console.WriteLine("Erster Client verbunden – Loop startet.");

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
                    Console.WriteLine("Gesendet an Client: " + json);
                }
                catch (Exception ex)
                {
                    Console.WriteLine("Fehler beim Senden an Client: " + ex.Message);
                    clients.TryRemove(client, out _);
                }
            }

            await Task.Delay(8);
        }
    }
}
