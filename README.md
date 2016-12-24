# Barnes-Hut 
> N-Body simulation in linearithmic time

This project aims to develop and analyse the [Barnes-Hut algorithm](http://arborjs.org/docs/barnes-hut), to solve the problem of n-bodies attracting each other gravitationally in O(n * log(n)) time.

The source code is of the implementation is in C in the ```src``` folder, and a profile written in Python can be found in the ```profiler``` folder.

![](https://github.com/MateusZitelli/N-Body_Barnes_Hut/raw/master/profiler/1_500_20corpos_1000frames_5medicoes.png)

## Dependencies of C implementation
Seen that this is a C program that uses OpenGL you'll need to install the basic
tools to build the program and the library glut.

For debian based distros is simple to install what we need:
```sh
$ sudo apt-get install build-essential freeglut3-dev  
```     

## Build

Then to build from the source is just:
```sh
$ make
```

## Running
After build the binary just run it:

```sh
$ ./nbody.bin
```

Some option can be passed by command line attributes, they are:
- `-f N` Close the simulation after N frames
- `-n N` Use N bodies in the simulation
- `-b 0|1` Run in benchmark mode

All the position from each body in each frame will be stored in a file called
positionData.csv for further studies.

The default configuration of the bodies is a random distribution inside a cubic
region.

You can navigate in the simulation using the arrow keys, to speed up the process
you can pause using the "r" key, witch will toggle the rendering on and off.
To pause the simulation for a more smooth navigation you can use the "p" key.

In the end of running with limited number of frames the program print the
duration of the simulation.

## Profiling

The actual profile configuration is a little user-hostile but I tried to keep
the simplest possible. Frist to profile the program you must compile it.

The profiler need ```python-numpy```, ```python-matplotlib``` and
```python-scipy``` packages. To install them in a debian based distro just run:

```sh
$ sudo apt-get install python-numpy python-matplotlib python-scipy
```

Them inside the profile folder run:
```sh
$ python profiler.py
```

The profiler will run the simulation with many attributes and plot the results
with an aproximated function to describe the data.

Seen the source the profiler is created by: 

```python
profiler = Profiler([<start-bodies-quantity>,<end-bodies-quantity>,
                      <bodies-quantity-steps],<frames-per-simulation>,
                      <exections-with-same-attrs>)
```

Then to execute and generate the data and plot the grahics:
```python
NbodyProfiler.run()
NbodyProfiler.analise()
NbodyProfiler.plot()
```




**Happy Hack :)**
