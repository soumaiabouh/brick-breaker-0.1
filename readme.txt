(a) Name and ID of you and your friend, 
Soumaia Bouhouia - 261053234
Alex Andrianavalontsalama - 260979679

(b) How to run the program, 
The following dependencies are needed:
#include <GL/glew.h>
#include <GL/glut.h>, 
and the following dlls need to be linked to bricks.cpp during compile time for it to run:
freeglut.dll and glew32.dll
I compiled it and ran it using Visual Studio (Soumaia), but it can also be done by linking it dynamically through the command line (Alex).
We included in the zip file "dependencies" the necesssary dll and header files to run the program. 

(c) Things I did not complete
- We were not able to add texture to the elements (did not work on my system (Soumaia)) but we did use shaders to change the colors and added depth by using gradients and by contouring the elements.

(d) Things that are different from the project description, 
- We did not keep the same values for the velocities and the positions since we were using OpenGL and C++.
- We use helper functions to make the code cleaner (ex: we update the ball not in the main loop directly, but in a function called updateBall that we call in the main loop)
- We support larger velocities for the ball
- We support restarting the game when winning or getting a game over
- We have a title page
- We detect collisions based on the position and not the color
- We do not use a clearPaddleZone() as we don't need it


(e) Things completed to the project specifications
- We implemented a drawBall() function
- We added the bricks and drew the static elements at the beginning of the loop
- We implemented the check collisions and handle collisions
- We update the score and lose lives according to the specifications
- The power ups are implemented as described.
- We have 3 sections for the paddle and change the velocity as described in the specifications
- We restart the positions of the paddle and the ball upon losing a life
- We made a hole for the paddle
