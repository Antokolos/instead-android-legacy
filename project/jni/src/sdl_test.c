#include "SDL.h"
#include "SDL_ttf.h" 

SDL_Rect messageRect;

static void draw_scene(SDL_Renderer *renderer, SDL_Texture *message)
{
    /* Clear the background to background color */
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(renderer);

    SDL_RenderCopy(renderer, message, NULL, &messageRect);
    SDL_RenderPresent(renderer);
}

SDL_Color textColor = { 0, 0, 0 };

int test_sdl(int argc, char* argv[]) { 
//void test_sdl(char* msg_text) {
  SDL_Window *window; 
  SDL_Renderer *renderer; 
  SDL_Rect rect={50,50,50,50}; 
  SDL_Event event;
  //The font that's going to be used
  TTF_Font *font = NULL; 
  SDL_Surface *text = NULL;
  SDL_Texture *message = NULL;

  if (SDL_Init(SDL_INIT_VIDEO) < 0) return 1; 
  if( TTF_Init() == -1 ) {
    return 1;    
  }
  //Open the font
  font = TTF_OpenFont( "/sdcard/STEINEMU.ttf", 28 );
    
  window = SDL_CreateWindow("Hallo", 100, 100, 200, 200, SDL_WINDOW_SHOWN); 
  renderer = SDL_CreateRenderer(window, -1,0); 
  //SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255); // red 
  //SDL_RenderClear(renderer); 

  //SDL_SetRenderDrawColor(renderer,100,100,100,255); // gray 
  //SDL_RenderFillRect(renderer,&rect); 
  //SDL_RenderPresent(renderer); 
  
  //Render the text
  text = TTF_RenderText_Solid( font, argv[0], textColor );
  messageRect.x = 10;
  messageRect.y = 10;
  messageRect.w = 800;
  messageRect.h = 25;
  message = SDL_CreateTextureFromSurface(renderer, text);
  draw_scene(renderer, message);
  while (1) 
    while(SDL_PollEvent(&event)) 
      if (event.type==SDL_QUIT) goto quit; 
  quit: 
  SDL_FreeSurface(text);
  TTF_CloseFont(font);
  SDL_DestroyTexture(message);
  SDL_Quit(); 
  return 0; 
}