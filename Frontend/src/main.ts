import { Chart, registerables } from "chart.js";

Chart.register(...registerables);

const canvas = document.getElementById("pendelCanvas") as HTMLCanvasElement;
canvas.width = 960;
canvas.height = 560;
const ctx = canvas.getContext("2d")!;
const Restart_Button = document.getElementById(
  "Reset-Button",
) as HTMLButtonElement;
const Download_Button = document.getElementById(
  "Download-Button",
) as HTMLButtonElement;
const Stardt_Boutton = document.getElementById(
  `Starte-Button`,
) as HTMLButtonElement;
const stopButton = document.getElementById("Stop-Button") as HTMLButtonElement;
const trainingPanel = document.getElementById("training-panel") as HTMLElement;
const generationValue = document.getElementById(
  "generation-value",
) as HTMLElement;
const fitnessCanvas = document.getElementById(
  "fitness-chart",
) as HTMLCanvasElement;
const survivalCanvas = document.getElementById(
  "survival-chart",
) as HTMLCanvasElement;
// Verbindung zum C# Server
const verbindung = new WebSocket("ws://127.0.0.1:8080");

// Interface passt zu Ihrer C#-Klasse "DatenPaket"
interface ChatMessage {
  Version: number;
  WagenX: number;
  Winkel1: number;
  Winkel2: number;
}

interface Trainigsmessge {
  Version: number;
  Generation: number;
  BesteFitness: number;
  UeberlebtSpiel1: number;
  UeberlebtSpiel2: number;
}

const fitnessData: number[] = [];
const survivalGameOneData: number[] = [];
const survivalGameTwoData: number[] = [];
const generations: string[] = [];

const chartOptions = {
  responsive: true,
  maintainAspectRatio: false,
  animation: { duration: 350 },
  plugins: { legend: { labels: { color: "#d7e1e8" } } },
  scales: {
    x: { ticks: { color: "#93a5b1" }, grid: { color: "#29404a" } },
    y: { ticks: { color: "#93a5b1" }, grid: { color: "#29404a" } },
  },
};

const fitnessChart = new Chart(fitnessCanvas, {
  type: "line",
  data: {
    labels: generations,
    datasets: [
      {
        label: "Fitness",
        data: fitnessData,
        borderColor: "#66e3b4",
        backgroundColor: "rgba(102, 227, 180, 0.14)",
        fill: true,
        tension: 0.35,
      },
    ],
  },
  options: chartOptions,
});

const survivalChart = new Chart(survivalCanvas, {
  type: "line",
  data: {
    labels: generations,
    datasets: [
      {
        label: "Spiel 1",
        data: survivalGameOneData,
        borderColor: "#ffb45c",
        tension: 0.35,
      },
      {
        label: "Spiel 2",
        data: survivalGameTwoData,
        borderColor: "#78b7ff",
        tension: 0.35,
      },
    ],
  },
  options: chartOptions,
});

function zeigeTrainingsdaten(message: Trainigsmessge): void {
  generations.push(String(message.Generation));
  fitnessData.push(message.BesteFitness);
  survivalGameOneData.push(message.UeberlebtSpiel1);
  survivalGameTwoData.push(message.UeberlebtSpiel2);

  if (generations.length > 100) {
    generations.shift();
    fitnessData.shift();
    survivalGameOneData.shift();
    survivalGameTwoData.shift();
  }

  generationValue.textContent = String(message.Generation);
  fitnessChart.update();
  survivalChart.update();
}

function aktualisiereChartGroesse(): void {
  fitnessChart.resize();
  survivalChart.resize();
}
class ServerMessage {
  Aktion: string;
  Inhalt: string;

  constructor(aktion: string, inhalt: string) {
    this.Aktion = aktion;
    this.Inhalt = inhalt;
  }
}

// Interface passt zu Ihrer C#-Klasse "KiPaket"
interface KiMessage {
  Version: number;
  KiDaten: string; // Enthält den serialisierten C#-Struct "RetrunKi"
}

verbindung.addEventListener("message", (event) => {
  try {
    // 1. Den empfangenen Text-String in ein allgemeines Objekt umwandeln
    const rohDaten = JSON.parse(event.data);

    // 2. Prüfen, ob es sich um Version 1 (DatenPaket) handelt
    if (rohDaten.Version === 1) {
      const message = rohDaten as ChatMessage;
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
      const bildschirmMitte = canvas.width / 2;
      const pixelProMeter = 160;

      // Echte Bildschirm-Koordinate berechnen
      const sichtbarerX = bildschirmMitte + wagenX * pixelProMeter;

      // Zeichnen
      zeichnePendel(sichtbarerX, winkel1Rad, winkel2Rad);

      // 3. Prüfen, ob es sich um Version 2 (KiPaket) handelt
    } else if (rohDaten.Version === 2) {
      const message = rohDaten as KiMessage;

      // Nutzt das Feld "KiDaten" aus Ihrem C#-Code und speichert es direkt als Datei
      downloadJsonString(message.KiDaten, "Ki.json");
    } else if (rohDaten.Version === 3) {
      const message = rohDaten as Trainigsmessge;
      zeigeTrainingsdaten(message);
    } else {
      console.warn("Unbekannte Paket-Version erhalten:", rohDaten.Version);
      console.log(rohDaten);
    }
  } catch (error) {
    console.error("Fehler beim Parsen:", error);
  }
});

/**
 * Wandelt einen rohen JSON-Text direkt im Browser in eine Datei um und lädt sie herunter.
 * Da die KI-Daten direkt vom Server als Text kommen, ist kein 'fetch' (HTTP-Anfrage) nötig.
 */
function downloadJsonString(jsonInhalt: string, filename: string): void {
  try {
    // Erstellt ein Daten-Blob direkt aus dem Text-Inhalt
    const blob = new Blob([jsonInhalt], { type: "application/json" });
    const blobUrl = window.URL.createObjectURL(blob);

    const link = document.createElement("a");
    link.href = blobUrl;
    link.download = filename;
    link.style.display = "none";

    document.body.appendChild(link);
    link.click();

    document.body.removeChild(link);
    window.URL.revokeObjectURL(blobUrl);
  } catch (error) {
    console.error("Download der KI-Datei fehlgeschlagen:", error);
  }
}

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
  ctx.arc(spitzeX, spitzeY, 16, 0, 2 * Math.PI);
  ctx.fillStyle = "#e74c3c";
  ctx.fill();
  ctx.strokeStyle = "#c0392b";
  ctx.lineWidth = 2;
  ctx.stroke();

  // ==========================================
  // DAS ZWEITE PENDEL
  // ==========================================

  // 8. Spitze der zweiten Stange berechnen
  const Stange_2nLaenge = 160;
  const spitze2X = spitzeX + Stange_2nLaenge * Math.sin(winkel2Rad);
  const spitze2Y = spitzeY - Stange_2nLaenge * Math.cos(winkel2Rad);

  // 9. Stange 2 zeichnen
  ctx.beginPath();
  ctx.moveTo(spitzeX, spitzeY);
  ctx.lineTo(spitze2X, spitze2Y);
  ctx.strokeStyle = "#555";
  ctx.lineWidth = 6;
  ctx.stroke();

  // 10. Gewicht 2 zeichnen (z. B. ein oranger Kreis)
  ctx.beginPath();
  ctx.arc(spitze2X, spitze2Y, 16, 0, 2 * Math.PI);
  ctx.fillStyle = "#e67e22";
  ctx.fill();
  ctx.strokeStyle = "#d35400";
  ctx.lineWidth = 2;
  ctx.stroke();
}

// Start-Bild zeichnen, bevor die Verbindung steht
zeichnePendel(480, 0, 0);

Restart_Button.addEventListener("click", () => {
  let Message = new ServerMessage("Restart", "");
  let JM = JSON.stringify(Message);
  verbindung.send(JM);
});

// Hole die vorhandenen HTML-Elemente
const uploadBtn = document.getElementById("Upload-Button") as HTMLButtonElement;

uploadBtn.addEventListener("click", () => {
  // 1. Verstecke das Canvas
  canvas.style.display = "none";

  // Prüfen, ob die Zone schon existiert (verhindert doppeltes Klicken)
  if (document.getElementById("upload-zone")) return;

  // 2. Erstelle den Drag & Drop Container
  const uploadZone = document.createElement("div");
  uploadZone.id = "upload-zone";
  uploadZone.className = "upload-zone";

  // HTML-Struktur innerhalb der Upload-Zone
  uploadZone.innerHTML = `
        <h2 class="upload-text">Datei hier ablegen<br>oder klicken, um den Explorer zu öffnen</h2>
        <input type="file" id="file-input" style="display: none;" />
        <p id="file-name" style="color: #666; font-family: sans-serif; font-weight: bold;"></p>
        <button class="Send-Button" id="Send-Button">Senden</button>
    `;

  // Füge die Zone direkt nach den Buttons (vor dem Canvas) in den DOM ein
  canvas.parentNode?.insertBefore(uploadZone, canvas);

  // 3. Referenzen auf die neuen Elemente holen
  const fileInput = uploadZone.querySelector("#file-input") as HTMLInputElement;
  const sendBtn = uploadZone.querySelector("#Send-Button") as HTMLButtonElement;
  const fileNameDisplay = uploadZone.querySelector(
    "#file-name",
  ) as HTMLParagraphElement;
  let selectedFile: File | null = null;

  // Funktion: UI updaten, wenn Datei ausgewählt wurde
  const handleFile = (file: File) => {
    selectedFile = file;
    fileNameDisplay.textContent = `Ausgewählt: ${file.name}`;
    sendBtn.style.display = "block"; // Senden-Button einblenden
  };

  // --- EVENT LISTENER FÜR DEN EXPLORER ---
  uploadZone.addEventListener("click", (e) => {
    // Öffne Explorer nur, wenn NICHT auf den Senden-Button geklickt wurde
    if (e.target !== sendBtn) {
      fileInput.click();
    }
  });

  fileInput.addEventListener("change", (e) => {
    const target = e.target as HTMLInputElement;
    if (target.files && target.files.length > 0) {
      handleFile(target.files[0]);
    }
  });

  // --- EVENT LISTENER FÜR DRAG & DROP ---
  uploadZone.addEventListener("dragover", (e) => {
    e.preventDefault();
    uploadZone.classList.add("dragover");
  });

  uploadZone.addEventListener("dragleave", () => {
    uploadZone.classList.remove("dragover");
  });

  uploadZone.addEventListener("drop", (e) => {
    e.preventDefault();
    uploadZone.classList.remove("dragover");

    if (e.dataTransfer && e.dataTransfer.files.length > 0) {
      handleFile(e.dataTransfer.files[0]);
    }
  });

  // --- EVENT LISTENER FÜR DEN SENDEN-BUTTON ---
  sendBtn.addEventListener("click", async (e) => {
    e.stopPropagation(); // Verhindert, dass sich der Explorer nochmal öffnet

    if (!selectedFile) return;

    const text = await selectedFile.text();
    const message = JSON.stringify(new ServerMessage("Upload", text));
    verbindung.send(message);
    // 4. Zurück zum Normalzustand (Upload-Zone löschen, Canvas zeigen)
    uploadZone.remove();
    canvas.style.display = "block";
  });
});

Download_Button.addEventListener("click", () => {
  let Message = new ServerMessage("Download", "");
  let JM = JSON.stringify(Message);
  verbindung.send(JM);
});

Stardt_Boutton.addEventListener("click", () => {
  canvas.style.display = "none";
  trainingPanel.hidden = false;
  requestAnimationFrame(aktualisiereChartGroesse);
  Stardt_Boutton.hidden = true;
  stopButton.hidden = false;
  generations.length = 0;
  fitnessData.length = 0;
  survivalGameOneData.length = 0;
  survivalGameTwoData.length = 0;
  fitnessChart.update();
  survivalChart.update();
  let Message = new ServerMessage("Starte Training", "");
  let JM = JSON.stringify(Message);
  verbindung.send(JM);
});

stopButton.addEventListener("click", () => {
  let Message = new ServerMessage("Stoppe Training", "");
  verbindung.send(JSON.stringify(Message));
  trainingPanel.hidden = true;
  canvas.style.display = "block";
  Stardt_Boutton.hidden = false;
  stopButton.hidden = true;
});
