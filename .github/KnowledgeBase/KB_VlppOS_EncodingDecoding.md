# Encoding and Decoding

Text encoding conversion between different UTF formats with BOM support and binary data encoding.

## UTF Encoding with BOM Support

Use `BomEncoder` and `BomDecoder` for UTF encoding with BOM support.

`BomEncoder` and `BomDecoder` convert data between `wchar_t` and a specified UTF encoding with BOM added to the very beginning.

## UTF Encoding without BOM

Use `UtfGeneralEncoder<Native, Expect>` and `UtfGeneralDecoder<Native, Expect>` for UTF conversion without BOM.

`UtfGeneralEncoder<Native, Expect>` encode from `Expect` to `Native`, `UtfGeneralDecoder<Native, Expect>` decode from `Native` to `Expect`. They should be one of `wchar_t`, `char8_t`, `char16_t`, `char32_t` and `char16be_t`.

Unlike `BomEncoder` and `BomDecoder`, `UtfGeneralEncoder` and `UtfGeneralDecodes` is without BOM.

`char16be_t` means UTF-16 Big Endian, which is not a C++ native type, it can't be used with any string literal.

## Specific UTF Conversion Aliases

There are aliases for them to convert between `wchar_t` and any other UTF encoding:

- Use `Utf8Encoder` and `Utf8Decoder` for UTF-8 conversion
- Use `Utf16Encoder` and `Utf16Decoder` for UTF-16 conversion  
- Use `Utf16BEEncoder` and `Utf16BEDecoder` for UTF-16 Big Endian conversion
- Use `Utf32Encoder` and `Utf32Decoder` for UTF-32 conversion

## ASCII/MBCS Encoding

Use `MbcsEncoder` and `MbcsDecoder` for ASCII/MBCS conversion.

`MbcsEncoder` and `MbcsDecoder` convert data between `wchar_t` and `char`, which is ASCII.

`BomEncoder::Mbcs` also handles ASCII meanwhile there is no BOM for ASCII. A `BomEncoder(BomEncoder::Mbcs)` works like a `MbcsEncoder`.

The actual encoding of `char` depends on the user setting in the running OS.

## Automatic Encoding Detection

Use `TestEncoding` for automatic encoding detection.

There is a function `TestEncoding` to scan a binary data and guess the most possible UTF encoding.

## Base64 Encoding

Use `Utf8Base64Encoder` and `Utf8Base64Decoder` for Base64 encoding in UTF-8.

`Utf8Base64Encoder` and `Utf6Base64Decoder` convert between binary data to Base64 in UTF8 encoding.
They can work with `UtfGeneralEncoder` and `UtfGeneralDecoder` to convert binary data to Base64 in a `WString`.

### Example: Converting Binary Data to Base64 WString

```cpp
MemoryStream memoryStream;
{
  UtfGeneralEncoder<wchar_t, char8_t> u8towEncoder;
  EncoderStream u8towStream(memoryStream, u8towEncoder);
  Utf8Base64Encoder base64Encoder;
  EncoderStream base64Stream(u8t0wStream, base64Encoder);
  base64Stream.Write(binary ...);
}
memoryStream.SeekFromBegin(0);
{
  StreamReader reader(memoryStream);
  auto base64 = reader.ReadToEnd(reader);
}
```

### Example: Converting Base64 WString to Binary Data

```cpp
MemoryStream memoryStreamn;
{
  StreamWriter writer(memoryStream);
  writer.WriteString(base64);
}
memoryStream.SeekFromBegin(0);
{
  UtfGeneralEncoder<wchar_t, char8_t> wtou8Decoder;
  DecoderStream wtou8Stream(memoryStream, wtou8Decoder);
  Utf8Base64Decoder base64Decoder;
  DecoderStream base64Stream(wtou8Stream, base64Decoder);
  base64Stream.Read(binary ...);
}
```

## Data Compression

Use `LzwEncoder` and `LzwDecoder` for data compression.

- `LzwEncoder` compress binary data.
- `LzwDecoder` decompress binary data.

## Helper Functions

Use `CopyStream`, `CompressStream`, `DecompressStream` helper functions.

There are help functions `CopyStream`, `CompressStream` and `DecompressStream` to make the code simpler.

## Extra Content

### Encoding Selection Guidelines

When choosing between different encoding methods:
- Use BOM encoders when you need to ensure proper encoding detection by other applications
- Use general UTF encoders for maximum compatibility and control over BOM presence
- Use MBCS encoders only when working with legacy systems that require ASCII compatibility

### Performance Considerations

Different encoding operations have varying performance characteristics:
- ASCII/MBCS encoding is fastest but limited to basic character sets
- UTF-8 encoding provides good balance between space efficiency and Unicode support
- UTF-16 and UTF-32 provide different trade-offs between processing speed and memory usage

### Error Handling

Encoding operations may fail when:
- Invalid byte sequences are encountered during decoding
- Characters cannot be represented in the target encoding
- BOM detection fails for corrupted files

Always handle potential encoding exceptions, especially when processing user-provided files.

### Pipeline Design

The encoder/decoder system is designed for pipeline composition. You can chain multiple encoding operations together to create complex data transformation workflows, such as:
1. Base64 decode → UTF-8 decode → String processing
2. String processing → UTF-8 encode → Compression → File output

This design provides flexibility for handling various data transformation scenarios efficiently.