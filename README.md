# Puppet Cube PDB Solvers

### Usage

- A makefile is used to compile the code
- The main function in **Main.cpp** is the program's entry point.
- use "make saveOutput" to run the program and save its output in a new file

### Modules

- **Puppet**  
  State representation of the cube, including functions for computing
  ranks and keys and additional utility methods.

- **PuppetShape**  
  Shape representation of the cube with the simulation of the
  blocking mechanism.

- **Table**  
  Methods to generate the shape tables and pattern databases (PDBs) required
  for solving.

- **Solver**  
  Contains the different PDB solver implementations, as well as
  methods for generating and solving sets of scrambles.

- **Main**  
  program's entry point.

### Experimental Data

- results_BA contains the results evaluated in the thesis.
- Scramble set used in the experiments: scrambles_500_sorted, scrambles_20perdistance
