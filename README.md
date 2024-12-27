# XNB Reader for SpeedRunners

This is a simple XNB reader for the game SpeedRunners. It can be used to read and write XNB files (Texture2D, SoundEffect, and TextureAtlas) from the game. It also support reading LZX compressed XNB files without xcompress32.dll (but not LZ4 compressed, sorry).

If you want output XNBs to be compressed, you need to install xcompress32.dll-- which is scary to include in this repository, but I found it [here](https://github.com/cpich3g/rpftool/blob/master/RPFTool/xcompress32.dll?raw=true).

## Usage

`xnb [-u] <input> <output>`
- `-u` is an optional flag to write the output XNB file **uncompressed** (default is compressed)

## Examples

`xnb -u image.png image.xnb` - Converts `image.png` to `image.xnb` (uncompressed)
`xnb image.xnb image.png` - Converts `image.xnb` to `image.png`

`xnb atlas.json atlas.xnb`
`xnb atlas.xnb atlas.json`

`xnb sound.wav sound.xnb`
`xnb sound.xnb sound.wav`

## Building

This was built with mingw (32-bit), but it should work with any 32-bit compiler. Compiling 64-bit is also possible, but xcompress32.dll is 32-bit, so you'd lose compressiong support, and also probably get a crash when loading xompress32.dll.

The CMakeLists.txt is included, but includes mingw-specific flags which you can comment out freely.

## Contributing

I probably won't be checking pull requests, but feel free to fork, modify, distribute, etc. on your own. This is a simple tool that I made for the community's use, and I'm sharing it in case it's useful to others.

If you want to add support for a custom XNB format, you just need to modify `xnb.h` and `xnb.cpp`. Right now the front-end just needs to run

```cpp
...
xnb::texture2D texture;
xnb::loadFromXnb(inputFile, texture); //or load from png
xnb::saveToXnb(outputFile, texture, boolCompress); //or save to png
```

so if you add your own format, ideally make support for `xnb::Type`, `xnb::saveToXnb`, `xnb::saveToType`, `xnb::loadFromXnb`, and `xnb::loadFromType` functions.

I've made some helpers for the repetitive tasks through the "helpers" at the bottom of `xnb.cpp` and `xnb.h`, so you can use those to help you out.

## TODO (likely not going to be done by me)

- Add support for LZ4 compressed XNB files
- Add support for more XNB types (I'm pretty sure Stardew Valley uses some custom ones)
- Disambiguate .json files, so they can be used for more than just TextureAtlas
- Reduce unnecessary copying of data with `std::stringstream`.
