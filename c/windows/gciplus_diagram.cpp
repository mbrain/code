/*
g++ paint_timeline.cpp -o diagram -lgdiplus -lgdi32 -lole32
*/
#include <windows.h>
#include <gdiplus.h>
#include <iostream>
#include <vector>
#include <algorithm>  // Für std::max und std::min
#include <string>
#include <sstream>  // Für std::wostringstream

using namespace Gdiplus;

int main() {
	
    // GDI+ initialisieren
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // Erstelle ein Bitmap-Objekt
    int width = 640, height = 480;
    int padding = 40; // Padding für den Rand (Rahmen)
    Bitmap bitmap(width, height, PixelFormat32bppARGB);

    // Zeichne auf das Bitmap
    Graphics graphics(&bitmap);
    graphics.Clear(Color(255, 255, 255, 255)); // Hintergrund: Weiß

    // Zeitlinien-Daten als Array
    std::vector<int> timelineData = {50, 80, 60, 480, 560, 990, 120, 100, 150, 180, 220, 200};
    int numPoints = timelineData.size();

    // X-Achsen-Beschriftungen
    std::vector<std::wstring> xLabels = {
        L"Jan", L"Feb", L"Mar", L"Apr", L"May", L"Jun", L"Jul", L"Aug", L"Sep", L"Oct", L"Nov", L"Dec"
    };

    // Y-Spacing Berechnen (Skalierung der Y-Werte)
    int maxY = *std::max_element(timelineData.begin(), timelineData.end());  // Höchster Wert im Array
    int minY = *std::min_element(timelineData.begin(), timelineData.end());  // Tiefster Wert im Array

    // Berechnung der Y-Range mit Anpassung für die Achse bei 0
    int adjustedMinY = std::min(0, minY); // Startpunkt der Y-Achse bei 0 oder dem tatsächlichen Minimum
    int yRange = maxY - adjustedMinY;     // Bereich der Y-Werte anpassen

    // Stift für die Zeitlinie
    Pen pen(Color(255, 0, 0, 255));  // Roter Stift für die Linie

    // Abstand zwischen den Punkten auf der X-Achse, angepasst für Padding
    int xSpacing = (width - 2 * padding) / (numPoints + 1);

    // Zeichne die Zeitlinie (Linien zwischen den Punkten)
    for (int i = 1; i < numPoints; ++i) {
        int x1 = padding + i * xSpacing;
        int y1 = height - padding - ((timelineData[i - 1] - adjustedMinY) * (height - 2 * padding) / yRange);  // Y-Wert skalieren
        int x2 = padding + (i + 1) * xSpacing;
        int y2 = height - padding - ((timelineData[i] - adjustedMinY) * (height - 2 * padding) / yRange);      // Y-Wert skalieren

        graphics.DrawLine(&pen, x1, y1, x2, y2);  // Linie zwischen den Punkten zeichnen
    }

    // Zeichne Marker für jeden Punkt
    SolidBrush brush(Color(255, 0, 255, 0));  // Grüne Marker für die Datenpunkte
    for (int i = 0; i < numPoints; ++i) {
        int x = padding + (i + 1) * xSpacing;
        int y = height - padding - ((timelineData[i] - adjustedMinY) * (height - 2 * padding) / yRange);  // Y-Wert skalieren
        graphics.FillEllipse(&brush, x - 5, y - 5, 10, 10);  // Kleine Kreise an den Punkten
    }

    // Zeichne Y-Achse mit Skala
    Pen axisPen(Color(255, 0, 0, 0));  // Schwarzer Stift für die Achse
    graphics.DrawLine(&axisPen, padding, padding, padding, height - padding); // Y-Achse zeichnen

    // Erstelle das Font-Objekt einmal außerhalb der Schleife
    Font font(L"Arial", 8);  // Font-Objekt erstellen

    // Erstelle den SolidBrush für die Textfarbe einmal außerhalb der Schleife
    SolidBrush axisLabelBrush(Color(255, 0, 0, 0));  // Schwarzer Brush für Skalenwerte

    int numLabels = 5;  // Anzahl der Skalenmarkierungen
    int labelSpacing = (height - 2 * padding) / numLabels;

    for (int i = 0; i <= numLabels; ++i) {
        int yLabelPos = height - padding - (i * labelSpacing);
        int value = adjustedMinY + i * (yRange / numLabels); // Werte relativ zu adjustedMinY berechnen

        // Zeichne die Markierung auf der Y-Achse
        graphics.FillEllipse(&axisLabelBrush, padding - 5, yLabelPos - 3, 6, 6);  // Markierung
        
        // Verwende std::wostringstream für die Umwandlung in wstring
        std::wostringstream oss;
        oss << value;
        std::wstring label = oss.str();
        
        // Verwende den SolidBrush außerhalb der Schleife für den Text
        graphics.DrawString(label.c_str(), -1, &font, PointF(padding + 10, yLabelPos - 10), &axisLabelBrush);  // Wert anzeigen
    }

    // Zeichne X-Achse mit Skala und Beschriftungen
    Pen xAxisPen(Color(255, 0, 0, 0));  // Schwarzer Stift für die X-Achse
    graphics.DrawLine(&xAxisPen, padding, height - padding, width - padding, height - padding); // X-Achse zeichnen

    // X-Achsen-Beschriftungen zeichnen
    for (int i = 0; i < numPoints; ++i) {
        int x = padding + (i + 1) * xSpacing;
        if (i < xLabels.size()) {  // Nur wenn es ein Label für den Punkt gibt
            std::wstring label = xLabels[i];

            // Zeichne das Label unterhalb der X-Achse
            graphics.DrawString(label.c_str(), -1, &font, PointF(x - 10, height - padding + 5), &axisLabelBrush);  // Wert anzeigen
        }
    }

    // Speichere als PNG
    CLSID clsid;
    wchar_t pngClsidStr[] = L"{557CF406-1A04-11D3-9A73-0000F81EF32E}"; // CLSID für PNG
    CLSIDFromString(pngClsidStr, &clsid);  // CLSID für PNG
    bitmap.Save(L"timeline.png", &clsid, NULL);

    // GDI+ beenden
    GdiplusShutdown(gdiplusToken);

    std::cout << "Bild wurde als timeline.png gespeichert.\n";
    return 0;
}
