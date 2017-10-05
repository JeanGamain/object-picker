/* libSDL and libVLC sample code
 * Copyright © 2008 Sam Hocevar <sam@zoy.org>
 * license: [http://en.wikipedia.org/wiki/WTFPL WTFPL] */

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>

#include <SDL/SDL.h>
#include <SDL/SDL_mutex.h>

#include <vlc/vlc.h>

#include "image.hpp"
#include "pixel16.hpp"
#include "canny.hpp"

#define WIDTH 520
#define HEIGHT 380

#define VIDEOWIDTH 520
#define VIDEOHEIGHT 380

float diff = 0;
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
	  > diff);

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

static void unlock(void *data, void *, void *const *p_pixels)
{
  struct ctx *ctx = static_cast<struct ctx *>(data);

  /* VLC just rendered the video, but we can also render stuff */
  image<pixel16> * img = new image<pixel16>(VIDEOWIDTH, VIDEOHEIGHT, static_cast<pixel16 *>(*p_pixels));
    //image<pixel16> * img2 = new image<pixel16>(VIDEOWIDTH, VIDEOHEIGHT);

  image<pixelf> *in = new image<pixelf>(img->size);
  image<pixelf> *out = new image<pixelf>(img->size);
  
  for (int x = 0; x < in->size.x; x++) {
    for (int y = 0; y < in->size.y; y++) {
      in->pixel[y * in->size.x + x].set(img->pixel[y * in->size.x + x].get());
    }
  }

  canny_edge_detection(in, out, 255, 29, 56, 1.84f + diff);

  for (int x = 0; x < img->size.x; x++) {
    for (int y = 0; y < img->size.y; y++) {
      if (out->pixel[y * img->size.x + x].get() == 255.0)
	img->pixel[y * img->size.x + x].set(255.0f);
    }
  }

  printf("rdr\n");
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
	      if (diff + 0.001 < 50)
		diff += 0.001;
	      printf("diff+ at %f\n", diff);
	      break;
	    case SDLK_p:
	      if (diff - 0.001 > -2)
		diff -= 0.001;
	      printf("diff- at %f\n", diff);	      
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
