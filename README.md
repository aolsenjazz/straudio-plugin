# DEPRECATED
There are now cheap or free options on the market which solve the problems Straudio used to solve, so this repository is now archived.

## Straudio (plugin)

An audio plugin built with [iPlug2](https://github.com/iPlug2/iPlug2) which sends audio from DAW to browser via [WebRTC](https://webrtc.org/) Datachannel. The web client for use with the Straudio plugin can be found [here](https://github.com/aolsenjazz/straudio-web).

### Features

- Ultra-low latency
- Resampling using [LibSampleRate](http://www.mega-nerd.com/SRC/)
- Supports all major DAWs

## Run Locally

### Clone

```
git clone https://github.com/aolsenjazz/straudio-plugin
```

### Install Dependencies

All dependencies must be static libraries as plugin hosts do not like dynamic libraries. Dependencies should be placed in the Straudio/libs folder.

- [LibCrypto](https://wiki.openssl.org/index.php/Libcrypto_API)
- [LibDataChannel](https://github.com/paullouisageneau/libdatachannel)
- [IXWebSocket](https://github.com/machinezone/IXWebSocket)
- [Juice](https://github.com/paullouisageneau/libjuice)
- [Nettle](https://www.lysator.liu.se/~nisse/nettle/)
- [LibSampleRate](http://www.mega-nerd.com/SRC/)
- LibSrtp2
- [LibSSL](https://wiki.openssl.org/index.php/Libssl_API)
- [Libusrsctp](https://github.com/sctplab/usrsctp)

### Install iPlug2

iPlug2 (and its dependencies) must be cloned and installed in the correct location. Project structure should be:

```
/workspace
    /straudio-plugin
    /iPlug2
```
