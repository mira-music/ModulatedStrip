#!/usr/bin/env python3
"""
Generate plugin icon for Modulated Strip VST
Creates PNG files at multiple sizes
Requires: pip install Pillow
"""

from PIL import Image, ImageDraw, ImageFont
import os

def create_icon(size):
    # Dark background matching plugin aesthetic
    img = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    # Outer rounded rectangle - dark chassis
    margin = size // 16
    draw.rounded_rectangle(
        [margin, margin, size-margin, size-margin],
        radius=size//8,
        fill=(14, 14, 14, 255),
        outline=(42, 42, 42, 255),
        width=max(1, size//64)
    )

    # Amber accent bar across top
    bar_h = max(2, size // 16)
    draw.rectangle(
        [margin*2, margin*2,
         size-margin*2, margin*2 + bar_h],
        fill=(232, 168, 56, 255)
    )

    # VU meter arc simulation in center
    cx = size // 2
    cy = size // 2 + size // 10

    # Three vertical bars representing VU segments
    bar_w = max(2, size // 20)
    bar_gap = max(1, size // 30)
    bar_heights = [size//3, size//2, size//3]
    bar_colors = [
        (58, 138, 58, 200),   # green
        (232, 168, 56, 220),  # amber
        (200, 48, 32, 200),   # red
    ]

    total_w = len(bar_heights) * (bar_w + bar_gap) - bar_gap
    start_x = cx - total_w // 2

    for i, (bh, bc) in enumerate(zip(bar_heights, bar_colors)):
        bx = start_x + i * (bar_w + bar_gap)
        by = cy - bh // 2
        draw.rounded_rectangle(
            [bx, by, bx + bar_w, by + bh],
            radius=max(1, bar_w // 3),
            fill=bc
        )

    # Needle line
    needle_len = size // 4
    draw.line(
        [cx, cy, cx + needle_len // 2, cy - needle_len],
        fill=(220, 200, 180, 200),
        width=max(1, size // 48)
    )

    # Small text "MS" at bottom if large enough
    if size >= 64:
        try:
            font_size = max(6, size // 10)
            font = ImageFont.truetype("arial.ttf", font_size)
        except:
            font = ImageFont.load_default()

        text = "MS"
        bbox = draw.textbbox((0, 0), text, font=font)
        tw = bbox[2] - bbox[0]
        th = bbox[3] - bbox[1]
        draw.text(
            (cx - tw//2, size - margin*3 - th),
            text,
            fill=(200, 168, 88, 200),
            font=font
        )

    return img

# Create output folder
os.makedirs("icons", exist_ok=True)

# Generate at multiple sizes
for size in [16, 32, 64, 128, 256, 512]:
    icon = create_icon(size)
    icon.save(f"icons/icon_{size}.png")
    print(f"Created icon_{size}.png")

# Create ICO file with multiple sizes
icons = []
for size in [16, 32, 48, 64, 128, 256]:
    icons.append(create_icon(size))

icons[0].save(
    "icons/ModulatedStrip.ico",
    format="ICO",
    sizes=[(s, s) for s in [16, 32, 48, 64, 128, 256]],
    append_images=icons[1:]
)
print("Created ModulatedStrip.ico")
print("Done - check the icons/ folder")