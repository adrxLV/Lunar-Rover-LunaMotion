import numpy as np
import matplotlib.pyplot as plt

# Constants
K_REP = 10.0
RHO_0 = 0.3

# Rho values from 0 to 0.8
rho = np.linspace(0.1, 0.8, 100)  # Starting from a small number to avoid division by zero

# Compute the factor
factor = K_REP * np.exp(-rho / RHO_0)
#factor = factor.clip(0, K_REP)

# Plot the result
plt.figure(figsize=(8, 6))
plt.plot(rho, factor, label="label")
plt.xlabel(r'$\rho$', fontsize=14)
plt.ylabel('Factor', fontsize=14)
plt.grid(True)
plt.legend()
plt.show()
