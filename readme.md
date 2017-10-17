Cemetech Contest #20 Entry (Space) - Loonar Landers
===================================================

Warning:
* This only works on the TI-84+ CE.
* This will not work on the TI-84+ CSE.
* This will not work on any monochrome calculator.

About
-----
You are a loon and it is your job to softly land your fragile lander onto
the designated landing pad. 

Installing and Running
----------------------

Use your favorite computer to calculator link software (TiLP, TIConnect, etc)
to send `LOONARLA.8xp` to your calculator.

To run without a shell, follow these steps:
1. Turn on the calculator and make sure you're on the home screen.
2. Then push the following keys in order:
   [CLEAR] [2nd] [0] [DOWN] [DOWN] [DOWN] [DOWN] [DOWN] [DOWN] [ENTER]
3. Push [PRGM] then move the cursor down until LOONARLA is selected.
   When you have done that, push [ENTER]
4. Your homescreen should look something like:

   `Asm(prgmLOONARLA)`

   Then push [ENTER] to start the game.
   
Controls
--------

| Key    | Function in menu
| ------:|:----------------
| [Up], [Down] | Change menu options
| [2nd]  | Select the option
| [Mode] | Quit the game


| Key(s)  | Function in game
| ----:|:----------------
| [Left],[Right] | Fires thrusters to move lander
| [Mode] | Return to the main menu
   
Troubleshooting
---------------
1. * Q: When I run the game, the calculator gives me `ERROR:ARCHIVED`
   * A: Follow the grey text prompt to unachive the variable.

2. * Q: When I run the game, the calculator is complaining
        about needing this "Libload" or something.
   * A: You need the libload library. Follow the link provided in the
        error message to find the needed libraries.
		
3. * Q: When I run the game, the calculator is complaining about
        library version or something like that.
   * A: You need to find the library specified in the error message.
        Follow the link provided to find the needed libraries.
		
4. * Q: I think I might have found a bug.
   * A: Read the thread and post about it if the problem hasn't been reported yet.
        The project's thread is here: https://www.cemetech.net/forum/viewtopic.php?p=261077

License
-------
I use the BSD 2-clause license for this project.
See `LICENSE` for more details.

Version History
---------------

* 0.1 - The project, as submitted to Cemetech for the contest.
* 0.2 -  Added side collision, menu, speed consistency, difficulty levels
		 and high score saving.
		 Optimized image data for size, program for size and speed.
		
How to Build the Project
------------------------

1. Download and install the CE C SDK if you don't have it installed.
   https://github.com/CE-Programming/toolchain/releases
2. (Re)build image data by navigating to the `src/gfx` folder and running `convpng`
3. Build main project by running `make` in the project's root directory.
4. If everything worked out, `LOONARLA.8xp` will be found in the `bin` folder