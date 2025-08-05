import numpy as np
import matplotlib.pyplot as plt

# Constants


omega = np.linspace(0.0, 12, 1000)  # Starting from a small number to avoid division by zero

# Compute
result = 0.0003*omega**4 - 0.0058*omega**3 + 0.0414*omega**2 - 0.0616*omega

# Plot the result
plt.figure(figsize=(8, 6))
plt.plot(omega, result, label="label")
plt.xlabel(r'$\romega$', fontsize=14)
plt.ylabel('result', fontsize=14)
plt.grid(True)
plt.legend()
plt.show()
