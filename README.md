# xnb reader for c++

This is a simple xnb reader written using c++. It can read LZX compressed xnb files (currently no support for LZ4 compression or writing xnb files).

The LZX decompression code is heavily based on the code from the [MonoGame](https://github.com/MonoGame/MonoGame) project. See the disclaimer in the `LzxDecoder` class for more information.

## Usage

Currently, only `Texture2D` and `SoundEffect` have supported readers. I will also be making a `TextureAtlas` reader for SpeedRunners soon.

```cpp
#include <xnb/readers.h>
//you can also include your own readers here
#include <xnb/xnb.h>

int main()
{
    std::string path = "path/to/your/file.xnb";
    xnb::xnb xnb;
    xnb::load(&xnb, path.c_str());

	xnb::readerManager readers;
	xnb::registerDefaultReaders(&readers);
    //xnb::registerReader(&readers, "MyType", myReaderFunction);

	xnb::texture2D texture;
	xnb::read(&xnb, &readers, &texture);
}
```

## Making your own reader

To make your own reader, you should (ideally) make a struct that contains the data of the type, then make a function `void myReader(xnb::xnb*, void* target)` that reads the data from the xnb file and stores it in the struct at `target`. `xnb->data` will contain all the object data-- I've also made a helper class `bufferReader` that can help with reading in data and does not make any redundant copies of the data.

!! ***Keep in mind*** !! the start of the data usually will contain a 7BitEncodedInt that represents the index of the type reader!

If you make a reader, using it is as easy as registering it with the `readerManager` and calling `xnb::read` with the correct type.

If your reader is a default reader, you can register it in the `registerDefaultReaders` function, and submit a pull request. A list of all the default readers can be found in the format guide (originally found [here](https://github.com/SimonDarksideJ/XNAGameStudio/wiki/Compiled-(XNB)-Content-Format)).

```cpp
#include <xnb/bufferReader.h>
#include <xnb/xnb.h>

struct MyType
{
    int myInt;
    float myFloat;
};

void myReader(xnb::xnb* xnb, void* target)
{
    MyType* myType = (MyType*)target;
    xnb::bufferReader reader(xnb->data, xnb->dataSize);

    //possibly needs to skip the type reader index (which despite being 7BitEncodedInt, is usually 1 byte unless you have hundreds of readers)
    //reader.seek(1, origin::current);

    reader->read(&myType->myInt);
    reader->read(&myType->myFloat);
}
```
```cpp
#include <myTypeReader.h>

int main()
{
    xnb::xnb xnb;
    xnb::load(&xnb, "path/to/your/file.xnb");

    xnb::readerManager readers;
    xnb::registerDefaultReaders(&readers);
    xnb::registerReader(&readers, "MyType", myReader);

    MyType myType;
    xnb::read(&xnb, &readers, &myType);
}
```

## Building

This project uses CMake and was originally built with MinGW. It only uses the standard library, so it should be easy to build on any platform by just:

```
mkdir build
cd build
cmake ..
```

then building with your preferred build system.
