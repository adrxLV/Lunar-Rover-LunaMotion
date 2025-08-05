import numpy as np
import matplotlib.pyplot as plt

# Constants
beta_1 = 3.0
beta_2 = 0.2

# Rho values from 0 to 0.8
rho = np.linspace(0.2, 0.8, 100)  # Starting from a small number to avoid division by zero

# Compute the factor
factor = beta_1*np.exp(- rho / beta_2)

# Plot the result
plt.figure(figsize=(8, 6))
plt.plot(rho, factor, label="label")
plt.xlabel(r'$\rho$', fontsize=14)
plt.ylabel('Factor', fontsize=14)
plt.grid(True)
plt.legend()
plt.show()
