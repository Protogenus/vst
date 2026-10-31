"""A/B: render_v1.wav vs target forward2 (31-36s). Same metrics both sides."""
import numpy as np
from numpy.fft import rfft
from scipy.io import wavfile

def load_wav24(path):
    with open(path, "rb") as f:
        data = f.read()
    idx = data.find(b"data")
    nbytes = int.from_bytes(data[idx+4:idx+8], "little")
    raw = data[idx+8:idx+8+nbytes]
    nsamp = len(raw)//6
    a = np.frombuffer(raw[:nsamp*6], dtype=np.uint8).reshape(nsamp*2, 3)
    x = (a[:,0].astype(np.int64) | (a[:,1].astype(np.int64)<<8) | (a[:,2].astype(np.int64)<<16))
    x = np.where(x >= 2**23, x - 2**24, x) / 2**23
    return x.reshape(nsamp, 2).astype(np.float64), 48000

def load_any(path):
    if path.endswith("synth_0_8mvf.wav"):
        return load_wav24(path)
    sr, d = wavfile.read(path)
    return d.astype(np.float64), sr

def metrics(x, sr, t0, t1, f0, tag):
    seg = x[int(t0*sr):int(t1*sr)]
    mid = (seg[:,0]+seg[:,1])/2
    side = (seg[:,0]-seg[:,1])/2
    N=32768; w=np.hanning(N); hop=N//2
    nfr=max(1,(len(side)-N)//hop)
    S=np.abs(rfft(side[np.arange(nfr)[:,None]*hop+np.arange(N)]*w,axis=1)).mean(axis=0)
    Sm=np.abs(rfft(mid[np.arange(nfr)[:,None]*hop+np.arange(N)]*w,axis=1)).mean(axis=0)
    f=np.fft.rfftfreq(N,1/sr)
    hs=[]
    for h in range(1,13):
        m=(f>f0*h-4)&(f<f0*h+4)
        hs.append(20*np.log10(S[m].max()+1e-12) if m.any() else -120)
    hs=np.array(hs); hsn=hs-hs[0]
    m=(f>150)&(f<6000)
    cent=(S[m]*f[m]).sum()/S[m].sum()
    print(f"--- {tag} ---")
    print("harmonic dB rel h1:", np.round(hsn,1))
    print("centroid: %.0f Hz" % cent)
    for fq in (190,380,760,1500,3000):
        i=np.argmin(np.abs(f-fq))
        print(f"  width @{fq}Hz: {10*np.log10((S[i]**2+1e-18)/(Sm[i]**2+1e-18)):+.1f} dB")
    return hsn, cent

xt, srt = load_any("/home/hatch/workspace/user/files/synth_0_8mvf.wav")
xr, srr = load_any("/home/hatch/workspace/vst3-absolute-reason/ab/render_v1.wav")
# target: forward2 31-36s, f0=186.77 ; render: first note F#3 0-3s, f0=185.0 (concert)
ht, ct = metrics(xt, srt, 31.0, 34.0, 186.77, "TARGET forward2")
hr, cr = metrics(xr, srr, 0.3, 2.8, 185.0, "RENDER v1")
print("delta harmonics (render-target):", np.round(hr-ht,1))
