#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#define MAGAZINE_SIZE 10
#define BULLETS_SIZE 100
#define DUCKS_SIZE 10
#define COLLISION_MARGIN 5
#define DUCK_WIDTH 40
#define DUCK_HEIGHT 30

struct sized_texture
{
  SDL_Texture* texture;
  int width;
  int height;
};

struct bullet
{
  int enabled;
  int x;
  int y;
  int vx;
  int vy;
};

struct duck
{
  int enabled;
  unsigned int shoot_time;
  unsigned int start;
  int x;
  int y;
  int vx;
  int vy;
};

void init();
void close_sdl();
void sync_render();
void update_game();
void render();
void loadTFTTexture(struct sized_texture *texture, TTF_Font *font, char* text);
void load_texture(struct sized_texture *texture, char *path);
void process_input(SDL_Event *e, int *quit);
void init_ball();
void fire();

//Screen dimension constants
int SCREEN_WIDTH;
int SCREEN_HEIGHT;

//The window we'll be rendering to
SDL_Window *sdl_window;

//The window renderer
SDL_Renderer* sdl_renderer;

// Display mode
SDL_DisplayMode sdl_display_mode;

//Game Controller 1 handler 
SDL_Joystick *sdl_gamepad;

//Globally used font 
TTF_Font *number_font = NULL;

unsigned int frames;

struct sized_texture texture_text_p1;
struct sized_texture texture_text_p2;
struct sized_texture texture_background;
struct sized_texture texture_hunter;
struct sized_texture texture_bulllet;
struct sized_texture texture_sprites;

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
float speed_bullet;
float angle_bullet;
struct bullet bullets[BULLETS_SIZE];
int magazine;
struct duck ducks[DUCKS_SIZE];

void init()
{
  SCREEN_WIDTH = 720;
  SCREEN_HEIGHT = 480;
  frames = 0;
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

    // Get display mode
    if (SDL_GetDesktopDisplayMode(0, &sdl_display_mode) != 0) {
      printf("SDL_GetDesktopDisplayMode failed: %s", SDL_GetError());
      exit(-1);
    }
    SCREEN_WIDTH=sdl_display_mode.w;
    SCREEN_HEIGHT=sdl_display_mode.h;
    
    //Create window
    sdl_window = SDL_CreateWindow("Duck_hunter", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_FULLSCREEN );
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
  // Destroy textures
  SDL_DestroyTexture(&texture_background);
  SDL_DestroyTexture(&texture_hunter);
  SDL_DestroyTexture(&texture_bulllet);
  SDL_DestroyTexture(&texture_sprites);
    
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
  
  //Load bullet 
  load_texture(&texture_bulllet, "bullet.png");
  
  // Load sprites
  load_texture(&texture_sprites, "duckhunt_sprites.png");
    
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
  // Count frames
  frames++;
  // Update game data
  update_game();
  // Render screen
  render();  
  
  remaining = ticks;
  //remaining = remaining + 16 - SDL_GetTicks();
  // 30 fps
  remaining = remaining + 32 - SDL_GetTicks();
  
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
  int i;
  p1_x=10;
  p2_x=SCREEN_WIDTH-10;
  p1_y=SCREEN_HEIGHT-texture_hunter.height-40;
  p2_y=0;
  p1_vx=0;
  p2_vx=0;
  p1_score=0;
  p2_score=0;
  player_speed=10;
  p1_flip=0;
  speed_bullet=50.0;
  angle_bullet=35.0*M_PI/180.0;
  magazine=MAGAZINE_SIZE;
  
  // Init bullets
  for(i=0; i<BULLETS_SIZE; i++)
  {
      bullets[i].enabled=0;
  }
  
  // init ducks
  for(i=0; i<DUCKS_SIZE; i++)
  {
    ducks[i].x=-20;
    ducks[i].y=150;
    ducks[i].vx=0;
    ducks[i].vy=0;
    ducks[i].shoot_time=0;
    ducks[i].start=100*i+50;
    ducks[i].enabled=0;
  }
  
}

void fire()
{
  struct bullet current;
  int i;
  
  if(magazine>0)
  {
    if(!p1_flip)
    {
      current.x=p1_x+texture_hunter.width;
      current.y=p1_y;
      current.vx=speed_bullet*cos(angle_bullet);
      current.vy=-1.0*speed_bullet*sin(angle_bullet);
    }
    else
    {
      current.x=p1_x;
      current.y=p1_y;
      current.vx=-1.0*speed_bullet*cos(angle_bullet);
      current.vy=-1.0*speed_bullet*sin(angle_bullet);
    }
    
    // Insert bullet in array
    for(i=0; i<BULLETS_SIZE; i++)
    {
        if(!bullets[i].enabled)
        {
            bullets[i]=current;
            bullets[i].enabled=1;
            magazine--;
            break;
        }
    }
  }
}

void update_game()
{
  int i,j;
  
  // Update game
  p1_x=p1_x+=p1_vx;
  
  // update ducks
  for(i=0; i<DUCKS_SIZE; i++)
  {
    // Update ducks speed
    
    // If is time for the duck to start
    if(frames==ducks[i].start)
    {
      ducks[i].enabled=1;
      ducks[i].vx=1;
      ducks[i].vy=0;
    }
    // Set speed to 0 to outscreen ducks
    if(ducks[i].x>SCREEN_WIDTH || ducks[i].y>SCREEN_HEIGHT)
    {
      ducks[i].enabled=0;
      ducks[i].vx=0;
      ducks[i].vy=0;
    }
    // Shot time
    if(ducks[i].shoot_time>0 && ducks[i].shoot_time==frames)
    {
      ducks[i].vx=0;
      ducks[i].vy=0;
    }
    // 30 frames after shot, the ducks falls
    if(ducks[i].shoot_time>0 && frames > ducks[i].shoot_time+30)
    {
      ducks[i].vx=0;
      ducks[i].vy=10;
    }
    
    // Update ducks position
    ducks[i].x+=ducks[i].vx;
    ducks[i].y+=ducks[i].vy;
  }
  
  // Update bullets
  for(i=0; i<BULLETS_SIZE; i++)
  {
    if(bullets[i].y>SCREEN_HEIGHT || bullets[i].y<0 || bullets[i].x>SCREEN_WIDTH || bullets[i].x<0)
    {
      bullets[i].enabled=0;
    }
    if(bullets[i].enabled)
    {
      bullets[i].x+=bullets[i].vx;
      bullets[i].y+=bullets[i].vy;
    }
  }
  
  // Check collisions
  for(i=0; i<BULLETS_SIZE; i++)
  {
    for(j=0; j<DUCKS_SIZE; j++)
    {
      if(bullets[i].enabled && ducks[j].enabled &&
        bullets[i].x>ducks[j].x+COLLISION_MARGIN && bullets[i].x<ducks[j].x+DUCK_WIDTH-COLLISION_MARGIN
        && bullets[i].y>ducks[j].y+COLLISION_MARGIN && bullets[i].y<ducks[j].y+DUCK_HEIGHT-COLLISION_MARGIN)
      {
        ducks[j].shoot_time=frames+1;
        bullets[i].enabled=0;
      }
    }
  }
}

void render()
{
  SDL_Rect sdl_rect;
  SDL_Rect sdl_rect2;
  int i;
  
  //Clear screen
  SDL_SetRenderDrawColor( sdl_renderer, 0x00, 0x00, 0x00, 0xFF );
  SDL_RenderClear( sdl_renderer );
  
  // Render background
  SDL_RenderCopy(sdl_renderer, texture_background.texture, NULL, NULL);
  
  // Render hunter
  sdl_rect.x=p1_x;
  sdl_rect.y=p1_y;
  sdl_rect.w=texture_hunter.width;
  sdl_rect.h=texture_hunter.height;
  SDL_RenderCopyEx(sdl_renderer, texture_hunter.texture, NULL, &sdl_rect, 0.0, NULL, p1_flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
  
  // Render ducks
  for(i=0; i<DUCKS_SIZE; i++)
  {
      if(ducks[i].enabled)
      {
        if(ducks[i].vx>0 && ducks[i].vy==0)
        {
            sdl_rect2.x=130+(frames/10%3*40);
            sdl_rect2.y=120;
        }
        else if(ducks[i].vx==0 && ducks[i].vy==0)
        {
            sdl_rect2.x=131;
            sdl_rect2.y=238;
        }
        else if(ducks[i].vx==0 && ducks[i].vy>0)
        {
            sdl_rect2.x=178;
            sdl_rect2.y=237;
        }
        sdl_rect2.w=DUCK_WIDTH;
        sdl_rect2.h=DUCK_HEIGHT;
        sdl_rect.x=ducks[i].x;
        sdl_rect.y=ducks[i].y;
        sdl_rect.w=sdl_rect2.w;
        sdl_rect.h=sdl_rect2.h;      
        SDL_RenderCopy(sdl_renderer, texture_sprites.texture, &sdl_rect2, &sdl_rect);
      }
  }
      
  // Render bullets remaining
  for(i=0; i<magazine; i++)
  {
    SDL_Rect fillRect3 = {10*i, SCREEN_HEIGHT - texture_bulllet.height-10, texture_bulllet.width, texture_bulllet.height};
    SDL_RenderCopy(sdl_renderer, texture_bulllet.texture, NULL, &fillRect3);
  }
  
  // Render fired bullets
  SDL_SetRenderDrawColor(sdl_renderer, 0x00, 0x00, 0x00, 0xFF );  
  for(i=0; i<BULLETS_SIZE; i++)
  {
    if(bullets[i].enabled)
    {
        sdl_rect.x=bullets[i].x;
        sdl_rect.y=bullets[i].y;
        sdl_rect.w=4;
        sdl_rect.h=4;
        SDL_RenderFillRect(sdl_renderer, &sdl_rect);
    }
  }
      
  //Update screen
  SDL_RenderPresent(sdl_renderer);
}

void process_input(SDL_Event *e, int *quit)
{
    //User requests quit
      if(e->type == SDL_QUIT 
          // User press ESC or q
          || e->type == SDL_KEYDOWN && (e->key.keysym.sym=='q' || e->key.keysym.sym == 27)
          // User 1 press select
          || e->type == SDL_JOYBUTTONDOWN && e->jbutton.button == 8)
      {
        *quit = 1;
      }
      // Axis 0 controls player velocity
      else if(e->type == SDL_JOYAXISMOTION && e->jaxis.axis == 0)
      {
        //printf("controller: %d, axis: %d, value: %d\n", e->jaxis.which, e->jaxis.axis, e->jaxis.value);
        p1_vx=player_speed*e->jaxis.value/32767; 
      }
      // Buttons
      else if(e->type == SDL_JOYBUTTONDOWN) 
      {
        switch(e->jbutton.button) 
        {
          // Flip hunter
          case 2: p1_flip=(p1_flip+1)%2; break;
          // Fire
          case 1: case 5: case 7: fire(); break;
          // Reload
          case 0: case 4: magazine=MAGAZINE_SIZE; break;          
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
  
  
  
  
