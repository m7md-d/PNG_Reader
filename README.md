# PNG_Reader

A simple C program that reads a PNG file and displays metadata from the IHDR chunk, including image dimensions, bit depth, and color type.

## Building

```bash
gcc -o png png_reader.c
```

## Usage

```bash
./png <file.png>
```

## Output Example

```
Width:            800
Height:           600
Bit depth:        8
Color type:       6 (RGBA)
```
