#pragma once

#include "main.h"

// Returns whether or not the variable in the specified instance exists.
bool InstanceVariableExists(RValue inst, string name);

// Returns a vector containing all of the names of the variables within the instance.
vector<string> GetInstanceVariableNames(RValue inst);

// Returns the amount of variables present in the instance.
int GetInstanceVariableCount(RValue inst);

// Returns the value of the specified variable within the instance.
RValue GetInstanceVariable(RValue inst, string name);

// Sets the variable to the specified value.
void SetInstanceVariable(RValue inst, string name, RValue value);

// Returns whether or not the global variable exists.
bool GlobalVariableExists(string name);

// Gets the value of the specified global variable.
RValue GetGlobalVariable(string name);

// Sets the global variable to the specified value.
void SetGlobalVariable(string name, RValue value);

// Creates a reference to a variable in an instance or struct.
RValue CreateReference(RValue inst, RValue var, int index = 0);

// Returns the asset from the specified name.
RValue GetAsset(string name);