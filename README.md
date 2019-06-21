# hullowheel-server
Server part of HulloWheel, a virtual gaming wheel for Android + Linux

## Why?
I'm a lover of Euro Truck Simulator 2. I love overspeeding in it. Problems are:

1. I play on Linux (Ubuntu + Steam OS)
2. I don't have a physical joystick / game wheel
3. There were no virtual "wheel + gas pedal + brake" controller for Linux, although there is a [great gamepad](https://github.com/rmst/yoke) for both Linux and Windows written by [rmst](https://github.com/rmst). But we drive trucks man, we want no yokes. Gas pedal and brake man!
4. Making my own game controller is cool.

## What then?
I had the following inputs:

1. An Android phone
2. Inspiration from [this repository](https://github.com/zvxryb/Linux-Virtual-Joystick) by [zvxryb](https://github.com/zvxryb)
3. ETS2 (that was how I tested my wheel)

and the following output:
1. HulloWheel, a Android client + Linux server combination. See [Hacking](https://github.com/hulloanson/hullowheel-server#Hacking) for details

## Installing
There is a pre-built program available in the [release](https://github.com/hulloanson/hullowheel-server/releases) page. It is only for 64-bit Linux though. If your system is not, you'll have to [build the program yourself](https://github.com/hulloanson/hullowheel-server#building).

Once downloaded, there are no installation step. Just run it.

## Running

```bash
./HulloWheel-x86_64 # Or only ./HulloWheel if you built it yourself
```

Also see the available options by:

```bash
./HulloWheel-x86_64 --help
```

## Building
### Dependencies
A list of libraries required:
```
# tools
cmake
make
gengetopt

# libraries + headers
zlib
pthread
```

On Ubuntu / Debian:
```bash
# With root privilege
apt-get install cmake make zlib1g zlib1g-dev libpthread-stubs0-dev gengetopt
```

I don't know other systems. Please make a PR to this README if you know. Thanks.

### Build steps
```bash
# Install dependencies first
https://github.com/hulloanson/hullowheel-server.git
cd hullowheel-server
./build.sh
```
A binary named `HulloWheel` will appear at the root directory. Run it. Try it out with https://github.com/hulloanson/hullowheel-android

### Run
```bash
./VWheel
# Press Ctrl-C / send SIGINT to stop it.
```
## TODOs
### Known issues
1. [ ] Now server normalizes client values. It shouldn't happen. Should leave it up to the client.

2. [ ] Performance. Now it lags behind a bit. 
  
    Possible reasons:
    - Now each frame averages 29 bytes gzipped, which contains the state of all 24 buttons and 3 axes (wheel, gas, brake). Perhaps send only the modified inputs?
  
3. [x] The only way to configure which port to listen to is to modify the code. Add argument support.
4. [x] Have to restart server and android app after disconnects. Probably due to UDP intricacies I'm not familiar with.
    - Solved. Now the server auto-restarts on timeout

### Hope-to-have Features
1. [ ] Runs on Windows
2. [ ] GUI

## Try it with other games!
Now works with:
  - Euro Truck Simulator 2
  - `jstest`
  
Please add to this list if you find HulloWheel works with a game / software. Thanks.

## Hacking
### Overview of this server code
I tried to keep it clean and clever, but I so rarely touch `C`. Roast me all you want.

3 parts:

#### Main
Entrypoint. It: 
  - sets up a wheel called "HulloWheel" (uinput blablabla.); then
  - starts a UDP server on port 20000 on a separate thread; and then
  - listens for interrupts and tear down the wheel and server
  
#### Wheel
Everything about the wheel. Creating the wheel, registering on `uinput`, writing event input, etc. Exposes function `emit` that `Server` uses

#### Server
The listening socket part. The data decompressing part. The data parsing part. Also "emits" input events. 

Also layout of a "frame":

```
0                  4                  8                  12                                                     36
|------------------|------------------|------------------|------------------------------------------------------|
|  wheel (float)   |  gas (float)     |  brake (float)   | buttons (byte) * 24                                  |
|------------------|------------------|------------------|------------------------------------------------------|
```

*Note: all floats are 32-bit precision float, little endian*

#### Misc
##### Logging 
The HulloWheel server uses macrologger (courtesy to https://github.com/dmcrodrigues/macro-logger) for logging. Kinda simple I know, but I don't want a full-blown library with additional configurations files / (de-)initalizing lines. Educate me if there was more well-tested and simple ones.

### Contributing
First a big thank you for your efforts. Please always make PRs on the `dev` branch.
