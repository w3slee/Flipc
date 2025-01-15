# Particle Simulation Using LibSDL 

Press R to randomize
Use Arrow Keys For Movement

Features
--------

Added particle persistence:

Added an 'active' flag to particles that's always maintained
Increased minimum velocity to prevent complete settling
Added constant random forces to keep particles moving


Improved physics:

Increased damping to 0.99 to preserve more energy
Added stronger minimum velocity enforcement
Added continuous random forces
Increased velocity cap for more dynamic movement


Better initialization:

Ensured particles start well within bounds
Added more initial energy to the system
Improved random distribution of particles


Enhanced collision handling:

Better bounce calculations with energy preservation
Added safety margin to keep particles inside the circle
Enforced minimum velocity after collisions
