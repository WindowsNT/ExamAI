import os
import torch
import torch.nn as nn
import torch.optim as optim
from torchvision import datasets, transforms
from torch.utils.data import DataLoader
import matplotlib.pyplot as plt
import numpy as np
from sys import argv
from torch.utils.data import random_split
from tqdm import tqdm


# ====== CONFIG ======
IMAGE_SIZE = 64
BATCH_SIZE = 32
EPOCHS = 7
LEARNING_RATE = 0.01
DEVICE = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
DATA_PATH = argv[1]
OUT_PATH = argv[2]

label_map = {
    "0_empty": 0,
    "1_tick": 1,
    "2_cross": 2,
    "3_dot": 3,
}

# ====== TRANSFORM ======
transform = transforms.Compose([
    transforms.Grayscale(),
    transforms.Resize((IMAGE_SIZE, IMAGE_SIZE)),
    transforms.ToTensor()
])

# ====== CUSTOM DATASET WITH FIXED LABEL MAP ======
class CustomImageFolder(datasets.ImageFolder):
    def find_classes(self, directory):
        classes = list(label_map.keys())
        class_to_idx = {cls: label_map[cls] for cls in classes}
        return classes, class_to_idx


# ====== DATASET SPLIT ======
full_dataset = CustomImageFolder(DATA_PATH, transform=transform)
total_len = len(full_dataset)
train_len = int(0.8 * total_len)
val_len = total_len - train_len

train_dataset, val_dataset = random_split(full_dataset, [train_len, val_len])
train_loader = DataLoader(train_dataset, batch_size=BATCH_SIZE, shuffle=True)


# ====== VISUALIZE A BATCH ======
def visualize_batch(data_loader):
    images, labels = next(iter(data_loader))
    fig, axes = plt.subplots(1, min(len(images), 8), figsize=(12, 3))
    for i, ax in enumerate(axes):
        ax.imshow(images[i][0], cmap='gray')
        ax.set_title(f"Label: {labels[i].item()}")
        ax.axis('off')
    plt.tight_layout()
    plt.show()

# visualize_batch(train_loader)

# ====== CNN DEFINITION ======

class Net(nn.Module):
    def __init__(self):
        super(Net, self).__init__()
        self.net = nn.Sequential(
            nn.Conv2d(1, 8, kernel_size=3, padding=1, bias=True),   # net.0
            nn.ReLU(),                                               # net.1
            nn.MaxPool2d(2),                                         # net.2
            nn.Conv2d(8, 64, kernel_size=3, padding=1, bias=True),  # net.3
            nn.ReLU(),                                               # net.4
            nn.MaxPool2d(2),                                         # net.5
            nn.Conv2d(64, 4, kernel_size=1, bias=True)              # net.6
        )

    def forward(self, x):
        x = self.net(x)
        return x.mean(dim=[2, 3])  # Global average pooling → [B, 4]


model = Net().to(DEVICE)
criterion = nn.CrossEntropyLoss()
optimizer = optim.Adam(model.parameters(), lr=LEARNING_RATE)

# ====== TRAIN LOOP ======
for epoch in range(EPOCHS):
    model.train()
    running_loss = 0.0
    correct = 0
    total = 0

    loop = tqdm(train_loader, desc=f"Epoch {epoch+1}/{EPOCHS}", leave=False)
    for imgs, labels in loop:
        imgs, labels = imgs.to(DEVICE), labels.to(DEVICE)

        optimizer.zero_grad()
        outputs = model(imgs)
        loss = criterion(outputs, labels)
        loss.backward()
        optimizer.step()

        running_loss += loss.item() * imgs.size(0)
        _, predicted = torch.max(outputs, 1)
        correct += (predicted == labels).sum().item()
        total += labels.size(0)

        loop.set_postfix(loss=loss.item())

    epoch_loss = running_loss / len(train_loader.dataset)
    accuracy = 100 * correct / total
    print(f"\nEpoch {epoch+1}/{EPOCHS} - Train Loss: {epoch_loss:.4f} - Train Accuracy: {accuracy:.2f}%")

    # Validation
    model.eval()
    val_correct = 0
    val_total = 0
    with torch.no_grad():
        for imgs, labels in DataLoader(val_dataset, batch_size=BATCH_SIZE):
            imgs, labels = imgs.to(DEVICE), labels.to(DEVICE)
            outputs = model(imgs)
            _, predicted = torch.max(outputs, 1)
            val_correct += (predicted == labels).sum().item()
            val_total += labels.size(0)

    val_accuracy = 100 * val_correct / val_total
    print(f"Epoch {epoch+1}/{EPOCHS} - Val Accuracy: {val_accuracy:.2f}%")


# ====== SAVE WEIGHTS ======
def save_tensor_as_bin(tensor, name):
    np_array = tensor.detach().cpu().numpy().astype(np.float32)
    np_array.tofile(f"{name}.bin")

print("Saving weights...")
torch.save(model.state_dict(), OUT_PATH + "\\ticknet.pth")
weights = model.state_dict()
weights['net.0.weight'].cpu().numpy().tofile(OUT_PATH + "\\net_0_weight.bin")
weights['net.3.weight'].cpu().numpy().tofile(OUT_PATH + "\\net_3_weight.bin")
weights['net.6.weight'].cpu().numpy().tofile(OUT_PATH + "\\net_6_weight.bin")
weights['net.0.bias'].cpu().numpy().tofile(OUT_PATH + "\\net_0_bias.bin")
weights['net.3.bias'].cpu().numpy().tofile(OUT_PATH + "\\net_3_bias.bin")
weights['net.6.bias'].cpu().numpy().tofile(OUT_PATH + "\\net_6_bias.bin")

# Save valuation dataset
torch.save(val_dataset, OUT_PATH + "/val_dataset.pt")

# --- Export to ONNX ---
dummy_input = torch.randn(1, 1, IMAGE_SIZE, IMAGE_SIZE).to(DEVICE)
torch.onnx.export(
    model, dummy_input, OUT_PATH + "\\ticknet.onnx",
    input_names=["input"], output_names=["logits"],
    dynamic_axes={"input": {0: "batch_size"}, "logits": {0: "batch_size"}},
    opset_version=11
)
print("Done.")
