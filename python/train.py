import os
import torch
import torch.nn as nn
import torch.optim as optim
from torchvision import datasets, transforms
from torch.utils.data import DataLoader
import matplotlib.pyplot as plt
import numpy as np
from sys import argv

# ====== CONFIG ======
IMAGE_SIZE = 64
BATCH_SIZE = 32
EPOCHS = 20
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

train_dataset = CustomImageFolder(DATA_PATH, transform=transform)
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
            nn.Conv2d(1, 8, kernel_size=3, padding=1, bias=False),   # net.0
            nn.ReLU(),                                               # net.1
            nn.MaxPool2d(2),                                         # net.2
            nn.Conv2d(8, 64, kernel_size=3, padding=1, bias=False),  # net.3
            nn.ReLU(),                                               # net.4
            nn.MaxPool2d(2),                                         # net.5
            nn.Conv2d(64, 4, kernel_size=1, bias=False)              # net.6
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
    for imgs, labels in train_loader:
        imgs, labels = imgs.to(DEVICE), labels.to(DEVICE)

        optimizer.zero_grad()
        outputs = model(imgs)
        loss = criterion(outputs, labels)
        loss.backward()
        optimizer.step()
        running_loss += loss.item() * imgs.size(0)

    epoch_loss = running_loss / len(train_loader.dataset)
    print(f"Epoch {epoch+1}/{EPOCHS} - Loss: {epoch_loss:.4f}")

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


# --- Export to ONNX ---
dummy_input = torch.randn(1, 1, IMAGE_SIZE, IMAGE_SIZE).to(DEVICE)
torch.onnx.export(
    model, dummy_input, OUT_PATH + "\\ticknet.onnx",
    input_names=["input"], output_names=["logits"],
    dynamic_axes={"input": {0: "batch_size"}, "logits": {0: "batch_size"}},
    opset_version=11
)
print("Done.")
