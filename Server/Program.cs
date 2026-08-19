using System;
using System.Collections.Concurrent;
using System.Runtime.InteropServices;
using System.Text.Json;
using System.Threading;
using System.Threading.Tasks;
using Fleck;

public class DatenPaket
{
    public double WagenX { get; set; }
    public double Winkel1 { get; set; }
    public double Winkel2 { get; set; }
}

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

    private static readonly SemaphoreSlim ersterClientTor = new SemaphoreSlim(0);
    private static int torSchonOffen = 0;

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

            socket.OnMessage = message =>
            {
                Console.WriteLine("Empfangene Nachricht vom Client: " + message);
            };
        });

        StarteTraining();

        Console.WriteLine("Warte auf Client");
        await updateLoop(clients);
    }

    public static async Task updateLoop(ConcurrentDictionary<IWebSocketConnection, byte> clients)
    {
        // Blockiert hier, bis OnOpen das Tor freigibt
        await ersterClientTor.WaitAsync();
        Console.WriteLine("Erster Client verbunden – Loop startet.");

        while (true)
        {
            MachEinenLiveSchritt();
            double WagenX = HoleWagenPosition();
            double Winkel1 = HoleWinkel1();
            double Winkel2 = HoleWinkel2();
            var paket = new DatenPaket
            {
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
