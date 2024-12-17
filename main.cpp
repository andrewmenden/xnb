#include <xnb/readers.h>
#include <xnb/xnb.h>
#include <cstdio>

int main()
{
	std::string path = "C:/Users/andre/dev/exampleImage.xnb";
	xnb::xnb xnb;
	xnb::load(&xnb, path.c_str());

	xnb::readerManager readers;
	xnb::registerDefaultReaders(&readers);

	xnb::texture2D texture;
	xnb::read(&xnb, &readers, &texture);
}
