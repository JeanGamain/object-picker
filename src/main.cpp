/* libSDL and libVLC sample code
 * Copyright © 2008 Sam Hocevar <sam@zoy.org>
 * license: [http://en.wikipedia.org/wiki/WTFPL WTFPL] */

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#include <SDL/SDL.h>
#include <SDL/SDL_mutex.h>

#include <vlc/vlc.h>

#include "image.hpp"
#include "pixel16.hpp"
#include "Canny.hpp"

#define WIDTH 512
#define HEIGHT 377

#define VIDEOWIDTH 512
#define VIDEOHEIGHT 377

float diffa = 0;
float diffb = 0;
float diffc = 0;
float diffd = 0;

int pause = 0;

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

static float lineRepartitor(int i, int max) {
  return ((float)(i * i) / (float)(max * max));
}

char pixelDiff(uint16_t a, uint16_t b) {
  // R: 0x001f, V:0x07e0, B:0xf800
  if (a == 0xFFFF || b == 0xFFFF)
    return 0;
  return (((float)(abs((a & 0x001f) - (b & 0x001f)) +
	  abs((a & 0x07e0 >> 5) - (b & 0x07e0 >> 5)) +
	   abs((a & 0xf800 >> 11) - (b & 0xf800 >> 11))) / 3 / 32)
	  > 0.04);

}

void detectBorder(uint16_t *pixels, int x, int y) {
  const int dirVec[8][2] = {{-1, -1}, {0, -1}, {1, -1}, {-1, 0}, {1, 0}, {-1, 1}, {0, 1}, {1, 1}};
  int rx, ry;
  
  for (int i = 0; i < 8; ++i) {
    rx = x + dirVec[i][0];
    if (rx < 0 || rx > VIDEOWIDTH)
      continue;
    ry = y + dirVec[i][1];
    if (ry < 0 || ry > VIDEOHEIGHT)
      continue;
    if (pixelDiff(pixels[y * VIDEOWIDTH + x], pixels[ry * VIDEOWIDTH + rx]) == 1) {
      pixels[y * VIDEOWIDTH + x] = 0xffff;
      detectBorder(pixels, rx, ry);
    }
  }
}

static void objectDetectionOverlay(void *const *pixelsbuffer) {
  uint16_t *pixels = static_cast<uint16_t *>(*pixelsbuffer);
  const int line = 18;
  int x, y, i;

  for (i = 0; i < line; ++i) {
    if (i == 0)
      x = VIDEOWIDTH / 2;
    else
      x = (int)(((VIDEOWIDTH - 10) / 2) * lineRepartitor((i / 2), line / 2)) * (i % 2 == 0 ? -1 : 1) + (VIDEOWIDTH / 2);
    /*if (x > VIDEOWIDTH)
      x = VIDEOWIDTH;
    else if (x < 0)
      x = 0;*/
    for (y = 0; y < (VIDEOHEIGHT / 2); ++y) {
      //haut
      detectBorder(pixels, x, y);
      // bas
      detectBorder(pixels, x, VIDEOHEIGHT - y);
    }
    //detectBorder(pixels, x, VIDEOHEIGHT / 2);
  }
}

void showfps()
{
  static unsigned int lasttime = 0;
  static unsigned int nbframe = 0;
  unsigned int curtime;
  unsigned int fps;
  static unsigned int maxfps = 0;

  curtime = (unsigned)time(NULL);
  if ((fps = curtime - lasttime) > 0)
    {
      fps = nbframe / fps;
      if (maxfps < fps)
	maxfps = fps;
      lasttime = curtime;
      nbframe = 0;
      printf("%d fps - %d max\n", fps, maxfps);
    }
  else
    nbframe++;
}

static void unlock(void *data, void *, void *const *p_pixels)
{
  struct ctx *ctx = static_cast<struct ctx *>(data);
  static float lastDiffa = diffa;
  static float lastDiffb = diffb;
  static float lastDiffc = diffc;
  static float lastDiffd = diffd;
  static Canny * cannyy = new Canny(vec2(VIDEOWIDTH, VIDEOHEIGHT), 255, 37, 100, 1.7f, 0.0f);
  
  /* VLC just rendered the video, but we can also render stuff */
  image<pixel16> * img = new image<pixel16>(VIDEOWIDTH, VIDEOHEIGHT, static_cast<pixel16 *>(*p_pixels));
    //image<pixel16> * img2 = new image<pixel16>(VIDEOWIDTH, VIDEOHEIGHT);

  static image<pixelf> *in = new image<pixelf>(img->size);
  static image<pixelf> *out = new image<pixelf>(img->size);

  out->clear();
  
  for (int x = 0; x < in->size.x; x++) {
    for (int y = 0; y < in->size.y; y++) {
      in->pixel[y * in->size.x + x].set(img->pixel[y * in->size.x + x].get());
    }
  }

  if (lastDiffa != diffa) {
    cannyy->setMin(37 + diffa);
    lastDiffa = diffa;
  }
  
  if (lastDiffb != diffb) {
    cannyy->setMax(100 + diffb);
    lastDiffb = diffb;
  }
  
  if (lastDiffc != diffc) {
    cannyy->setBlur(1.7f + diffc);
    lastDiffc = diffc;
  }

  if (lastDiffd != diffd) {
    cannyy->setResize(0.7f + diffd);
    lastDiffd = diffd;
  }

  cannyy->edgeDetection(in, out);
  for (int x = 0; x < img->size.x; x++) {
    for (int y = 0; y < img->size.y; y++) {
      //img->pixel[y * img->size.x + x].set(out->pixel[y * img->size.x + x].get());
      if (out->pixel[y * img->size.x + x].get() == 255)
	img->pixel[y * img->size.x + x].setrvb(255, 0, 0);
    }
  }
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
  libvlc_instance_t *libvlc;
  libvlc_media_t *m;
  libvlc_media_player_t *mp;
      char const *vlc_argv[] =
	{
	  "--no-audio", /* skip any audio track */
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
       *  Initialise libSDL
       */
      if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTTHREAD) == -1)
	{
	  printf("cannot initialize SDL\n");
	  return EXIT_FAILURE;
	}

      empty = SDL_CreateRGBSurface(SDL_SWSURFACE, VIDEOWIDTH, VIDEOHEIGHT,
				   32, 0, 0, 0, 0);
      ctx.surf = SDL_CreateRGBSurface(SDL_SWSURFACE, VIDEOWIDTH, VIDEOHEIGHT,
				      16, 0x001f, 0x07e0, 0xf800, 0);

      ctx.mutex = SDL_CreateMutex();

      int options = SDL_ANYFORMAT | SDL_HWSURFACE | SDL_DOUBLEBUF;

      screen = SDL_SetVideoMode(WIDTH, HEIGHT, 0, options);
      if(!screen)
	{
	  printf("cannot set video mode\n");
	  return EXIT_FAILURE;
	}

      /*
       *  Initialise libVLC
       */
      libvlc = libvlc_new(vlc_argc, vlc_argv);
      m = libvlc_media_new_path(libvlc, argv[1]);
      mp = libvlc_media_player_new_from_media(m);
      libvlc_media_release(m);

      libvlc_video_set_callbacks(mp, lock, unlock, display, &ctx);
      libvlc_video_set_format(mp, "RV16", VIDEOWIDTH, VIDEOHEIGHT, VIDEOWIDTH*2);
      libvlc_media_player_play(mp);

      /*
       *  Main loop
       */
      rect.w = 0;
      rect.h = 0;

      while(!done)
	{
	  action = 0;

	  /* Keys: enter (fullscreen), space (pause), escape (quit) */
	  while( SDL_PollEvent( &event ) )
	    {
	      switch(event.type)
		{
		case SDL_QUIT:
		  done = 1;
		  break;
		case SDL_KEYDOWN:
		  action = event.key.keysym.sym;
		  break;
		}
	    }

	  switch(action)
	    {
	    case SDLK_ESCAPE:
	      done = 1;
	      break;
	    case SDLK_RETURN:
	      options ^= SDL_FULLSCREEN;
	      screen = SDL_SetVideoMode(WIDTH, HEIGHT, 0, options);
	      break;
	    case SDLK_a:
	      pause = !pause;
	      libvlc_media_player_set_pause(mp, pause);
	      break;
	    case SDLK_o:
	      if (diffa + 1 < 250)
		diffa += 1;
	      printf("diffa+ at %f\n", diffa);
	      break;
	    case SDLK_p:
	      if (diffa - 1 > -200)
		diffa -= 1;
	      printf("diffa- at %f\n", diffa);	      
	      break;
	    case SDLK_j:
	      if (diffb + 1 < 250)
		diffb += 1;
	      printf("diffb+ at %f\n", diffb);
	      break;
	    case SDLK_k:
	      if (diffb - 1 > -200)
		diffb -= 1;
	      printf("diffb- at %f\n", diffb);	      
	      break;
	    case SDLK_b:
	      if (diffc + 0.01 < 250)
		diffc += 0.01;
	      printf("diffc+ at %f\n", diffc);
	      break;
	    case SDLK_n:
	      if (diffc - 0.01 > -200)
		diffc -= 0.01;
	      printf("diffc- at %f\n", diffc);
	      break;
	    case SDLK_q:
	      if (diffd + 0.01 < 2)
		diffd += 0.01;
	      printf("diffd+ at %f\n", diffd);
	      break;
	    case SDLK_s:
	      if (diffd - 0.01 > 0)
		diffd -= 0.01;
	      printf("diffd- at %f\n", diffd);
	      break;
	    }

	  /* Blitting the surface does not prevent it from being locked and
	   * written to by another thread, so we use this additional mutex. */
	  SDL_LockMutex(ctx.mutex);
	  SDL_BlitSurface(ctx.surf, NULL, screen, &rect);
	  SDL_UnlockMutex(ctx.mutex);

	  SDL_Flip(screen);
	  SDL_Delay(10);

	  SDL_BlitSurface(empty, NULL, screen, &rect);
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
