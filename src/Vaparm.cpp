#include "Vaparm.hpp"

void	Vaparm::push(unsigned int min, unsigned int max, unsigned int step,
		    const unsigned int* value, const char* name) {
  parms.push_back((Vaparm::varSet){
        new (unsigned int)(min),
	new (unsigned int)(max),
	new (unsigned int)(step),
	value, name, UINT
	}
    );
  if (i == parms.end()) {
    i = parms.begin();
  }
}

void	Vaparm::push(int min, int max, int step, const int * value, const char * name) {
  parms.push_back((Vaparm::varSet){ new int(min), new int(max), new int(step), value, name, INT });
  if (i == parms.end()) {
    i = parms.begin();
  }
}

void	Vaparm::push(long int min, long int max, long int step, const long int * value, const char * name) {
  parms.push_back((Vaparm::varSet){ new int(min), new int(max), new int(step), value, name, INT });
  if (i == parms.end()) {
    i = parms.begin();
  }
}
    
void	Vaparm::push(float min, float max, float step, const float * value, const char * name) {
  parms.push_back((Vaparm::varSet){ new float(min), new float(max), new float(step), value, name, FLOAT });
  if (i == parms.end()) {
    i = parms.begin();
  }
}
  
void	Vaparm::push(double min, double max, double step, const double * value, const char * name) {
  parms.push_back((Vaparm::varSet){ new double(min), new double(max), new double(step), value, name, DOUBLE });
  if (i == parms.end()) {
    i = parms.begin();
  }
}

void	Vaparm::next() {
  if (++i == parms.end()) {
    i = parms.begin();
  }
  callPlayers((*i).name);
}

void	Vaparm::previous() {
  if (i == parms.begin()) {
    i = std::prev(parms.end());
  } else {
    i = std::prev(i);
  }
  callPlayers((*i).name);
}

void	Vaparm::update(char add) {
  if (i != parms.end()) {
    callPlayers((this->*(updaters[(*i).type]))(add));
  }
}

void	Vaparm::showChanges() {
  for (std::list<varSet>::iterator a = parms.begin(); a != parms.end(); a++) {
    (this->*(updaters[(*i).type]))(true);
    callPlayers((this->*(updaters[(*i).type]))(false));
  }
}

void	Vaparm::pushPlayer(player dsp) {
  players.push_back(dsp);
}

void	Vaparm::callPlayers(const char * str) {
  for (std::list<player>::const_iterator a = players.begin(); a != players.end(); a++) {
    (*a)(str);
  }
}

char *	Vaparm::updateUINT(char add) {
  if (add) {
    if (*(unsigned int*)((*i).value) + *(unsigned int*)((*i).step) <= *(unsigned int*)((*i).max)) {
      *((unsigned int*)((*i).value)) += *((unsigned int*)((*i).step));
      snprintf(msgBuffer, sizeof(msgBuffer), "%s: %u +\n",
	       (*i).name, *(unsigned int*)((*i).value));
    } else {
      snprintf(msgBuffer, sizeof(msgBuffer), "%s: %u MAX\n",
	       (*i).name, *(unsigned int*)((*i).value));
    }
  } else {
    if (*(unsigned int*)((*i).value) != 0 && *(unsigned int*)((*i).value) - *(unsigned int*)((*i).step) >= *(unsigned int*)((*i).min)) {
      *((unsigned int*)((*i).value)) -= *((unsigned int*)((*i).step));
      snprintf(msgBuffer, sizeof(msgBuffer), "%s: %u -\n",
	       (*i).name, *(unsigned int*)((*i).value));
    } else {
      snprintf(msgBuffer, sizeof(msgBuffer), "%s: %u MIN\n",
	       (*i).name, *(unsigned int*)((*i).value));
    }
  }
  return msgBuffer;
}

char *	Vaparm::updateINT(char add) {
  if (add) {
    if (*(int*)((*i).value) + *(int*)((*i).step) <= *(int*)((*i).max)) {
      *((int*)((*i).value)) += *((int*)((*i).step));
      snprintf(msgBuffer, sizeof(msgBuffer), "%s: %d +\n",
	       (*i).name, *(int*)((*i).value));
    } else
      snprintf(msgBuffer, sizeof(msgBuffer), "%s: %d MAX\n",
	       (*i).name, *(int*)((*i).value));
  } else {
    if (*(int*)((*i).value) - *(int*)((*i).step) >= *(int*)((*i).min)) {
      *((int*)((*i).value)) -= *((int*)((*i).step));
      snprintf(msgBuffer, sizeof(msgBuffer), "%s: %d -\n",
	       (*i).name, *(int*)((*i).value));
    } else {
      snprintf(msgBuffer, sizeof(msgBuffer), "%s: %d MIN\n",
	       (*i).name, *(int*)((*i).value));
    }
  }
  return msgBuffer;
}

char *	Vaparm::updateFLOAT(char add) {
  if (add) {
    if (*((float*)((*i).value)) + *((float*)((*i).step)) <= *(float*)((*i).max)) {
      *((float*)((*i).value)) += *((float*)((*i).step));
      snprintf(msgBuffer, sizeof(msgBuffer), "%s: %f +\n",
	       (*i).name, *((float*)((*i).value)));
    } else
      snprintf(msgBuffer, sizeof(msgBuffer), "%s: %f MAX\n",
	       (*i).name, *((float*)((*i).value)));
  } else {
    if (*((float*)((*i).value)) - *((float*)((*i).step)) >= *(float*)((*i).min)) {
      *((float*)((*i).value)) -= *((float*)((*i).step));
      snprintf(msgBuffer, sizeof(msgBuffer), "%s: %f -\n",
	       (*i).name, *((float*)((*i).value)));
    } else {
      snprintf(msgBuffer, sizeof(msgBuffer), "%s: %f MIN\n",
	       (*i).name, *((float*)((*i).value)));
    }
  }
  return msgBuffer;
}

char *	Vaparm::updateDOUBLE(char add) {
  if (add) {
    if (*(double*)((*i).value) + *(double*)((*i).step) <= *(double*)((*i).max)) {
      *((double*)((*i).value)) += *((double*)((*i).step));
      snprintf(msgBuffer, sizeof(msgBuffer), "%s: %f +\n",
	       (*i).name, *((double*)((*i).value)));
    } else {
      snprintf(msgBuffer, sizeof(msgBuffer), "%s: %f MAX\n",
	       (*i).name, *((double*)((*i).value)));
    }
  } else {
    if (*(double*)((*i).value) - *(double*)((*i).step) >= *(double*)((*i).min)) {
      *((double*)((*i).value)) -= *((double*)((*i).step));
      snprintf(msgBuffer, sizeof(msgBuffer), "%s: %f -\n",
	       (*i).name, *((double*)((*i).value)));
    } else {
      snprintf(msgBuffer, sizeof(msgBuffer), "%s: %f MIN\n",
	       (*i).name, *((double*)((*i).value)));
    }
  }
  return msgBuffer;
}
