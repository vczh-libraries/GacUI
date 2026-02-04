# Using Streams

Streams in this library are composable.

Let's say we need to read a file, it has a BOM at the beginning of the file to indicate the text encoding. Now we need to print the file line by line to the screen, and save the file in UTF-8 and UTF-16 without BOM, in two separated files.

Here is what we need:
- **FileStream**for accessing files.
- **BroadcastStream**for copy content to multiple streams.
- **RecorderStream**for reading data from a stream, and save what is read to another stream.
- **EncoderStream**and**DecoderStream**for handling text encoding.We compose these streams in this way:![](https://gaclib.net/doc/vlppos/using-streams.png)

When we read text from the**StreamReader**, the**StreamReader**reads data from the**RecorderStream**, which reads data from the**DecoderStream**.

At the same time, the**RecorderStream**write all bytes that is read to the**BroadcastStream**, which writes data to two**EncoderStream**.

**DecoderStream**uses the assigned decoder to convert raw bytes from the**FileStream**to**wchar_t**.**EncoderStream**uses the assigned encoder to convert**wchar_t**to raw bytes and write to the**FileStream**.

By composing all streams together, what we need to do is just read text line by line from the**StreamReader**, and everything else will be done by all these connected streams.


```C++
#include <vlppos.h>

using namespace vl::console;
using namespace vl::stream;

int main()
{
    FileStream inputFileStream(L"C:/bom.txt", FileStream::ReadOnly);
    BomDecoder bomDecoder;
    DecoderStream inputDecoderStream(inputFileStream, bomDecoder);

    FileStream utf8FileStream(L"C:/utf8.txt", FileStream::WriteOnly);
    Utf8Encoder utf8Encoder;
    EncoderStream utf8EncoderStream(utf8FileStream, utf8Encoder);

    FileStream utf16FileStream(L"C:/utf16.txt", FileStream::WriteOnly);
    Utf16Encoder utf16Encoder;
    EncoderStream utf16EncoderStream(utf16FileStream, utf16Encoder);

    BroadcastStream broadcastStream;
    broadcastStream.Targets().Add(&utf8EncoderStream);
    broadcastStream.Targets().Add(&utf16EncoderStream);

    RecorderStream recorderStream(inputDecoderStream, broadcastStream);

    StreamReader reader(recorderStream);
    while (!reader.IsEnd())
    {
        Console::WriteLine(reader.ReadLine());
    }
}
```


