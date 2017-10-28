#include <assert.h>
#include <list>
#include <iterator>
#include "LinearDisplacement.hpp"
#include "ObjectPicker.hpp"

extern float diffa;
extern float diffb;
extern float diffc;
extern float diffd;

ObjectPicker::ObjectPicker(vec2 size)
  : truesize(size),
    resize(0.0f),
    size((resize > 0.0f) ? size * vec2(resize, resize) : size),
    dump(1),
    minlength(1), //10
    tmin(50),
    tmax(70),
    sigma(1.5f),
    canny(new Canny(size, dump, minlength, tmin, tmax, sigma)),
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
    canny->setMinLength(minlength + lasttmin);
  }

  if (lasttmax != diffc) {
    lasttmax = diffc;
    canny->setMax(tmax + lasttmax);
  }

  if (lastsigma != diffd) {
    lastsigma = diffd;
    canny->setBlur(sigma + lastsigma);
  }
  
  #pragma omp for
  for (int x = 0; x < (img->size.x * img->size.y); x++) {
      inbw->pixel[x].set(img->pixel[x].get());
  }
  /*
  if (resize > 0) {
    in->resize(newin, resize, resize);
    newin = G;
    }*/
  
  image<pixelf> * scany = canny->scan(inbw);
  
  const vec2 rayon = vec2(10, 0); // pixel
  const vec2 center = scany->size / 2;
  const int ray = 9;
  vec2 pos, vector;
  colorSplit	maxSplit[ray];
  colorSplit	split;
  unsigned long colorComponent[3];
  std::list<colorSplit> lineSplit[ray];

  for (int i = 0; i < ray; i++) {
    vector = rayon;
    vector.rotate(360.0f / ray * float(i));
    LinearDisplacement line(center + vector, vector * img->size.y);
    for (pos = line.get();
	 pos > vec2(0, 0) && pos < scany->size && !line.end();) {
      while (pos > vec2(0, 0) && pos < scany->size && !line.end() // jump edge
	     && scany->pixel[pos.to1D(img->size.x)] >= tmin)
	pos = line.get();
      split.position = pos;
      split.length = 0;
      colorComponent[0] = 0;
      colorComponent[1] = 0;
      colorComponent[2] = 0;
      for (;pos > vec2(0, 0) && pos < scany->size && !line.end()
	     && scany->pixel[pos.to1D(img->size.x)] < tmax;
	   pos = line.get()) {
	split.length++;
	colorComponent[0] += img->pixel[pos.to1D(img->size.x)].getr();
	colorComponent[1] += img->pixel[pos.to1D(img->size.x)].getv();
	colorComponent[2] += img->pixel[pos.to1D(img->size.x)].getb();
	img->pixel[pos.to1D(img->size.x)].pixel = uint16_t(i * lineSplit[i].size() * (65025 / (ray * 18)));
      }
      if (split.length > minlength) {
	colorComponent[0] /= split.length;
	colorComponent[1] /= split.length;
	colorComponent[2] /= split.length;
	split.color.setrvb(colorComponent[0], colorComponent[1], colorComponent[2]);
	if (lineSplit[i].size() == 0 || maxSplit[i].length < split.length) {
	  maxSplit[i] = split; 
	}
	lineSplit[i].push_front(split);
      }
    }
  }
 
  std::list<Canny::edge> * edges = canny->get();
  for (std::list<Canny::edge>::const_iterator i = edges->begin();
       i != edges->end();
       ++i) {
    img->pixel[(*i).pos.to1D(img->size.x)].setrvb(0, 0, 255);
    for (std::list<int>::const_iterator j = (*i).point->begin();
	 j != (*i).point->end();
	 ++j) {
      // if color == max or before
      img->pixel[(*j).position].pixel = uint16_t((*i).color * (65025 / 255));
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

float	ObjectPicker::getResize() const {
  return resize;
)

void	ObjectPicker::setResize(float r) {
  if (r <= 0) {
    size = truesize;
  } else {
    size = vec2(cordinate(r * truesize.x), cordinate(r * truesize.y));
  }
  // resize canny
  resize = r;
}
