#ifndef VAPARM_HPP_
# define VAPARM_HPP_

#include <list>
#include <iterator>

class Vaparm {
public:
  typedef enum { UINT, INT, FLOAT, DOUBLE } varType;

  typedef struct	{
    const void *	min;
    const void *	max;
    const void *	step;
    const void *	value;
    const char *	name;
    varType		type;
  }			varSet;

  typedef void		(*player)(const char*);
  
private:
  Vaparm(player dsp) :
    i(parms.begin()),
    updaters {
      &Vaparm::updateUINT,
      &Vaparm::updateINT,
      &Vaparm::updateFLOAT,
      &Vaparm::updateDOUBLE
    }
{
  players.push_back(dsp);
}
  
public:
  static Vaparm &	getInstance()
  {
    static Vaparm	instance([](const char* msg) { printf("%s\n", msg); });

    return instance;
  }

  Vaparm	(Vaparm const&) = delete;
  void		operator=(Vaparm const&)  = delete;
  
  void	push(unsigned int min, unsigned int max, unsigned int step,
	     const unsigned int* value, const char* name);
  void	push(long int min, long int max, long int step, const long int * value, const char* name);
  void	push(int min, int max, int step, const int * value, const char* name);
  void	push(float min, float max, float step, const float* value, const char* name);
  void	push(double min, double max, double step, const double* value, const char* name);
  
  void	next();
  void	previous();
  void	update(char add);
  void	showChanges();
  void	pushPlayer(player);
  
private:
  void	callPlayers(const char* str);

  char*	updateUINT(char add);
  char*	updateINT(char add);
  char*	updateFLOAT(char add);
  char*	updateDOUBLE(char add);

  
private:

  std::list<varSet>		parms;
  std::list<varSet>::iterator	i;
  std::list<player>		players;
  char				msgBuffer[80];
  
  char *			(Vaparm::*updaters[4])(char);
};

#endif /* !VAPARM_HPP_ */
