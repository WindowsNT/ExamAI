import torch
import torch.nn as nn
from torchvision import transforms
from torchvision.datasets import ImageFolder
from torch.utils.data import DataLoader, Subset
import sys

if len(sys.argv) < 3:
    print("Usage: python evaluate.py <dataset_folder> <model.pth>")
    sys.exit(1)

dataset_path = sys.argv[1]
model_path = sys.argv[2]

# === Match the same transformation as during training ===
transform = transforms.Compose([
    transforms.Grayscale(),
    transforms.Resize((64, 64)),
    transforms.ToTensor()
])

# Load dataset and split last 20% for evaluation
full_dataset = ImageFolder(root=dataset_path, transform=transform)
val_size = len(full_dataset) // 5
val_indices = list(range(len(full_dataset) - val_size, len(full_dataset)))
val_dataset = Subset(full_dataset, val_indices)
val_loader = DataLoader(val_dataset, batch_size=32, shuffle=False)

# === Model must match structure used during training ===
net = nn.Sequential(
    nn.Conv2d(1, 8, 3, padding=1),   # net.0
    nn.ReLU(),                       # net.1
    nn.MaxPool2d(2, 2),              # net.2

    nn.Conv2d(8, 64, 3, padding=1),  # net.3
    nn.ReLU(),                       # net.4
    nn.MaxPool2d(2, 2),              # net.5

    nn.Conv2d(64, 4, 1),             # net.6
    nn.AdaptiveAvgPool2d((1, 1)),    # net.7
    nn.Flatten()                     # net.8
)

device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
net.to(device)

# === Load weights with matching keys ===
state_dict = torch.load(model_path, map_location=device)
net.load_state_dict(state_dict)
net.eval()

# === Evaluation ===
correct = 0
total = 0
class_correct = [0] * 4
class_total = [0] * 4

with torch.no_grad():
    for images, labels in val_loader:
        images, labels = images.to(device), labels.to(device)
        outputs = net(images)
        _, predicted = torch.max(outputs, 1)

        for i in range(len(labels)):
            label = labels[i].item()
            class_total[label] += 1
            if predicted[i] == label:
                class_correct[label] += 1
                correct += 1
            total += 1

# === Print report ===
print(f"\n✅ Overall Accuracy: {100.0 * correct / total:.2f}%")

for i, (c, t) in enumerate(zip(class_correct, class_total)):
    acc = 100.0 * c / t if t else 0
    print(f"Class {i}: {acc:.2f}% ({c}/{t})")
