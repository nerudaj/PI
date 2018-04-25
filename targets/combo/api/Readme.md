# Combo card control API

## Introduction

Source files found in this folder are part of the Combo-CG control API.
They will merge with the "old" API into libp4dev in the future but for
now these two parts are separate.

## File overview

 * P4Dev.hpp - The main header file for the API. The only thing you need to include
 * Device.hpp - Declaration of the Device class. This models the abstraction over Combo-CG card
 * Device.cpp - Implementation of Device class methods
 * Table.hpp - Declaration of the Table class. This models the abstraction over tables defined in P4 program
 * Table.cpp - Implementation of Table class methods
 * RegisterArray.cpp - Memory manager around Register class
 * Register.hpp - Declaration of the Register class. This models the abstraction over registers, counters and meters defined in P4 program
 * Register.cpp - Implementation of Register class methods
 
## Compilation

At this moment, if you want to compile your source file against this API,
you have to compile Table.cpp, Register.cpp, RegisterArray.cpp Device.cpp into object files and add
them to your binary. Dynamic library libp4dev has to be linked to the
binary as well. Consult Makefile.example for more info.

For compiling, use at least C++11 or higher.