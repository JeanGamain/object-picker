/* libSDL and libVLC sample code
 * Copyright © 2008 Sam Hocevar <sam@zoy.org>
 * license: [http://en.wikipedia.org/wiki/WTFPL WTFPL] */

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <unistd.h>

#include <SDL/SDL.h>
#include <SDL/SDL_mutex.h>

#include <vlc/vlc.h>

#include "image.hpp"
#include "pixel16.hpp"
#include "ObjectPicker.hpp"

/*
#define VIDEOWIDTH 512
#define VIDEOHEIGHT 377

#define WIDTH VIDEOWIDTH + 10
#define HEIGHT VIDEOHEIGHT + 10
*/

float diffa = 0;
float diffb = 0;
float diffc = 0;
float diffd = 0;
vec2 size;
image<pixel16> * img;

int setPause = 0;

struct ctx
{
  SDL_Surface *surf;
  SDL_mutex *mutex;
};

static void *lock(void *data, void **p_pixels)
{
  struct ctx *ctx = static_cast<struct ctx*>(data);

  SDL_LockMutex(ctx->mutex);
  SDL_LockSurface(ctx->surf);
  *p_pixels = ctx->surf->pixels;
  return NULL; /* picture identifier, not needed here */
}

void showfps()
{
  static double lasttime = 0;
  static unsigned int nbframe = 0;
  static double maxfps = 0;
  struct timeval tv;
  double curtime;
  double fps;

  gettimeofday(&tv, NULL);
  curtime = double(tv.tv_sec) + double(tv.tv_usec) / 1000000;
  if (lasttime == 0) {
    lasttime = curtime;
  }
  if ((fps = curtime - lasttime) > 1)
    {
      fps = nbframe / fps;
      if (maxfps < fps)
	maxfps = fps;
      lasttime = curtime;
      nbframe = 0;
      printf("%.01f fps - %.01f max\n", fps, maxfps);
    }
  else
    nbframe++;
}

static void unlock(void *data, void *, void *const *p_pixels)
{
  struct ctx *ctx = static_cast<struct ctx *>(data);
  static ObjectPicker * objectpick = new ObjectPicker(size);

  if (p_pixels == NULL)
    return;
  img->pixel = static_cast<pixel16 *>(*p_pixels);
  /* VLC just rendered the video, but we can also render stuff */

  objectpick->detect(img);
  
  showfps();
  SDL_UnlockSurface(ctx->surf);
  SDL_UnlockMutex(ctx->mutex);
}

static void display(void *data, void *id)
{
  /* VLC wants to display the video */
  (void) data;
  assert(id == NULL);
}

int main(int argc, char *argv[])
{
      char const *vlc_argv[] =
	{
	  //"--no-audio", /* skip any audio track */
	  "--no-xlib", /* tell VLC to not use Xlib */
	};
      int vlc_argc = sizeof(vlc_argv) / sizeof(*vlc_argv);

      SDL_Surface *screen, *empty;
      SDL_Event event;
      SDL_Rect rect;
      int done = 0, action = 0;
      
      struct ctx ctx;

      if(argc < 2)
	{
	  printf("Usage: %s <filename>\n", argv[0]);
	  return EXIT_FAILURE;
	}


      /*
       *  Initialise libVLC
       */
      unsigned int width, height;
      libvlc_instance_t *libvlc = libvlc_new(vlc_argc, vlc_argv);
      libvlc_media_t *m = libvlc_media_new_path(libvlc, argv[1]);
      libvlc_media_player_t *mp = libvlc_media_player_new_from_media(m);

      libvlc_media_player_play(mp);
      while(!libvlc_media_is_parsed(m))
	usleep(15);
      libvlc_video_get_size(mp, 0, &width, &height);
      img = new image<pixel16>(width, height, NULL);      
      libvlc_media_release(m);
      libvlc_video_set_format(mp, "RV16", width, height, width * 2);
      libvlc_video_set_callbacks(mp, lock, unlock, display, &ctx);
      size = vec2(width, height);
      
      printf("size: %d %d\n", size.x, size.y);


      /*
       *  Initialise libSDL
       */
      if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTTHREAD) == -1) {
	printf("cannot initialize SDL\n");
	return EXIT_FAILURE;
      }
      empty = SDL_CreateRGBSurface(SDL_SWSURFACE, size.x, size.y, 32, 0, 0, 0, 0);
      ctx.surf = SDL_CreateRGBSurface(SDL_SWSURFACE, size.x, size.y, 16, 0x001f, 0x07e0, 0xf800, 0);
      ctx.mutex = SDL_CreateMutex();

      int options = SDL_ANYFORMAT | SDL_HWSURFACE | SDL_DOUBLEBUF;

      screen = SDL_SetVideoMode(size.x + 10, size.y + 10, 0, options);
      if (!screen) {
	  printf("cannot set video mode\n");
	  return EXIT_FAILURE;
      }
      
      /*
       *  Main loop
       */
      rect.w = 0;
      rect.h = 0;
      rect.x = 5;
      rect.y = 5;
      while(!done)
	{
	  action = 0;
	  /* Keys: enter (fullscreen), space (pause), escape (quit) */
	  while (SDL_PollEvent(&event)) {
	    switch(event.type) {
	    case SDL_QUIT:
	      done = 1;
	      break;
	    case SDL_KEYDOWN:
	      action = event.key.keysym.sym;
	      break;
	    }
	  }

	  switch(action) {
	    case SDLK_ESCAPE:
	      done = 1;
	      break;
	    case SDLK_RETURN:
	      options ^= SDL_FULLSCREEN;
	      screen = SDL_SetVideoMode(size.x + 10, size.y + 10, 0, options);
	      break;
	    case SDLK_a:
	      setPause = !setPause;
	      libvlc_media_player_set_pause(mp, setPause);
	      break;
	    case SDLK_o:
	      if (diffa + 1 < 250)
		diffa += 1;
	      printf("a+ at %f\n", diffa);
	      break;
	    case SDLK_p:
	      if (diffa - 1 > -200)
		diffa -= 1;
	      printf("a- at %f\n", diffa);	      
	      break;
	    case SDLK_j:
	      if (diffb + 1 < 250)
		diffb += 1;
	      printf("b+ at %f\n", diffb);
	      break;
	    case SDLK_k:
	      if (diffb - 1 > -200)
		diffb -= 1;
	      printf("b- at %f\n", diffb);	      
	      break;
	    case SDLK_b:
	      if (diffc + 0.01 < 250)
		diffc += 0.01;
	      printf("c+ at %f\n", diffc);
	      break;
	    case SDLK_n:
	      if (diffc - 0.01 > -200)
		diffc -= 0.01;
	      printf("c- at %f\n", diffc);
	      break;
	    case SDLK_q:
	      if (diffd + 0.01 < 40)
		diffd += 0.01;
	      printf("d+ at %f\n", diffd);
	      break;
	    case SDLK_s:
	      if (diffd - 0.01 > -40)
		diffd -= 0.01;
	      printf("d- at %f\n", diffd);
	      break;
	    }

	  /* Blitting the surface does not prevent it from being locked and
	   * written to by another thread, so we use this additional mutex. */
	  SDL_LockMutex(ctx.mutex);
	  SDL_BlitSurface(ctx.surf, NULL, screen, &rect);
	  SDL_UnlockMutex(ctx.mutex);
	  SDL_Flip(screen);
	}

      /*
       * Stop stream and clean up libVLC
       */
      libvlc_media_player_stop(mp);
      libvlc_media_player_release(mp);
      libvlc_release(libvlc);

      /*
       * Close window and clean up libSDL
       */
      SDL_DestroyMutex(ctx.mutex);
      SDL_FreeSurface(ctx.surf);
      SDL_FreeSurface(empty);

      SDL_Quit();

      return 0;
}
