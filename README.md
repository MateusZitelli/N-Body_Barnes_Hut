# Barnes-Hut 
> N-Body simulation in linearithmic time 

## Dependencies
Seen that this is a C program that uses OpenGL you'll need to install the basic
tools to build the program and the library glut.

For debian based distros is simple to install what we need:
```sh
$ sudo apt-get install build-essential freeglut3-dev  
```     

## Build
All the project files are in the src folder so:
```sh
$ cd src
```
Then to build from the source is just:
```sh
$ make
```

## Running
After build the binary just run it:

```sh
$ ./nbody.bin
```

All the position from each body in each frame will be stored in a file called
positionData.csv for further studies.

You can navigate in the simulation using the arrow keys, to speed up the process
you can pause using the "r" key, witch will toggle the rendering on and off.
To pause the simulation for a more smooth navigation you can use the "p" key.

**Happy Hack :)**
