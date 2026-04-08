import numpy as np
import matplotlib.pyplot as plt
import struct

filename = "log015.bin"

# ========= LOAD =========
samples = []

with open(filename, "rb") as f:
    while True:
        chunk = f.read(10)  # 10 bytes per sample
        if len(chunk) < 10:
            break
        
        t, x, y, z = struct.unpack("<Ihhh", chunk)
        samples.append((t, x, y, z))

samples = np.array(samples)

t = samples[:,0].astype(np.float64)
x = samples[:,1]
y = samples[:,2]
z = samples[:,3]

# ========= TIME =========
t = (t - t[0]) / 1e6  # seconds
dt = np.diff(t)

fs_mean = 1 / np.mean(dt)

print("==== SAMPLING STATS ====")
print(f"Samples: {len(t)}")
print(f"Mean dt: {np.mean(dt)*1e6:.2f} us")
print(f"Std dt (jitter): {np.std(dt)*1e6:.2f} us")
print(f"Mean Fs: {fs_mean:.2f} Hz")

# ========= TIME SERIES =========
scale = 1 / 4096.0  # g per LSB (for ±8g)

x_g = x * scale
y_g = y * scale
z_g = z * scale


plt.figure()
plt.plot(t, x_g, label='X')
plt.plot(t, y_g, label='Y')
plt.plot(t, z_g, label='Z')
plt.legend()
plt.xlabel("Time (s)")
plt.ylabel("Raw Accel")
plt.title("Time Series")
plt.grid()

# ========= FFT =========
mag = np.sqrt(x_g**2 + y_g**2 + z_g**2)

signal = mag - np.mean(mag)
N = len(signal)

fft_vals = np.fft.rfft(signal)
freqs = np.fft.rfftfreq(N, d=1/fs_mean)

plt.figure()
plt.plot(freqs, np.abs(fft_vals))
plt.title("FFT (mag)")
plt.xlabel("Frequency (Hz)")
plt.ylabel("Magnitude")
plt.xlim(0, fs_mean/2)
plt.grid()

plt.show()