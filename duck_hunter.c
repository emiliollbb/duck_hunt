//--------------------------------- EMILIO'S GAME TEMPLATE --------------------------------

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#define FULL_SCREEN 0
#define BUTTON_A 1
#define BUTTON_B 2
#define BUTTON_X 0
#define BUTTON_Y 3
#define BUTTON_L1 4
#define BUTTON_R1 5
#define BUTTON_L2 6
#define BUTTON_R2 7
#define BUTTON_SELECT 8
#define BUTTON_START 9

struct sized_texture
{
  SDL_Texture* texture;
  int width;
  int height;
};

/* Global variables */
//Screen dimension constants
int SCREEN_WIDTH;
int SCREEN_HEIGHT;
//The window we'll be rendering to
SDL_Window *sdl_window;
//The window renderer
SDL_Renderer* sdl_renderer;
// Display mode
SDL_DisplayMode sdl_display_mode;
//Game Controllers 
SDL_Joystick *sdl_gamepads[2];
// Frames count
unsigned int frames;
// SELECT Button status
int select_button;
// START Button status
int start_button;
// Game over flag
int game_over;
// quit flag
int quit;
// Pause flag
int pause;

// Players number
int players;

// Players menu
int players_menu;

//Globally used font 
TTF_Font *font_small = NULL;
TTF_Font *font_medium = NULL;
TTF_Font *font_big = NULL;


/* Method already implemented */
void init();
void close_sdl();
void load_texture(struct sized_texture *texture, char *path);
TTF_Font* load_font(char *font_path, int size);
void loadTFTTexture(struct sized_texture *texture, TTF_Font *font, char* text, SDL_Color color);
void sync_render();
void process_input(SDL_Event *e);
void render_menu();


/******* Methods to implement *******/
void load_media();
void close_media();
void init_game();
void update_game();
void render();
void process_axis(int controller, int axis, int value);
void process_button_down(int controller, int button);
void process_button_up(int controller, int button);

/* Methods implementation */
void init()
{
  int i;
  
  SCREEN_WIDTH = 720;
  SCREEN_HEIGHT = 480;
  frames = 0;
  game_over=0;
  pause=0;
  quit=0;
  players=1;
  players_menu=1;
  select_button=0;
  start_button=0;
  sdl_window=NULL;
  sdl_renderer = NULL;
  sdl_gamepads[0] = NULL;
  sdl_gamepads[1] = NULL;
  
  //Initialize SDL
  if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_JOYSTICK ) < 0 )
  {
    printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
    exit(-1);
  }
  
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
    printf("%d joysticks connected\n", SDL_NumJoysticks());
    for(i=0; i<SDL_NumJoysticks(); i++)
    {
      //Load joystick 
      sdl_gamepads[i] = SDL_JoystickOpen(i); 
      if(sdl_gamepads[i] == NULL ) 
      { 
	printf( "Warning: Unable to open game controller %d! SDL Error: %s\n", i, SDL_GetError() ); 
	
      }
    }
    
  }
  
  if(FULL_SCREEN)
  {
    // Get display mode
    if (SDL_GetDesktopDisplayMode(0, &sdl_display_mode) != 0) {
      printf("SDL_GetDesktopDisplayMode failed: %s", SDL_GetError());
      exit(-1);
    }
    SCREEN_WIDTH=sdl_display_mode.w;
    SCREEN_HEIGHT=sdl_display_mode.h;
    
    //Create window
    sdl_window = SDL_CreateWindow("Duck_hunter", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_FULLSCREEN);
    if( sdl_window == NULL )
    {
      printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
      exit(-1);
    }
  }
  else
  {
    sdl_window = SDL_CreateWindow("Duck_hunter", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
  }
  
  //Create renderer for window
  sdl_renderer = SDL_CreateRenderer( sdl_window, -1, SDL_RENDERER_ACCELERATED );
  if( sdl_renderer == NULL )
  {
    printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
    exit(-1);
  }
  
  //Initialize SDL_ttf 
  if(TTF_Init()<0) 
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
  
  //Initialize SDL_mixer 
  if(Mix_OpenAudio( 22050, MIX_DEFAULT_FORMAT, 2, 512 )<0) 
  { 
    printf( "SDL_mixer could not initialize!\n");
    exit(-1);
  }
  
  //Initialize renderer color
  SDL_SetRenderDrawColor( sdl_renderer, 0xFF, 0xFF, 0xFF, 0xFF );
  
  // Load small font
  font_small = load_font("ArcadeClassic.ttf", 50);
  
  // Load medium font
  font_medium = load_font("ArcadeClassic.ttf", 80);
  
  // Load big font
  font_big = load_font("ArcadeClassic.ttf", 100);  
  
}

void close_sdl()
{
  int i;
  
  // Close media
  close_media();

  // Close small font
  TTF_CloseFont(font_small);
  // Close medium font
  TTF_CloseFont(font_medium);
  // Close big font
  TTF_CloseFont(font_big);
  
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
  
  // Close gamepads
  for(i=0; i<SDL_NumJoysticks(); i++)
  {
    SDL_JoystickClose(sdl_gamepads[i]);
    sdl_gamepads[i]=NULL;
  }
  
  // Exit SDL
  Mix_CloseAudio();
  TTF_Quit();
  IMG_Quit();
  SDL_Quit();
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

TTF_Font* load_font(char *font_path, int size)
{
  TTF_Font *font;
  
  //Open the font 
  font = TTF_OpenFont(font_path, size); 
  if( font == NULL ) 
  { 
    printf( "Failed to load lazy font! SDL_ttf Error: %s\n", TTF_GetError());
    exit(-1);
  }
  
  return font;
}

void loadTFTTexture(struct sized_texture *texture, TTF_Font *font, char* text, SDL_Color color)
{
  //The final texture
  texture->texture = NULL;
    
  //Load image at specified path
  SDL_Surface *loadedSurface = TTF_RenderText_Solid( font, text, color );
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

void sync_render()
{
  unsigned int ticks; 
  long remaining;
  
  ticks = SDL_GetTicks();
  if(!game_over && !pause && !players_menu)
  {
    // Count frames
    frames++;
    // Update game data
    update_game();
  }
  // Render screen
  if(players_menu)
  {
    render_menu();
  }
  else
  {
    render();  
  }
  
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
    printf("%ld remaining!!!\n", remaining);
  }
  
}

void process_input(SDL_Event *e)
{
  //User requests quit
  if(e->type == SDL_QUIT 
    // User press ESC or q
    || (e->type == SDL_KEYDOWN && (e->key.keysym.sym=='q' || e->key.keysym.sym == 27))
  )
  {
    quit = 1;
  }
  // Axis 0 controls player velocity
  else if(e->type == SDL_JOYAXISMOTION)
  {
    //printf("controller: %d, axis: %d, value: %d\n", e->jaxis.which, e->jaxis.axis, e->jaxis.value);
    process_axis(e->jaxis.which, e->jaxis.axis, e->jaxis.value);
  }
  // Buttons down
  else if(e->type == SDL_JOYBUTTONDOWN) 
  {
    if(e->jbutton.button == BUTTON_SELECT)
    {
      select_button=1;
    }
    if(e->jbutton.button == BUTTON_START)
    {
      start_button=1;
    }
    if(e->jbutton.button != BUTTON_SELECT && e->jbutton.button != BUTTON_START)
    {
      process_button_down(e->jbutton.which, e->jbutton.button);
    }
  }
  // Buttons up
  else if(e->type == SDL_JOYBUTTONUP) 
  {
    if(e->jbutton.button == BUTTON_SELECT)
    {
      select_button=0;
    }
    if(e->jbutton.button == BUTTON_START)
    {
      start_button=0;
    }
    if(e->jbutton.button != BUTTON_SELECT && e->jbutton.button != BUTTON_START)
    {
      process_button_up(e->jbutton.which, e->jbutton.button);
    }
  }
  if(start_button && select_button)
  {
    quit=1;
  }
  if(players_menu && select_button)
  {
    players++;
    if(players>2)
    {
      players=1;
    }
  }
  if(players_menu && start_button)
  {
    // Init game data
    init_game();
    // Close menu
    players_menu=0;
  }
}

void render_menu()
{
  SDL_Rect sdl_rect;
  struct sized_texture texture_text;
  SDL_Color sdl_color;
  
  //Clear screen
  SDL_SetRenderDrawColor( sdl_renderer, 0x00, 0x00, 0x00, 0xFF );
  SDL_RenderClear( sdl_renderer );
  
  if(players==1)
  {
    sdl_color.r=255;
    sdl_color.g=255;
    sdl_color.b=255;
    sdl_color.a=255;
  }
  else
  {
    sdl_color.r=70;
    sdl_color.g=70;
    sdl_color.b=70;
    sdl_color.a=255;
  }
  loadTFTTexture(&texture_text, font_medium, "1 Player", sdl_color);
  sdl_rect.x=SCREEN_WIDTH/2-texture_text.width/2;
  sdl_rect.y=125;
  sdl_rect.w=texture_text.width;
  sdl_rect.h=texture_text.height;  
  SDL_RenderCopy(sdl_renderer, texture_text.texture, NULL, &sdl_rect);
  SDL_DestroyTexture(texture_text.texture);
  
  if(players==2)
  {
    sdl_color.r=255;
    sdl_color.g=255;
    sdl_color.b=255;
    sdl_color.a=255;
  }
  else
  {
    sdl_color.r=70;
    sdl_color.g=70;
    sdl_color.b=70;
    sdl_color.a=255;
  }
  sdl_rect.y+=100;
  loadTFTTexture(&texture_text, font_medium, "2 Players", sdl_color);
  sdl_rect.w=texture_text.width;
  sdl_rect.h=texture_text.height;  
  SDL_RenderCopy(sdl_renderer, texture_text.texture, NULL, &sdl_rect);
  SDL_DestroyTexture(texture_text.texture);
  
  //Update screen
  SDL_RenderPresent(sdl_renderer);
}


int main( int argc, char* args[] )
{
  //Event handler
  SDL_Event e;
  
  // Init quit flag
  quit=0;
  
  // Initialize random seed
  srand(time(NULL));
  
  
  // Start up SDL and create window
  init();
  
  // Load Media
  load_media();
  
  // Main game loop
  while(!quit)
  {
    //Handle events on queue
    while( SDL_PollEvent( &e ) != 0 )
    {
      process_input(&e);
    }
    // Render
    sync_render();
  }
  
  
  close_sdl();
  return 0;
}

//-----------------------------------------------------------------------------------------


#define MAGAZINE_SIZE 4
#define BULLETS_SIZE 100
#define DUCK_WIDTH 40
#define DUCK_HEIGHT 30
#define ANGLE_BULLET 35.0*M_PI/180.0
#define SPEED_BULLET 40.0
#define DUCK_SPEED 5
#define DUCK_START_X 0


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
  int x;
  int y;
  int vx;
  int vy;
};

struct shot_gun
{
  int magazine;
  unsigned int cocking_time;
};

void init_ball();
void fire();
void cock();
void process_start_button();
void process_select_button();





// Fire sound
Mix_Chunk *fire_chunk = NULL;
Mix_Chunk *fire_dry_chunk = NULL;
Mix_Chunk *cocking_chunk = NULL;
Mix_Chunk *quack_chunk = NULL;



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
int p2_score;
struct shot_gun shotgun;
struct bullet bullets[BULLETS_SIZE];
struct duck ducks[20];
int ducks_size;
int hunter_height;
int hunter_width;
int duck_height;
int duck_width;
double speed_bullet;


void load_media()
{ 
  //Load background 
  load_texture(&texture_background, "field.png"); 
  
  //Load hunter 
  load_texture(&texture_hunter, "hunter.png"); 
  
  //Load bullet 
  load_texture(&texture_bulllet, "bullet.png");
  
  // Load sprites
  load_texture(&texture_sprites, "duckhunt_sprites.png");
  
  // Load firing chunk
  fire_chunk = Mix_LoadWAV("firing.wav");
  
  // Load dry firing chunk
  fire_dry_chunk = Mix_LoadWAV("firing_dry.wav");
  
  // Load cooking chunk
  cocking_chunk = Mix_LoadWAV("cocking.wav");
  
  // Load quack
  quack_chunk = Mix_LoadWAV("quack.wav");
  
}

void close_media()
{
  // Free sound effects
  Mix_FreeChunk(fire_chunk);
  Mix_FreeChunk(fire_dry_chunk);
  Mix_FreeChunk(cocking_chunk);
  Mix_FreeChunk(quack_chunk);
  
  // Destroy textures
  SDL_DestroyTexture(texture_background.texture);
  SDL_DestroyTexture(texture_hunter.texture);
  SDL_DestroyTexture(texture_bulllet.texture);
  SDL_DestroyTexture(texture_sprites.texture);
}

void init_game()
{
  int i,j;
  
  if(SCREEN_WIDTH>720)
  {
    hunter_height=2*texture_hunter.height;
    hunter_width=2*texture_hunter.width;
    duck_height=2*DUCK_HEIGHT;
    duck_width=2*DUCK_WIDTH;
    speed_bullet=2*SPEED_BULLET;
  }
  else
  {
    hunter_height=texture_hunter.height;
    hunter_width=texture_hunter.width;
    duck_height=DUCK_HEIGHT;
    duck_width=DUCK_WIDTH;
    speed_bullet=SPEED_BULLET;
  }
  
  p1_x=10;
  p2_x=SCREEN_WIDTH-110;
  p1_y=SCREEN_HEIGHT-hunter_height-40;
  p2_y=p1_y;
  p1_vx=0;
  p2_vx=0;
  p1_score=0;
  p2_score=0;
  player_speed=10;
  shotgun.magazine=MAGAZINE_SIZE;
  shotgun.cocking_time=0;
  
  // Init bullets
  for(i=0; i<BULLETS_SIZE; i++)
  {
    bullets[i].enabled=0;
  }
  
  // init ducks
  ducks_size=10;
  for(i=0; i<10; i++)
  {
    ducks[i].x=DUCK_START_X-300*i-200*(i%2);
    ducks[i].y=50+50*(i%2);
    ducks[i].vx=DUCK_SPEED;
    ducks[i].vy=0;
    ducks[i].shoot_time=0;
    ducks[i].enabled=1;
  }
  if(players==2)
  {
    ducks_size=20;
    for(i=10,j=0; i<20; i++,j++)
    {
      ducks[i].x=SCREEN_WIDTH+300*j+200*(j%2);
      ducks[i].y=20+50*(j%2);
      ducks[i].vx=-DUCK_SPEED;
      ducks[i].vy=0;
      ducks[i].shoot_time=0;
      ducks[i].enabled=1;
    }
  }
  
}

void update_game()
{
  int i,j, all_ducks_disabled;
  
  if(game_over || pause) return;
  
  // Update game
  p1_x+=p1_vx;
  
  // update ducks
  for(i=0; i<ducks_size; i++)
  {
    // Update ducks speed
    
    // Set speed to 0 to outscreen ducks
    if(ducks[i].y>SCREEN_HEIGHT)
    {
      ducks[i].enabled=0;
      ducks[i].vx=0;
      ducks[i].vy=0;
    }
    // Disable outscreen ducks
    if(ducks[i].vx>0 && ducks[i].x>SCREEN_WIDTH)
    {
      ducks[i].enabled=0;
    }
    // Disable outscreen ducks
    if(ducks[i].vx<0 && ducks[i].x<0)
    {
      ducks[i].enabled=0;
    }
    // Shot time
    if(frames == ducks[i].shoot_time)
    {
      ducks[i].vx=0;
      ducks[i].vy=0;
      p1_score++;
    }
    // 10 frames after shot, the ducks falls
    if(ducks[i].shoot_time != 0 && frames == ducks[i].shoot_time+10)
    {
      ducks[i].vx=0;
      ducks[i].vy=10;
    }
    
    // Update ducks position
    ducks[i].x+=ducks[i].vx;
    ducks[i].y+=ducks[i].vy;
    
    // Update shotgun status
    if(frames == shotgun.cocking_time)
    {
      shotgun.magazine=MAGAZINE_SIZE;
    }
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
    for(j=0; j<ducks_size; j++)
    {
      if(bullets[i].enabled && ducks[j].enabled &&
	bullets[i].x>ducks[j].x && bullets[i].x<ducks[j].x+duck_width
	&& bullets[i].y>ducks[j].y && bullets[i].y<ducks[j].y+duck_height)
      {
	ducks[j].shoot_time=frames+1;
	bullets[i].enabled=0;
      }
    }
  }
  
  // Check if end of game
  all_ducks_disabled=1;
  for(i=0; i<ducks_size; i++)
  {
    if(ducks[i].enabled)
    {
      all_ducks_disabled=0;
      break;
    }
  }
  if(all_ducks_disabled)
  {
    game_over=1;
  }
}

void render()
{
  SDL_Rect sdl_rect;
  SDL_Rect sdl_rect2;
  SDL_Color sdl_color;
  int i;
  struct sized_texture texture_text_p1;
  //  struct sized_texture texture_text_p2;
  char p1_score_s[5];
  struct sized_texture texture_game_over;
  
  //Clear screen
  SDL_SetRenderDrawColor( sdl_renderer, 0x00, 0x00, 0x00, 0xFF );
  SDL_RenderClear( sdl_renderer );
  
  // Render background
  SDL_RenderCopy(sdl_renderer, texture_background.texture, NULL, NULL);
  
  // Render hunter
  sdl_rect.x=p1_x;
  sdl_rect.y=p1_y;
  sdl_rect.w=hunter_width;
  sdl_rect.h=hunter_height;
  SDL_RenderCopyEx(sdl_renderer, texture_hunter.texture, NULL, &sdl_rect, 0.0, NULL, SDL_FLIP_NONE);
  
  // Render hunter p2
  if(players==2)
  {
    sdl_rect.x=p2_x;
    sdl_rect.y=p2_y;
    sdl_rect.w=hunter_width;
    sdl_rect.h=hunter_height;
    SDL_RenderCopyEx(sdl_renderer, texture_hunter.texture, NULL, &sdl_rect, 0.0, NULL, SDL_FLIP_HORIZONTAL);
  }
  
  // Render ducks
  for(i=0; i<ducks_size; i++)
  {
    if(ducks[i].enabled)
    {
      if(ducks[i].vx>0 && ducks[i].vy==0)
      {
	sdl_rect.x=130+(frames/10%3*40);
	sdl_rect.y=120;
      }
      else if(ducks[i].vx==0 && ducks[i].vy==0)
      {
	sdl_rect.x=131;
	sdl_rect.y=238;
      }
      else if(ducks[i].vx==0 && ducks[i].vy>0)
      {
	sdl_rect.x=178;
	sdl_rect.y=237;
      }
      sdl_rect.w=DUCK_WIDTH;
      sdl_rect.h=DUCK_HEIGHT;
      sdl_rect2.x=ducks[i].x;
      sdl_rect2.y=ducks[i].y;
      sdl_rect2.w=duck_width;
      sdl_rect2.h=duck_height;      
      SDL_RenderCopyEx(sdl_renderer, texture_sprites.texture, &sdl_rect,  &sdl_rect2, 0.0, NULL, ducks[i].vx>0 ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL);
    }
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
  
  // Render bullets remaining
  for(i=0; i<shotgun.magazine; i++)
  {
    sdl_rect.x=10*i;
    sdl_rect.y=SCREEN_HEIGHT - texture_bulllet.height-10;
    sdl_rect.w=texture_bulllet.width;
    sdl_rect.h=texture_bulllet.height;  
    SDL_RenderCopy(sdl_renderer, texture_bulllet.texture, NULL, &sdl_rect);
  }
  
  // Render ducks counter
  sdl_color.r=0;
  sdl_color.g=0;
  sdl_color.b=0;
  sdl_color.a=255;
  sdl_rect.x=130;
  sdl_rect.y=120;
  sdl_rect.w=DUCK_WIDTH;
  sdl_rect.h=DUCK_HEIGHT;
  sdl_rect2.x=60;
  sdl_rect2.y=SCREEN_HEIGHT - DUCK_HEIGHT - 10;
  sdl_rect2.w=DUCK_WIDTH;
  sdl_rect2.h=DUCK_HEIGHT;
  SDL_RenderCopy(sdl_renderer, texture_sprites.texture, &sdl_rect, &sdl_rect2);
  sprintf(p1_score_s, "%02d", p1_score);
  loadTFTTexture(&texture_text_p1, font_small, p1_score_s, sdl_color);
  sdl_rect.x=100;
  sdl_rect.y=SCREEN_HEIGHT - 49;
  sdl_rect.w=texture_text_p1.width;
  sdl_rect.h=texture_text_p1.height;  
  SDL_RenderCopy(sdl_renderer, texture_text_p1.texture, NULL, &sdl_rect);
  SDL_DestroyTexture(texture_text_p1.texture);
  
  
  // Render game game  over
  if(game_over)
  {
    loadTFTTexture(&texture_game_over, font_big, "GAME OVER", sdl_color);
    sdl_rect.x=SCREEN_WIDTH/2-texture_game_over.width/2;
    sdl_rect.y=SCREEN_HEIGHT/2-texture_game_over.height/2;
    sdl_rect.w=texture_game_over.width;
    sdl_rect.h=texture_game_over.height;  
    SDL_RenderCopy(sdl_renderer, texture_game_over.texture, NULL, &sdl_rect);
    SDL_DestroyTexture(texture_game_over.texture);
  }
  
  // Render pause
  if(pause)
  {
    loadTFTTexture(&texture_game_over, font_big, "PAUSE", sdl_color);
    sdl_rect.x=SCREEN_WIDTH/2-texture_game_over.width/2;
    sdl_rect.y=SCREEN_HEIGHT/2-texture_game_over.height/2;
    sdl_rect.w=texture_game_over.width;
    sdl_rect.h=texture_game_over.height;  
    SDL_RenderCopy(sdl_renderer, texture_game_over.texture, NULL, &sdl_rect);
    SDL_DestroyTexture(texture_game_over.texture);
  }
  
  // Play quacks
  if(!game_over && frames%90==0)
  {
    Mix_PlayChannel(-1, quack_chunk, 0);
  }
  
  //Update screen
  SDL_RenderPresent(sdl_renderer);
}


void process_axis(int controller, int axis, int value)
{
}

void process_button_down(int controller, int button)
{
  switch(button) 
  {
    // Fire
    case BUTTON_A: case BUTTON_R1: case BUTTON_R2: fire(); break;
    // Reload
    case BUTTON_B: case BUTTON_L1: cock(); break;
    case BUTTON_SELECT: process_select_button(); break;
    case BUTTON_START: process_start_button(); break;    
  }
}
void process_button_up(int controller, int button)
{
}


void fire()
{
  struct bullet current;
  int i;
  
  if(game_over) return;
  
  if(shotgun.magazine>0)
  {
    current.x=p1_x+hunter_width;
    current.y=p1_y;
    current.vx=speed_bullet*cos(ANGLE_BULLET);
    current.vy=-1.0*speed_bullet*sin(ANGLE_BULLET);
    
    // Insert bullet in array
    for(i=0; i<BULLETS_SIZE; i++)
    {
      if(!bullets[i].enabled)
      {
	bullets[i]=current;
	bullets[i].enabled=1;
	shotgun.magazine--;
	Mix_PlayChannel(-1, fire_chunk, 0);
	break;
      }
    }
  }
  else
  {
    Mix_PlayChannel(-1, fire_dry_chunk, 0);
  }
}

void cock()
{
  if(game_over) return;
  shotgun.magazine=0;
  Mix_PlayChannel(-1, cocking_chunk, 0);
  shotgun.cocking_time=frames+30;
}

void process_start_button()
{
  if(game_over)
  {
    game_over=0;
    pause=0;
    init_game();
  }
  else
  {
    pause=!pause;
  }
}

void process_select_button()
{
}












