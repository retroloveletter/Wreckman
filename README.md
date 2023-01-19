# 3do-wreck-man
Retroloveletter https://www.youtube.com/@retroloveletter

A clone of classic Pacman without Pacman assets. This is not a port, as existing game source code was not used/adapted for 3DO. 

Originally created on 12-May-2021, refactored in 2023 prior to release.

NTSC and PAL are supported.

# Soapbox	 
This work is **not affiliated with Namco** and will not be released by the creator using namco or like assets.
The creator is not responsible if others mod this program to resemble pacman.
This work was created for enjoyment purposes, not to sell nor infringe on copyrights/trademarks.

# Tech details 
- I developed this on Windows 10 using cygwin.
- launch.sh fires up retroarch, loads the Opera core, and launches the iso.
- Calls to printLine() won't try to print anything unless DEBUG_VERBOSE is set to 1.
- DEBUG_VERBOSE should be set to 0 for production build.
- Audio files are mono 44100Hz 32-bit float Aiff files. 
- Easter egg: Hold X and B buttons together during startup to make pacman invisible.
- The intermission is a separate 3DO program I wrote to test 3DO's task launcher capabilities.
  The game task will execute the intermission as a separate task, then return after it is done.
	If intermission code is updated, rename its LaunchMe to Intermission and place it in the game's CD folder.
	The intermission program references the same asset folder as the game program.

# Score system

10 points for each pellet eaten.
	
50 points for each powerup eaten.
	
There are 8 fruit types presented in order, below are the points for each fruit eaten:
- 100
- 300
- 500
- 700
- 1000
- 2000
- 3000
- 5000

Fruits only spawn twice per level once you eat enough pellets.

Points for eating ghosts during powerup phase = (number of ghosts eaten in succession) x 200

You get 1 and only 1 extra life at 10K points.
