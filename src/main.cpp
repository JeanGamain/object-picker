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
#include "parm.hpp"

varSet	vaParm[24];
int	maxParm = 0;

vec2 size;
image<pixel16> * img;

int setPause = 0;

struct ctx
{
  SDL_Surface *surf = NULL;
  SDL_mutex *mutex = NULL;
};

static void *lock(void *data, void **p_pixels)
{
  struct ctx *ctx = static_cast<struct ctx*>(data);

  SDL_LockMutex(ctx->mutex);
  while (ctx->surf == NULL)
    usleep(10);
  SDL_LockSurface(ctx->surf);
  *p_pixels = ctx->surf->pixels;
  return NULL; /* picture identifier, not needed here */
}

void showfps()
{
  static double lasttime = 0;
  static unsigned int nbframe = 0;
  static double fpsSum = 0;
  static unsigned int fpsCount = 0;
  struct timeval tv;
  double curtime;
  double fps, fpsmoy;

  gettimeofday(&tv, NULL);
  curtime = double(tv.tv_sec) + double(tv.tv_usec) / 1000000;
  if (lasttime == 0) {
    lasttime = curtime;
  }
  if ((fps = curtime - lasttime) > 1)
    {
      fps = nbframe / fps;
      fpsSum += fps;
      fpsCount++;
      lasttime = curtime;
      nbframe = 0;
      fpsmoy = fpsSum / fpsCount;
      printf("%.01f fps - %.01f moy - %.01fms\n", fps, fpsmoy, (24.8 - fpsmoy) * 40.322);
    }
  else
    nbframe++;
}

static void unlock(void *data, void *, void *const *p_pixels)
{
  struct ctx *ctx = static_cast<struct ctx *>(data);
  static ObjectPicker * objectpick = new ObjectPicker(size);

  img->pixel = static_cast<pixel16 *>(*p_pixels);
  /* VLC just rendered the video, but we can also render stuff */

  objectpick->detect(img);
  
  showfps();
  SDL_UnlockSurface(ctx->surf);
  SDL_UnlockMutex(ctx->mutex);
}

static void display(void *data, void *id)
{
  (void)data;
  /* VLC wants to display the video */
  assert(id == NULL);
}


int		vaParmI = 0;
void		ChangevaParm(char add) {
  int		i = vaParmI;

  if (maxParm == 0)
    return;
  if (vaParm[i].type == UINT) {
    if (add) {
      if (*(unsigned int*)(vaParm[i].value) + *(unsigned int*)(vaParm[i].step) <= *(unsigned int*)(vaParm[i].max)) {
	*((unsigned int*)(vaParm[i].value)) += *((unsigned int*)(vaParm[i].step));
	printf("%s: %u +\n", vaParm[i].name, *(unsigned int*)(vaParm[i].value));
      } else
	printf("%s: %u MAX\n", vaParm[i].name, *(unsigned int*)(vaParm[i].value));
    } else {
      if (*(unsigned int*)(vaParm[i].value) - *(unsigned int*)(vaParm[i].step) >= *(unsigned int*)(vaParm[i].min)) {
	*((unsigned int*)(vaParm[i].value)) -= *((unsigned int*)(vaParm[i].step));
	printf("%s: %u -\n", vaParm[i].name, *(unsigned int*)(vaParm[i].value));
      } else
	printf("%s: %u MIN\n", vaParm[i].name, *(unsigned int*)(vaParm[i].value));
    }
  } else if (vaParm[i].type == INT) {
    if (add) {
      if (*(int*)(vaParm[i].value) + *(int*)(vaParm[i].step) <= *(int*)(vaParm[i].max)) {
	*((int*)(vaParm[i].value)) += *((int*)(vaParm[i].step));
	printf("%s: %d +\n", vaParm[i].name, *(int*)(vaParm[i].value));
      } else
	printf("%s: %d MAX\n", vaParm[i].name, *(int*)(vaParm[i].value));
    } else {
      if (*(int*)(vaParm[i].value) - *(int*)(vaParm[i].step) >= *(int*)(vaParm[i].min)) {
	*((int*)(vaParm[i].value)) -= *((int*)(vaParm[i].step));
	printf("%s: %d -\n", vaParm[i].name, *(int*)(vaParm[i].value));
      } else
	printf("%s: %d MIN\n", vaParm[i].name, *(int*)(vaParm[i].value));
    }
  } else if (vaParm[i].type == FLOAT) {
      if (add) {
	if (*((float*)(vaParm[i].value)) + *((float*)(vaParm[i].step)) <= *(float*)(vaParm[i].max)) {
	  *((float*)(vaParm[i].value)) += *((float*)(vaParm[i].step));
	  printf("%s: %f +\n", vaParm[i].name, *((float*)(vaParm[i].value)));
	} else
	  printf("%s: %f MAX\n", vaParm[i].name, *((float*)(vaParm[i].value)));
      } else {
	if (*((float*)(vaParm[i].value)) - *((float*)(vaParm[i].step)) >= *(float*)(vaParm[i].min)) {
	  *((float*)(vaParm[i].value)) -= *((float*)(vaParm[i].step));
	  printf("%s: %f -\n", vaParm[i].name, *((float*)(vaParm[i].value)));
	} else
	  printf("%s: %f MIN\n", vaParm[i].name, *((float*)(vaParm[i].value)));
      }
  } else if (vaParm[i].type == DOUBLE) {
    if (add) {
      if (*(double*)(vaParm[i].value) + *(double*)(vaParm[i].step) <= *(double*)(vaParm[i].max)) {
	*((double*)(vaParm[i].value)) += *((double*)(vaParm[i].step));
	printf("%s: %f +\n", vaParm[i].name, *((double*)(vaParm[i].value)));
      } else
	printf("%s: %f MAX\n", vaParm[i].name, *((double*)(vaParm[i].value)));
    } else {
      if (*(double*)(vaParm[i].value) - *(double*)(vaParm[i].step) >= *(double*)(vaParm[i].min)) {
	*((double*)(vaParm[i].value)) -= *((double*)(vaParm[i].step));
	printf("%s: %f -\n", vaParm[i].name, *((double*)(vaParm[i].value)));
      } else
	printf("%s: %f MIN\n", vaParm[i].name, *((double*)(vaParm[i].value)));
    }
  }
}

int main(int argc, char *argv[])
{
      char const *vlc_argv[] =
	{
	  //"--no-audio", /* skip any audio track */
	  "--no-xlib", /* tell VLC to not use Xlib */
	};
      int vlc_argc = sizeof(vlc_argv) / sizeof(*vlc_argv);

      SDL_Surface *screen;
      SDL_Event event;
      SDL_Rect rect;
      int done = 0, action = 0;
      
      struct ctx ctx;

      if(argc < 2)
	{
	  printf("Usage: %s <filename>\n", argv[0]);
	  return EXIT_FAILURE;
	}

      ctx.mutex = SDL_CreateMutex();
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
      //libvlc_video_set_key_input(mp, 1);
      size = vec2(width, height);
      
      printf("size: %d %d\n", size.x, size.y);


      /*
       *  Initialise libSDL
       */
      if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTTHREAD) == -1) {
	printf("cannot initialize SDL\n");
	return EXIT_FAILURE;
      }
      ctx.surf = SDL_CreateRGBSurface(SDL_SWSURFACE, size.x, size.y, 16, 0x001f, 0x07e0, 0xf800, 0);

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
	    case SDLK_u:
	      if (vaParmI + 1 >= maxParm)
		vaParmI = 0;
	      else
		vaParmI++;
	      printf("parm: %s\n", vaParm[vaParmI].name);
	      break;
	    case SDLK_i:
	      if (vaParmI - 1 < 0)
		vaParmI = maxParm - 1;
	      else
		vaParmI--;
	      printf("parm: %s\n", vaParm[vaParmI].name);
	      break;
	    case SDLK_o:
	      ChangevaParm(1);
	      break;
	    case SDLK_p:
	      ChangevaParm(0);
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

      SDL_Quit();

      return 0;
}
