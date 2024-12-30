#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_syswm.h>
#include <cmath>
#include <ctime>
#include <string>
#include <cstdio>
#include <windows.h> // Necessario per la gestione delle finestre in Windows
#include <windows.h>

void SetDPIAwareness() {
    typedef HRESULT(WINAPI *SetProcessDpiAwarenessContext_t)(DPI_AWARENESS_CONTEXT);
    HMODULE user32 = LoadLibrary("user32.dll");
    if (user32) {
        auto SetProcessDpiAwarenessContext = (SetProcessDpiAwarenessContext_t)GetProcAddress(user32, "SetProcessDpiAwarenessContext");
        if (SetProcessDpiAwarenessContext) {
            SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
        }
        FreeLibrary(user32);
    }
}

const int SCREEN_WIDTH = 500;
const int SCREEN_HEIGHT = 500;
const int CLOCK_RADIUS = 200;
HWND GetWindowFromSDL(SDL_Window *window);

// Funzione per disegnare il valore sul quadrante
void drawValueOnQuadrant(SDL_Renderer *renderer, TTF_Font *font, int centerX, int centerY, double angle, int value, SDL_Color color, double radiusMultiplier) {
    double rad = angle * M_PI / 180.0;
    int x = centerX + CLOCK_RADIUS * radiusMultiplier * cos(rad);
    int y = centerY + CLOCK_RADIUS * radiusMultiplier * sin(rad);

    std::string valueStr = std::to_string(value);

    SDL_Surface *textSurface = TTF_RenderText_Solid(font, valueStr.c_str(), color);
    if (textSurface) {
        SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_Rect textRect = {x - textSurface->w / 2, y - textSurface->h / 2, textSurface->w, textSurface->h};
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
    }
}


void drawHand(SDL_Renderer *renderer, TTF_Font *font, int centerX, int centerY, double angle, int length, SDL_Color color, const std::string &label, Uint8 alpha, double textPosition) {
    double rad = angle * M_PI / 180.0;

    // Calcola le coordinate di inizio e fine della lancetta
    int endX = centerX + length * cos(rad);
    int endY = centerY + length * sin(rad);

    // Disegna la lancetta
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 35);
    //SDL_RenderDrawLine(renderer, centerX, centerY, endX, endY);

    // Calcola la posizione del testo al centro della lancetta
    //int textX = centerX + (length / 2) * cos(rad);
    //int textY = centerY + (length / 2) * sin(rad);
    int textX = centerX + (length * textPosition) * cos(rad);
    int textY = centerY + (length * textPosition) * sin(rad);
    // Imposta il colore del testo con alpha
    SDL_Color textColor = {color.r, color.g, color.b, alpha};

    // Renderizza il testo con trasparenza
    SDL_Surface *textSurface = TTF_RenderText_Blended(font, label.c_str(), textColor);
    if (textSurface) {
        SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

        // Imposta l'alpha della texture
        SDL_SetTextureAlphaMod(textTexture, alpha);

        // Ottieni la dimensione del testo
        SDL_Rect textRect = {textX - textSurface->w / 2, textY - textSurface->h / 2, textSurface->w, textSurface->h};

        // Disegna il testo orientato lungo l'angolo della lancetta
        SDL_RenderCopyEx(renderer, textTexture, NULL, &textRect, angle, NULL, SDL_FLIP_NONE);

        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
    }
}






int main(int argc, char *argv[]) {
    
    SetDPIAwareness(); //Agisce sulla risoluzione
    
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    if (TTF_Init() == -1) {
        printf("TTF could not initialize! TTF_Error: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Orologio con Valori sul Quadrante", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_BORDERLESS);
    if (!window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Ottieni l'hwnd della finestra SDL
    HWND hwnd = GetWindowFromSDL(window);

    // Rendi la finestra trasparente
 //   SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
  //  SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA); // Full transparency

    // Imposta la finestra sempre sullo sfondo
    //SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
RECT desktopRect;
GetWindowRect(GetDesktopWindow(), &desktopRect); // Get desktop dimensions

int windowWidth = SCREEN_WIDTH;
int windowHeight = SCREEN_HEIGHT;

// Calculate centered position
//int xPos = (desktopRect.right - windowWidth) / 2; // Center horizontally
//int yPos = (desktopRect.bottom - windowHeight) / 2; // Center vertically
int xPos = (desktopRect.right - windowWidth); // Center horizontally
int yPos = (desktopRect.bottom - windowHeight)/2; // Center vertically

SetWindowPos(hwnd, HWND_BOTTOM, xPos, yPos, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);


    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    TTF_Font *font = TTF_OpenFont("C:/Windows/Fonts/ARLRDBD.TTF", 24); // Assicurati che il percorso del font sia corretto
    TTF_Font *fontLancette = TTF_OpenFont("C:/Windows/Fonts/Arial.TTF", 16);
    if (!font) {
        printf("Failed to load font! TTF_Error: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        // Ottieni l'ora corrente
        time_t now = time(0);
        tm *ltm = localtime(&now);
        int hours = ltm->tm_hour % 12;
        int minutes = ltm->tm_min;
        int seconds = ltm->tm_sec;

        // Angoli per i valori
        double hourAngle = hours * 30 - 90;
        double minuteAngle = minutes * 6 - 90;
        double secondAngle = seconds * 6 - 90;

        // Pulizia dello schermo con trasparenza
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0); // Trasparente
        SDL_RenderClear(renderer);

        // Disegna le lancette
// Disegna le lancette con le scritte
drawHand(renderer, fontLancette, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, hourAngle, CLOCK_RADIUS * 1, {255,255 ,255 }, "ore",255,0.1);
drawHand(renderer, fontLancette, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, minuteAngle, CLOCK_RADIUS * 1, {255, 255,255 }, "min",255,0.5);
drawHand(renderer, fontLancette, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, secondAngle, CLOCK_RADIUS * 1, {255,255 , 255}, "sec",255,0.85);
        
        // Disegna i valori sul quadrante
        drawValueOnQuadrant(renderer, font, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, hourAngle, hours, {252,182 ,3 }, 0.25);    // Ore (rosso)
        drawValueOnQuadrant(renderer, font, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, minuteAngle, minutes, {252,182 ,3 }, 0.68); // Minuti (verde)
        drawValueOnQuadrant(renderer, font, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, secondAngle, seconds, {252,182 ,3 }, 1);  // Secondi (blu)

        // Aggiorna lo schermo
        SDL_RenderPresent(renderer);

        SDL_Delay(1000 / 60); // 60 FPS
    }

    // Pulizia delle risorse
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}

HWND GetWindowFromSDL(SDL_Window *window) {
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    if (SDL_GetWindowWMInfo(window, &wmInfo)) {
        return wmInfo.info.win.window;
    }
    return NULL;
}

