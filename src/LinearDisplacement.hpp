#ifndef LINEARDISPLACEMENT_HPP_
# define LINEARDISPLACEMENT_HPP_

#include "vec2f.hpp"

class LinearDisplacement {
public:
  LinearDisplacement(vec2f const & a, vec2f const & b);
  virtual ~LinearDisplacement();
  
  bool	end();
  void	goNext();
  vec2	get();

private:
  int	i[3];
  int	cumul;
  vec2f	vr[3];
  vec2f	e;
  vec2f	position;
};

#endif /* !LINEARDISPLACEMENT */
