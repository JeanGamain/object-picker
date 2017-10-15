#include <assert.h>
#include <list>
#include <iterator>
#include "ObjectPicker.hpp"

extern float diffa;
extern float diffb;
extern float diffc;
extern float diffd;

ObjectPicker::ObjectPicker(vec2 size)
  : size(size),
    dump(5),
    minlength(10),
    tmin(50),
    tmax(65),
    sigma(1.0f),
    resize(0.0f),
    canny(new Canny(size, dump, minlength, tmin, tmax, sigma, resize)),
    inbw(new image<pixelf>(size)),
    lock(0)
{
}

ObjectPicker::~ObjectPicker() {
  delete canny;
  delete inbw;
}

unsigned int	ObjectPicker::detect(image<pixel16> * img) {
  static float lastdump = diffa;
  static float lasttmin = diffb;
  static float lasttmax = diffc;
  static float lastsigma = diffd;

  assert(img != NULL && img->pixel != NULL);
  assert(img->size == size);

  if (lastdump != diffa) {
    lastdump = diffa;
    canny->setDump(dump + lastdump);
  }
  
  if (lasttmin != diffb) {
    lasttmin = diffb;
    canny->setMin(tmin + lasttmin);
  }

  if (lasttmax != diffc) {
    lasttmax = diffc;
    canny->setMax(tmax + lasttmax);
  }

  if (lastsigma != diffd) {
    lastsigma = diffd;
    canny->setBlur(sigma + lastsigma);
  }
  for (int x = 0; x < img->size.x; x++) {
    for (int y = 0; y < img->size.y; y++) {
      inbw->pixel[y * img->size.x + x].set(img->pixel[y * img->size.x + x].get());
    }
  }
  
  std::list<Canny::edge> * edges = canny->edgeDetection(inbw);
  
  printf("detected edges%d : %lu\n", img->size.x, edges->size());
  for (std::list<Canny::edge>::const_iterator i = edges->begin();
       i != edges->end();
       ++i) {
    img->pixel[(*i).pos.y * img->size.x + (*i).pos.x].setrvb(0, 255, 0);
    for (std::list<vec2>::const_iterator j = (*i).point->begin();
	 j != (*i).point->end();
	 ++j) {
      if ((*i).loop) {
	img->pixel[(*j).to1D(img->size.x)].setrvb(255, 0, 0);
      } else {
	img->pixel[(*j).to1D(img->size.x)].setrvb(0, 0, 255);
      }
    }
  }
  
  return 0;
}

bool ObjectPicker::setLock(unsigned int l) {
  lock = l;
  return true;
}

unsigned int ObjectPicker::getLock() const {
  return lock;
}

/*
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
void de(uint16_t *pixels, int x, int y) {
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
*/
/*
static void objectDetectionOverlay(void *const *pixelsbuffer) {
  uint16_t *pixels = static_cast<uint16_t *>(*pixelsbuffer);
  const int line = 18;
  int x, y, i;

  for (i = 0; i < line; ++i) {
    if (i == 0)
      x = VIDEOWIDTH / 2;
    else
      x = (int)(((VIDEOWIDTH - 10) / 2) * lineRepartitor((i / 2), line / 2)) * (i % 2 == 0 ? -1 : 1) + (VIDEOWIDTH / 2);
    //if (x > VIDEOWIDTH)
    //  x = VIDEOWIDTH;
    //else if (x < 0)
    //  x = 0;
    for (y = 0; y < (VIDEOHEIGHT / 2); ++y) {
      //haut
      detectBorder(pixels, x, y);
      // bas
      detectBorder(pixels, x, VIDEOHEIGHT - y);
    }
    //detectBorder(pixels, x, VIDEOHEIGHT / 2);
  }
}
*/
