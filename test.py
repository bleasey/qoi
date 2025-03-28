import os
import ctypes

def main():
    lib = ctypes.CDLL("./qoi.dll")
    lib.run_compression.argtypes = [ctypes.c_char_p]

    folderpath = "./images"
    for filename in os.listdir(folderpath):
        print("Image: ", filename)
        binaryPath = os.path.join(folderpath, filename).encode()
        lib.run_compression(binaryPath)

if __name__ == '__main__':
    main()
