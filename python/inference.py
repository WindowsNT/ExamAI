import torch
import torch.nn as nn
import torchvision.transforms as transforms
from PIL import Image
import sys

# ---- Configuration ----
#IMAGE_PATH = "synthetic_marks_drawn/2_cross/2_cross_0013.png"  # Replace or use sys.argv[1]
#IMAGE_PATH = "synthetic_marks_drawn/3_dot/3_dot_0029.png"  # Replace or use sys.argv[1]
#IMAGE_PATH = "synthetic_marks_drawn/1_tick/1_tick_0213.png"  # Replace or use sys.argv[1]
MODEL_PATH = "ticknet.pth"            # Path to your trained model
IMAGE_SIZE = 64                     # Or 32 if that's what you trained on
NUM_CLASSES = 4

# ---- Define the model (same architecture as training) ----
class TickClassifier(nn.Module):
    def __init__(self):
        super().__init__()
        self.net = nn.Sequential(
            nn.Conv2d(1, 8, 3, padding=1, bias=False),  # [B, 1, 64, 64] ? [B, 8, 64, 64]
            nn.ReLU(),
            nn.MaxPool2d(2),                # ? [B, 8, 32, 32]
            nn.Conv2d(8, 64, 3, padding=1, bias=False), # ? [B, 64, 32, 32]
            nn.ReLU(),
            nn.MaxPool2d(2),                # ? [B, 64, 16, 16]
            nn.Conv2d(64, NUM_CLASSES, 1, bias=False),  # ? [B, 4, 16, 16]
            nn.AdaptiveAvgPool2d((1, 1))    # ? [B, 4, 1, 1]
        )

    def forward(self, x):
        x = self.net(x)
        return x.view(x.size(0), -1)  # Flatten to [B, 4]

# ---- Load model ----
model = TickClassifier()
model.load_state_dict(torch.load(MODEL_PATH, map_location='cpu'))
model.eval()

# ---- Load image ----
img_path = sys.argv[1] if len(sys.argv) > 1 else IMAGE_PATH
img = Image.open(img_path).convert('L')

# ---- Preprocess image ----
transform = transforms.Compose([
    transforms.Resize((IMAGE_SIZE, IMAGE_SIZE)),
    transforms.ToTensor(),  # Converts to [0,1] and shape [1, H, W]
])

tensor = transform(img).unsqueeze(0)  # Add batch dim ? [1, 1, 64, 64]

# ---- Inference ----
with torch.no_grad():
    logits = model(tensor)
    probs = torch.softmax(logits, dim=1)
    pred_class = torch.argmax(probs, dim=1).item()

# ---- Output ----
print(f"Logits: {logits.numpy().flatten()}")
print(f"Probs:  {probs.numpy().flatten()}")
print(f"Predicted class: {pred_class}")
