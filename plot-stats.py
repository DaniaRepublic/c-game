import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv('miss-stats.csv')

# Create a figure and plot a line graph.
plt.figure(figsize=(8, 5))
plt.plot(df['id'], df['miss'], marker='o', linestyle='-', color='b')

plt.xlabel('ID')
plt.ylabel('Miss')
plt.title('Line Graph of Miss vs ID')
plt.grid(True)

plt.show()
