#include "Vaparm.hpp"

#ifndef VISUALDEBUGER
/*
#ifdef VSIDEBUG
#define VISUALDEBUGER
#endif
*/

#define VISUALDEBUGER
#endif

#ifndef VISUALDEBUG_HPP_
#define VISUALDEBUG_HPP_

#ifndef VISUALDEBUGER
#define PARMVSVAR
#define PARMVSVAR_GET
#define PARMVSVAR_NEXT
#define PARMVSVAR_PREVIOUS
#define PARMVSVAR_UPDATE

#else

#define PARMVSVAR_GET() Vaparm::getInstance()
#define PARMVSVAR(min, max, step, value, name) PARMVSVAR_GET().push((min), (max), (step), (value), (name))
#define PARMVSVAR_NEXT() PARMVSVAR_GET().next()
#define PARMVSVAR_PREVIOUS() PARMVSVAR_GET().previous()
#define PARMVSVAR_UPDATE(add) PARMVSVAR_GET().update(add)

#endif
#endif

/* ! VISUALDEBUG_HPP_ */
