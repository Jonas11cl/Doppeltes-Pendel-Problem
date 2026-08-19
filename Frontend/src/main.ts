const canvas = document.getElementById("pendelCanvas") as HTMLCanvasElement;
canvas.width = 960;
canvas.height = 560;
const ctx = canvas.getContext("2d")!;

// Verbindung zum C# Server
const verbindung = new WebSocket("ws://127.0.0.1:8080");

interface ChatMessage {
  WagenX: number;
  Winkel1: number;
  Winkel2: number;
}

verbindung.addEventListener("message", (event) => {
  try {
    const message: ChatMessage = JSON.parse(event.data) as ChatMessage;
    const wagenX = Number(message.WagenX);
    const winkel1Rad = Number(message.Winkel1);
    const winkel2Rad = Number(message.Winkel2);

    if (
      !Number.isFinite(wagenX) ||
      !Number.isFinite(winkel1Rad) ||
      !Number.isFinite(winkel2Rad)
    ) {
      console.warn("Ungültige Daten empfangen:", event.data);
      return;
    }

    // --- DIE FESTE KAMERA (Meter in Pixel umrechnen) ---
    // Die Mitte des Bildschirms ist X = 480 Pixel (entspricht in der Physik X = 0.0 Meter)
    const bildschirmMitte = canvas.width / 2;

    // 1 Meter in der Physik-Engine sind 160 Pixel auf dem Bildschirm.
    // Bei max. 2.4 Metern nach rechts bewegt sich der Wagen also um 384 Pixel
    // (480 + 384 = 864), bleibt also exakt auf der grauen Schiene!
    const pixelProMeter = 160;

    // Echte Bildschirm-Koordinate berechnen
    const sichtbarerX = bildschirmMitte + wagenX * pixelProMeter;

    // Zeichnen
    zeichnePendel(sichtbarerX, winkel1Rad, winkel2Rad);
  } catch (error) {
    console.error("Fehler beim Parsen:", error);
  }
});

// Eine wiederverwendbare Funktion zum Zeichnen des Pendels
function zeichnePendel(wagenX: number, winkel1Rad: number, winkel2Rad: number) {
  // 1. Leinwand komplett sauber wischen
  ctx.clearRect(0, 0, canvas.width, canvas.height);

  // 2. Schiene zeichnen (Graue Linie unten)
  const schieneY = 380;
  ctx.beginPath();
  ctx.moveTo(80, schieneY);
  ctx.lineTo(880, schieneY);
  ctx.strokeStyle = "#ccc";
  ctx.lineWidth = 4;
  ctx.stroke();

  // 3. Wagen zeichnen (Blaues Rechteck, zentriert auf wagenX)
  const wagenBreite = 80;
  const wagenHoehe = 40;
  const wagenTopY = schieneY - wagenHoehe / 2;

  ctx.fillStyle = "#007acc";
  ctx.fillRect(wagenX - wagenBreite / 2, wagenTopY, wagenBreite, wagenHoehe);

  // 4. Drehpunkt (Obere Mitte des Wagens)
  const pivotX = wagenX;
  const pivotY = wagenTopY;

  // 5. Spitze der Stange_1 berechnen (Trigonometrie: Sinus & Kosinus)
  const Stange_1nLaenge = 160;
  const spitzeX = pivotX + Stange_1nLaenge * Math.sin(winkel1Rad);
  // WICHTIG: Minus, weil Y im Browser von oben nach unten zählt!
  const spitzeY = pivotY - Stange_1nLaenge * Math.cos(winkel1Rad);

  // 6. Stange_1 zeichnen (Dunkle Linie)
  ctx.beginPath();
  ctx.moveTo(pivotX, pivotY);
  ctx.lineTo(spitzeX, spitzeY);
  ctx.strokeStyle = "#333";
  ctx.lineWidth = 6;
  ctx.stroke();

  // 7. Gewicht / Kugel zeichnen (Roter Kreis an der Spitze)
  ctx.beginPath();
  // arc(X, Y, Radius, Startwinkel, Endwinkel)
  ctx.arc(spitzeX, spitzeY, 16, 0, 2 * Math.PI);
  ctx.fillStyle = "#e74c3c";
  ctx.fill();
  ctx.strokeStyle = "#c0392b";
  ctx.lineWidth = 2;
  ctx.stroke();
  // ==========================================
  // NEU: DAS ZWEITE PENDEL
  // ==========================================

  // 8. Spitze der zweiten Stange berechnen
  // Der Startpunkt ist hierbei die Spitze der ersten Stange!
  const Stange_2nLaenge = 160;
  const spitze2X = spitzeX + Stange_2nLaenge * Math.sin(winkel2Rad);
  const spitze2Y = spitzeY - Stange_2nLaenge * Math.cos(winkel2Rad); // Wieder Minus wegen Y-Achse

  // 9. Stange 2 zeichnen (vielleicht etwas dunkler zur Unterscheidung)
  ctx.beginPath();
  ctx.moveTo(spitzeX, spitzeY); // Startet exakt in der Mitte der ersten Kugel
  ctx.lineTo(spitze2X, spitze2Y); // Geht zur neuen Spitze
  ctx.strokeStyle = "#555";
  ctx.lineWidth = 6;
  ctx.stroke();

  // 10. Gewicht 2 zeichnen (z. B. ein oranger Kreis)
  ctx.beginPath();
  ctx.arc(spitze2X, spitze2Y, 16, 0, 2 * Math.PI);
  ctx.fillStyle = "#e67e22"; // Orange
  ctx.fill();
  ctx.strokeStyle = "#d35400";
  ctx.lineWidth = 2;
  ctx.stroke();
}

// Start-Bild zeichnen, bevor die Verbindung steht
zeichnePendel(480, 0, 0);
