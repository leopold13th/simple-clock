# Desktop Clock Application

A desktop clock application for Linux built with X11 and Cairo graphics library.

## Dependencies

- X11 development libraries
- Cairo graphics library
- GCC compiler

### Install Dependencies

**Ubuntu/Debian:**
```bash
sudo apt install libcairo2-dev libx11-dev pkg-config
```

## Building

```bash
make
```

## Running
```bash
./clock
```

## Clean

```bash
make clean
```

## Usage

The window is centered on screen and updates every second.
