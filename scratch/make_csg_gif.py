# SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
#
# SPDX-License-Identifier: GPL-3.0-or-later

"""Caption the frames csganim wrote and join them into a looping GIF with ffmpeg.

    python3 make_csg_gif.py <frame folder> <out.gif> [frames per second]
"""

import os
import subprocess
import sys

from PIL import Image, ImageDraw, ImageFont

folder = sys.argv[1]
out = sys.argv[2]
fps = sys.argv[3] if len(sys.argv) > 3 else "20"

font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf", 20)

with open(os.path.join(folder, "captions.txt")) as f:
    captions = dict(line.rstrip("\n").split("\t", 1) for line in f)

for index, caption in captions.items():
    image = Image.open(os.path.join(folder, "frame_%04d.ppm" % int(index))).convert("RGBA")
    layer = Image.new("RGBA", image.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(layer)
    left, top, right, bottom = draw.textbbox((0, 0), caption, font=font)
    draw.rounded_rectangle((12, 12, 12 + right + 20, 12 + bottom + 16), radius=6, fill=(20, 24, 32, 190))
    draw.text((22, 20), caption, font=font, fill=(255, 255, 255, 255))
    Image.alpha_composite(image, layer).convert("RGB").save(os.path.join(folder, "cap_%04d.png" % int(index)))

# One palette for the whole animation, so colours do not flicker between frames.
subprocess.run(
    [
        "ffmpeg", "-y", "-loglevel", "error", "-framerate", fps,
        "-i", os.path.join(folder, "cap_%04d.png"),
        "-vf", "split[a][b];[a]palettegen=max_colors=256[p];[b][p]paletteuse=dither=sierra2_4a",
        "-loop", "0", out,
    ],
    check=True,
)

print("wrote %s, %.1f MB" % (out, os.path.getsize(out) / 1e6))
