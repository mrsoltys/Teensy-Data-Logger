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
plt.figure()
plt.plot(t, x, label='X')
plt.plot(t, y, label='Y')
plt.plot(t, z, label='Z')
plt.legend()
plt.xlabel("Time (s)")
plt.ylabel("Raw Accel")
plt.title("Time Series")
plt.grid()

# ========= JITTER =========
plt.figure()
plt.plot(dt * 1e6)
plt.title("Timing Jitter (µs)")
plt.xlabel("Sample Index")
plt.ylabel("dt (µs)")
plt.grid()

# ========= FFT =========
signal = x - np.mean(x)
N = len(signal)

fft_vals = np.fft.rfft(signal)
freqs = np.fft.rfftfreq(N, d=1/fs_mean)

plt.figure()
plt.plot(freqs, np.abs(fft_vals))
plt.title("FFT (X-axis)")
plt.xlabel("Frequency (Hz)")
plt.ylabel("Magnitude")
plt.xlim(0, fs_mean/2)
plt.grid()

plt.show()