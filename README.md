# hullowheel-server
Server part of the HulloWheel, a virtual gaming wheel on Linux.

## Why?
I'm a lover of Euro Truck Simulator. I love overspeeding in it. Problems are:

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
HulloWheel, a Android client + Linux server combination. See [Hacking](https://github.com/hulloanson/hullowheel-server#Hacking) for details

## How to install?
This is super-beta. You will need to build it yourself for now. No binaries I'm sorry.
### Dependencies
A list of libraries required:
```
// tools
cmake
make

// libraries + headers
zlib
pthread
```

On Ubuntu / Debian:
```bash
// With root privilege
apt-get install cmake make zlib1g zlib1g-dev libpthread-stubs0-dev
```

I don't know other platforms. Please make a PR to this README if you know. Thanks.

### Build steps
```
// Install dependencies first
https://github.com/hulloanson/hullowheel-server.git
cd hullowheel-server
cmake . && make
```
A binary named `VWheel` will appear at the root directory. Run it. Try it out with https://github.com/hulloanson/hullowheel-android

## Known issues & TODOs
1. Now server normalizes client values. It shouldn't happen. Should leave it up to the client.
2. Haven't tested it with other games. Tested it with `jstest` though. Was fine.
3. Performance optimization. Now it lags behind a bit. 
  
    Possible reasons:
    - Now each frame averages 29 bytes gzipped, which contains the state of all 24 buttons and 3 axes (wheel, gas, brake). Perhaps send only the modified inputs?
  
4. The only way to configure which port to listen to is to modify the code. Add argument support.
5. There are also integration issues (nothing major, just inconveniences like restarting the server) with [hullowheel-anrdroid](https://github.com/hulloanson/hullowheel-android)

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
Everything about the wheel. Cerating the wheel, registering on `uinput`, writing event input, etc. Exposes function `emit` that `Server` uses

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

### Contributing
First a big thank you for your efforts. Please always make a PR on the `dev` branch.
