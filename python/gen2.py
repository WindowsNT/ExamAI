import cv2
import numpy as np
import os
import random
from pathlib import Path
from sys import argv

# === Settings ===
output_dir = argv[1] if len(argv) > 1 else "dataset"
img_size = 64
samples_per_class = 1500  # 50% normal, 50% sparse
classes = ["0_empty", "1_tick", "2_cross", "3_dot"]

def create_blank_box():
    img = np.ones((img_size, img_size), dtype=np.uint8) * 255
    thickness = random.randint(1, 2)
    cv2.rectangle(img, (5, 5), (img_size - 5, img_size - 5), (0,), thickness)
    return img

def draw_mark(img, label, sparse=False):
    if label == "0_empty":
        return img

    jitter = 5 if sparse else 3
    offset_x = random.randint(-jitter, jitter)
    offset_y = random.randint(-jitter, jitter)
    cx = img_size // 2 + offset_x
    cy = img_size // 2 + offset_y

    thickness = random.choice([1, 2]) if sparse else random.randint(2, 4)
    color = random.randint(100, 180) if sparse else random.randint(0, 50)

    if label == "1_tick":
        start = (cx - 10, cy)
        mid = (cx - 2, cy + 10)
        end = (cx + 12, cy - 10)
        cv2.line(img, start, mid, color, thickness)
        cv2.line(img, mid, end, color, thickness)

    elif label == "2_cross":
        off = 10
        cv2.line(img, (cx - off, cy - off), (cx + off, cy + off), color, thickness)
        cv2.line(img, (cx + off, cy - off), (cx - off, cy + off), color, thickness)

    elif label == "3_dot":
        radius = random.randint(2, 4)
        cv2.circle(img, (cx, cy), radius, color, -1)

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

    return img

# === Generation Loop ===
for label in classes:
    path = Path(output_dir) / label
    path.mkdir(parents=True, exist_ok=True)

    for i in range(samples_per_class):
        sparse = i >= samples_per_class // 2  # Half sparse, half normal
        img = create_blank_box()
        img = draw_mark(img, label, sparse)
        img = apply_augmentations(img)
        fname = f"{label}_{'s' if sparse else 'n'}_{i:04d}.png"
        cv2.imwrite(str(path / fname), img)
