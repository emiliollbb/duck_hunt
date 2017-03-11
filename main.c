#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

struct sized_texture
{
  SDL_Texture* texture;
  int width;
  int height;
};

void init();
void close_sdl();
void sync_render();
void render();
void loadTFTTexture(struct sized_texture *texture, TTF_Font *font, char* text);
void load_texture(struct sized_texture *texture, char *path);
void process_input(SDL_Event *e, int *quit);
void init_ball();

//Screen dimension constants
int SCREEN_WIDTH;
int SCREEN_HEIGHT;

//The window we'll be rendering to
SDL_Window *sdl_window;

//The window renderer
SDL_Renderer* sdl_renderer;

//Game Controller 1 handler 
SDL_Joystick *sdl_gamepad;

//Globally used font 
TTF_Font *number_font = NULL;

struct sized_texture texture_text_p1;
struct sized_texture texture_text_p2;
struct sized_texture texture_background;
struct sized_texture texture_hunter;

/** GAME DATA **/
int p1_y;
int p2_y;
int p1_x;
int p2_x;
int p1_vx;
int p2_vx;
int player_speed;
int p1_score;
char p1_score_s[10];
int p2_score;
char p2_score_s[10];
int p1_flip;

void init()
{
  SCREEN_WIDTH = 720;
  SCREEN_HEIGHT = 480;
  sdl_window=NULL;
  sdl_renderer = NULL;
  sdl_gamepad = NULL;
  
  //Initialize SDL
  if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_JOYSTICK ) < 0 )
  {
    printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
    exit(-1);
  }
  else
  {
    //Set texture filtering to linear
    if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
    {
      printf( "Warning: Linear texture filtering not enabled!" );
      exit(-1);
    }
    
    //Check for joysticks 
    if( SDL_NumJoysticks() < 1 ) 
    { 
      printf( "Warning: No joysticks connected!\n" ); 
      
    } 
    else 
    { 
      //Load joystick 
      sdl_gamepad = SDL_JoystickOpen( 0 ); 
      if( sdl_gamepad == NULL ) 
      { 
        printf( "Warning: Unable to open game controller! SDL Error: %s\n", SDL_GetError() ); 
        
      } 
      
    }

    //Create window
    sdl_window = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
    //sdl_window = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_FULLSCREEN );
    if( sdl_window == NULL )
    {
      printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
      exit(-1);
    }
    
    //Create renderer for window
    sdl_renderer = SDL_CreateRenderer( sdl_window, -1, SDL_RENDERER_ACCELERATED );
    if( sdl_renderer == NULL )
    {
      printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
      exit(-1);
    }
    
    //Initialize SDL_ttf 
    if( TTF_Init() == -1 ) 
    {
      printf( "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError() ); 
      exit(-1);
    }
    
    //Initialize PNG loading
    if( !( IMG_Init( IMG_INIT_PNG ) & IMG_INIT_PNG ) )
    {
      printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
      exit(-1);
    }
    
    //Initialize renderer color
    SDL_SetRenderDrawColor( sdl_renderer, 0xFF, 0xFF, 0xFF, 0xFF );
    
    
  }
}

void close_sdl()
{
  //Destroy renderer  
  if(sdl_renderer!=NULL)
  {
    SDL_DestroyRenderer( sdl_renderer );
    sdl_renderer=NULL;
  }
  
  if(sdl_window != NULL)
  {
    // Destroy window
    SDL_DestroyWindow( sdl_window );
    sdl_window=NULL;
  }
  
  // Exit SDL
  TTF_Quit();
  IMG_Quit();
  SDL_Quit();
}

void load_media()
{
  //Open the font 
  number_font = TTF_OpenFont( "DSEG7Classic-Bold.ttf", 50 ); 
  if( number_font == NULL ) 
  { 
    printf( "Failed to load lazy font! SDL_ttf Error: %s\n", TTF_GetError() );
    exit(-1);
  }
  
  //Load background 
  load_texture(&texture_background, "field.png"); 
  
  //Load hunter 
  load_texture(&texture_hunter, "hunter.png"); 
    
}

void loadTFTTexture(struct sized_texture *texture, TTF_Font *font, char* text)
{
  //The final texture
  texture->texture = NULL;
  // Text color
  SDL_Color textColor = { 255, 255, 255 };

  //Load image at specified path
  SDL_Surface *loadedSurface = TTF_RenderText_Solid( font, text, textColor );
  if( loadedSurface == NULL )
  {
    printf( "Unable to render text! SDL_image Error: %s\n", TTF_GetError() );
    exit(-1);
  }
  else
  {
    //printf("Surface: %d, %d\n", loadedSurface->w, loadedSurface->h);
    
    //Create texture from surface pixels
        texture->texture = SDL_CreateTextureFromSurface(sdl_renderer, loadedSurface );
    if( texture->texture == NULL )
    {
      printf( "Unable to create texture! SDL Error: %s\n", SDL_GetError() );
      exit(-1);
    }
    else {
      //Get image dimensions 
      texture->width = loadedSurface->w; 
      texture->height = loadedSurface->h;      
    }

    //Get rid of old loaded surface
    SDL_FreeSurface( loadedSurface );
  }

}

void load_texture(struct sized_texture *texture, char *path)
{
  // Aux surface
  SDL_Surface* loadedSurface;

  //Load image at specified path
  loadedSurface = IMG_Load(path);
  if(loadedSurface == NULL )
  {
    printf( "Unable to load image %s! SDL_image Error: %s\n", path, IMG_GetError() );
    exit(-1);
  }
  //Get image dimensions 
  texture->width = loadedSurface->w; 
  texture->height = loadedSurface->h;
  //Create texture from surface pixels
  texture->texture = SDL_CreateTextureFromSurface(sdl_renderer, loadedSurface);
  if( texture->texture == NULL )
  {
    printf( "Unable to create texture from %s! SDL Error: %s\n", path, SDL_GetError() );
  }

  //Get rid of old loaded surface
  SDL_FreeSurface(loadedSurface);

}

void sync_render()
{
  unsigned int ticks; 
  long remaining;
  
  ticks = SDL_GetTicks();
  
  render();  
  
  remaining = ticks;
  remaining = remaining + 16 - SDL_GetTicks();
  
  if(remaining > 0)
  {
    //remaining = 1;
    SDL_Delay(remaining);
  }
  else
  {
    printf("%d remaining!!!\n", remaining);
  }

}  

void init_game()
{
  p1_x=10;
  p2_x=SCREEN_WIDTH-10;
  p1_y=SCREEN_HEIGHT-texture_hunter.height-25;
  p2_y=0;
  p1_vx=0;
  p2_vx=0;
  p1_score=0;
  p2_score=0;
  player_speed=10;
  p1_flip=0;
}


void render()
{  
  // Update game
  p1_x=p1_x+=p1_vx;
  
  //Clear screen
  SDL_SetRenderDrawColor( sdl_renderer, 0x00, 0x00, 0x00, 0xFF );
  SDL_RenderClear( sdl_renderer );
    
  
  // Render background
  SDL_Rect fillRect = {0, 0, texture_background.width, texture_background.height};
  SDL_RenderCopy(sdl_renderer, texture_background.texture, NULL, &fillRect);
  
  // Render hunter
  SDL_Rect fillRect2 = {p1_x, p1_y, texture_hunter.width, texture_hunter.height};
  SDL_RenderCopyEx(sdl_renderer, texture_hunter.texture, NULL, &fillRect2, 0.0, NULL, p1_flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
  
  
  SDL_SetRenderDrawColor(sdl_renderer, 0xFF, 0x00, 0x00, 0xFF );
  
  SDL_Rect fillRect3 = {20, 20, 3, 3};
  SDL_RenderFillRect( sdl_renderer, &fillRect3 );
  
  
  //Update screen
  SDL_RenderPresent(sdl_renderer);
}

void process_input(SDL_Event *e, int *quit)
{
      //User requests quit
      if( e->type == SDL_QUIT )
      {
        *quit = 1;
      }
      //User presses a key 
      else if( e->type == SDL_KEYDOWN ) 
      {
        //Select surfaces based on key press 
        switch( e->key.keysym.sym ) 
        {
          case 'q': *quit = 1; break;
          case 27:  *quit = 1; break;
          case SDLK_UP: ; break; 
          case SDLK_DOWN: ; break; 
          case SDLK_LEFT: p1_vx=-1*player_speed ; break; 
          case SDLK_RIGHT: p1_vx=player_speed ; break;
	  case 'z': p1_flip=(p1_flip+1)%2;
	  case 'w': ; break;
	  case 's': ; break;
          default: printf("key: %d\n", e->key.keysym.sym); break; 
             
        }
      }
      else if( e->type == SDL_KEYUP ) 
      {
        //Select surfaces based on key press 
        switch( e->key.keysym.sym ) 
        {
          case SDLK_UP: ; break; 
          case SDLK_DOWN: ; break; 
          case SDLK_LEFT: p1_vx=0 ; break; 
          case SDLK_RIGHT: p1_vx=0 ; break;
	  case 'w': ; break;
	  case 's': ; break;
         }
      }
      /* Handle Joystick Button Presses */
      else if( e->type == SDL_JOYBUTTONDOWN)
      {
        switch( e->jbutton.button) 
        {
          case 0: ;
          break;
        }
      }
      else if( e->type == SDL_JOYAXISMOTION)
      {
	//Motion on controller 0 
	if( e->jaxis.which == 0 ) 
	{ 
	  //X axis motion 
	  if( e->jaxis.axis == 0 ) 
	  { 
	    printf("axis: %d\n", e->jaxis.value);
	  }
	}
      }
}

int main( int argc, char* args[] )
{
  //Main loop flag
  int quit=0;
  
  //Event handler
  SDL_Event e;
  
  // Initialize random seed
  srand(time(NULL));
  
  
  // Start up SDL and create window
  init();
  
  // Load Media
  load_media();
  
  // Init game data
  init_game();
  
  // Main game loop
  while(!quit)
  {
    //Handle events on queue
    while( SDL_PollEvent( &e ) != 0 )
    {
      process_input(&e, &quit);
    }
    // Render
    sync_render();
  }


  close_sdl();
  return 0;
}
  
  
  
  