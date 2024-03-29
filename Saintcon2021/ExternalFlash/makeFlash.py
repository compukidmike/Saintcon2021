#! /usr/bin/env python3

from PIL import Image
import struct, os, sys

##
if __name__ == '__main__':
    with open('flash.bin', 'wb') as outf:
        for filename in ['sclogo.png', 'machine_bkg.png', 'machine.png', 'build.png', 'parts.png', 'menu.png', 'crate.png', 
            'nfc.png', 'ball.png', 'inventory.png', 'ship.png', 'explode.png', 'trade.png', 'trade2.png', 'whammy.png', 'rick.png']:
            if os.path.exists(filename) == False: 
                error('not exists: ' + filename)
            body, _ = os.path.splitext(filename)

            img = Image.open(filename).convert('RGB')
            pixels = list(img.getdata())
            count =0

            print("#define %-20s \t ((uint32_t)0X%06X)"%(body.upper()+"_IMG", outf.tell()))

            for pix in pixels:
                r = (pix[0] >> 3) & 0x1F
                g = (pix[1] >> 2) & 0x3F
                b = (pix[2] >> 3) & 0x1F
                outf.write(struct.pack('H', (r << 11) + (g << 5) + b))
                count += 2

            while (outf.tell() %1024):
                outf.write(b'\xFF')

        outf.write(b'\0'*118)
        outf.write(b'Lock Combo: 38-8-18 ')
        outf.write(b'\0'*118)
        print()
        print("%dKB used"%(outf.tell()/1024))
