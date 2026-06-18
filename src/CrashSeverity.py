import pandas as pd
import torch
import torch.nn as nn

df = pd.read_csv("crash_data_plausible.csv")

print(df.head())

x = df.drop("severity", axis=1).values
y = df["severity"].values

x_tensor = torch.tensor(x, dtype=torch.float32)
y_tensor = torch.tensor(y, dtype=torch.float32).view(-1, 1)

model = nn.Sequential(
    nn.Linear(50,32),
    nn.ReLU(),
    nn.Linear(32,16),
    nn.ReLU(),
    nn.Linear(16,1)
    

)

accuracy = nn.MSELoss()
optimizer = torch.optim.Adam(model.parameters(), lr=0.001)

for epoch in range(1000):
    predictions = model(x_tensor)
    loss = accuracy(predictions, y_tensor)
    optimizer.zero_grad()
    loss.backward()
    optimizer.step()

    if epoch % 50 == 0:
        print(f"Epoch {epoch} loss: {loss.item()}")

with torch.no_grad():
    sample = x_tensor[1].unsqueeze(0)
    prediction = model(sample)
    print(f"Prediction: {prediction.item()}")
    print(f"Actual: {y_tensor[1].item()}")
    



