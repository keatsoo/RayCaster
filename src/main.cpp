#include "../SDL2_TTF/include/SDL2/SDL_ttf.h"
#include "SDL2/SDL.h"
#include <iostream>

using namespace std;

SDL_Window *window;
SDL_Renderer *renderer;

int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        cerr << "Oh non! Il y a eu une erreur à l'initialisation de SDL2! La voilà : " << SDL_GetError() << endl;
        return -1;
    }

    if (TTF_Init() != 0) {
        cerr << "Oh non! Il y a eu une erreur à l'initialisation du module de texte (SDL2_TTF)! La voilà : " << TTF_GetError() << endl;
        return -1;
    }

    TTF_Quit();
    SDL_Quit();
    return 0;
}
