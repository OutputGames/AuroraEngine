#ifndef EGS_HPP
#define EGS_HPP

#include "utils/utils.hpp"

struct SaveState
{
    string data;
};

struct EditorGameSystem
{

  static void LoadSaveState(SaveState* state);
  static SaveState* SaveCurrentState();
    
};


#endif