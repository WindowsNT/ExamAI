import cv2
import numpy as np
import os
import random
from pathlib import Path
from sys import argv

output_dir = argv[1] if len(argv) > 1 else "dataset"
img_size = 64
samples_per_class = 15000
classes = ["0_empty", "1_tick", "2_cross", "3_dot"]

def create_blank_box():
    img = np.ones((img_size, img_size), dtype=np.uint8) * 255
    thickness = random.randint(1, 2)
    cv2.rectangle(img, (5, 5), (img_size - 5, img_size - 5), (0,), thickness)
    return img

def draw_mark(img, label):
    if label == "0_empty":
        return img

    center_x = img_size // 2 + random.randint(-4, 4)
    center_y = img_size // 2 + random.randint(-4, 4)
    thickness = random.randint(2, 4)
    color = random.randint(0, 50)

    if label == "1_tick":
        start = (center_x - 10, center_y)
        mid = (center_x - 2, center_y + 10)
        end = (center_x + 12, center_y - 10)
        cv2.line(img, start, mid, color, thickness)
        cv2.line(img, mid, end, color, thickness)

    elif label == "2_cross":
        offset = 10
        cv2.line(img, (center_x - offset, center_y - offset), (center_x + offset, center_y + offset), color, thickness)
        cv2.line(img, (center_x + offset, center_y - offset), (center_x - offset, center_y + offset), color, thickness)

    elif label == "3_dot":
        radius = random.randint(2, 4)
        cv2.circle(img, (center_x, center_y), radius, color, -1)

    return img

def apply_augmentations(img):
    angle = random.randint(-25, 25)
    M = cv2.getRotationMatrix2D((img_size // 2, img_size // 2), angle, 1)
    img = cv2.warpAffine(img, M, (img_size, img_size), borderValue=255)

    if random.random() > 0.7:
        ksize = random.choice([3, 5])
        img = cv2.GaussianBlur(img, (ksize, ksize), 0)

    noise = np.random.randint(0, 10, (img_size, img_size), dtype='uint8')
    img = cv2.add(img, noise)

    # Removed handwriting/embedded text simulation here

    return img

for label in classes:
    path = Path(output_dir) / label
    path.mkdir(parents=True, exist_ok=True)
    for i in range(samples_per_class):
        img = create_blank_box()
        img = draw_mark(img, label)
        img = apply_augmentations(img)
        cv2.imwrite(str(path / f"{label}_{i:05d}.png"), img)
