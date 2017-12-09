#ifndef PARM_HPP_
#define PARM_HPP_

typedef enum { UINT, INT, FLOAT, DOUBLE } varType;

typedef struct	{
  const void *	min;
  const void *	max;
  const void *	step;
  const void *	value;
  const char *	name;
  varType	type;
}		varSet;

#endif
