#!/usr/bin/env python3

import sys
import os.path
import zlib
import subprocess
from PIL import Image


colortable_path = '/tmp/colortable.gif'

px_width = 0
px_height = 0

def create_color_table():
    subprocess.call(['convert', '-size', '1x16', 'gradient:gray100-gray0', colortable_path])

def get_image_height(image_path):
    im = Image.open(image_path)
    _, heigth = im.size
    return heigth

def scale_and_convert_image(image_path):
    converted_image_path_raw   = os.path.splitext(image_path)[0]
    converted_image_path       = '/tmp/current.gif'

    print("resizing, converting color ...")
    target_size = str(px_width) + 'x' + str(px_height)
    subprocess.call(['convert', image_path,
                     # '-rotate', '-90',
                     '-resize', target_size + '^',
                     '-gravity', 'center',
                     '-extent', target_size,
                     '-quantize', 'gray',
                     # '-brightness-contrast', '0x00',
                     '-dither', 'Floyd-Steinberg', # see 'convert -list' for other options
                     '-remap', colortable_path,
                     # '-contrast-stretch', '20%', # macht die 16 colors kaputt
                     converted_image_path])
    print("converting to raw image ...")
    make_raw(converted_image_path, converted_image_path_raw)

def make_raw(image_path, destination_path):
    im = Image.open(image_path)
    width, height = im.size
    start_height = 0
    if height > px_height:
        start_height = (height - px_height)//2
        height = start_height + px_height

    px = im.load()

    f = open(destination_path, 'wb')

    val = 0
    i = 0
    for y in range(start_height, height):
        for x in range(0, width):
            i = not i
            # little-endian-like
            if i:
                val = px[x,y]
            else:
                val += px[x,y] << 4
                f.write(bytearray([val]))

if __name__ == "__main__":
    if len(sys.argv) < 4:
        print(f'Usage: {sys.argv[0]} [width] [height] [images]')
        exit(-1)

    px_width = int(sys.argv[1])
    px_height = int(sys.argv[2])

    create_color_table()
    for image_path in sys.argv[3:]:
        scale_and_convert_image(image_path)
