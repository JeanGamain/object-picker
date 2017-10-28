#ifndef LINEARDISPLACEMENT_HPP_
# define LINEARDISPLACEMENT_HPP_

#include "math.hpp"

class LinearDisplacement {
public:
  LinearDisplacement(vec2 const & a, vec2 const & b);
  virtual ~LinearDisplacement();
  
  bool	end();
  void	goNext();
  vec2	get();

private:
  int	i[3];
  int	cumul;
  vec2	vr[3];
  vec2	e;
  vec2	position;
};

#endif /* !LINEARDISPLACEMENT */
