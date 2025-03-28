# QOI - The 'Quite OK Image' Format
This is a simple implementation of QOI, which is a lossless image compression algorithm that compresses RGB and RGBA images to a similar size of PNG, while offering a 20x-50x speedup in compression and 3x-4x speedup in decompression.

## Running Tests
To check the compression ratio i.e. space saved by qoi, first compile a shared library (DLL) using the C files, and run test.py.

```
gcc -shared -o qoi.dll main.c qoi.c
python3 test.py
```


## Resources
- [Paper](https://qoiformat.org/qoi-specification.pdf)
- [Original Blog](https://phoboslab.org/log/2021/11/qoi-fast-lossless-image-compression)
- [Github implementation](https://github.com/phoboslab/qoi/tree/master)