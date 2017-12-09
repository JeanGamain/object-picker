#include "LinearDisplacement.hpp"

LinearDisplacement::LinearDisplacement(vec2f const & a, vec2f const & b)
  : e(b), position(a)
{
  vr[1] = a;
  vr[0].x = b.x - a.x;
  vr[0].y = b.y - a.y;
  vr[2].x = (vr[0].x > 0) ? 1 : -1;
  vr[2].y = (vr[0].y > 0) ? 1 : -1;
  vr[0].x = (vr[0].x < 0) ? -vr[0].x : vr[0].x;
  vr[0].y = (vr[0].y < 0) ? -vr[0].y : vr[0].y;
  i[0] = 1;
  i[2] = !(i[1] = (vr[0].x > vr[0].y));
  cumul = vr[0][i[2]] / 2;
}

LinearDisplacement::~LinearDisplacement() {
}

bool	LinearDisplacement::end()
{
  return i[0] > vr[0][i[2]];
}

void	LinearDisplacement::goNext()
{
  i[0]++;
  vr[1][i[2]] = vr[1][i[2]] + vr[2][i[2]];
  cumul = cumul + vr[0][i[1]];
  if (cumul >= vr[0][i[2]])
    {
      cumul = cumul - vr[0][i[2]];
      vr[1][i[1]] = vr[1][i[1]] + vr[2][i[1]];
    }
}

vec2	LinearDisplacement::get()
{
  vec2f next = vr[1];
  if (!end())
    goNext();
  return next.asDec();
}
