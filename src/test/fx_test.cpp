#include "../dsp/SynthEngine.h"
#include <cstdio>
#include <vector>
#include <cmath>
using namespace absynth;
int main(){
    SynthEngine eng; eng.prepare(48000);
    Params p; // default incl. FX
    eng.setParams(p);
    const int N = 48000*2;
    std::vector<float> L(N), R(N);
    MidiEvent on{true, 54, 0}, off{false, 54, 0};
    // render in blocks: note on at 0, off at 1s, then 1s of reverb tail
    const int BS=512; int idx=0;
    std::vector<float> bL(BS), bR(BS);
    eng.process(bL.data(), bR.data(), 1, &on, 1); // prime
    while(idx < N){
        int n = BS < (N-idx) ? BS : (N-idx);
        int nev=0; MidiEvent ev[1];
        if(idx==0){ ev[0]=on; nev=1; }
        if(idx >= 48000 && idx < 48000+BS){ ev[0]=off; nev=1; }
        eng.process(bL.data(), bR.data(), n, ev, nev);
        for(int i=0;i<n;i++){ L[idx+i]=bL[i]; R[idx+i]=bR[i]; }
        idx+=n;
    }
    double peak=0, tailE=0;
    for(int i=0;i<N;i++){ double a=std::abs(L[i]); if(a>peak)peak=a; }
    for(int i=48000+24000;i<N;i++) tailE += L[i]*L[i]+R[i]*R[i];
    printf("peak=%.3f tailEnergy(1.5-2s)=%.6f %s\n", peak, tailE,
           (peak>0.01 && peak<2.0 && tailE>1e-8) ? "OK" : "SUSPECT");
    return 0;
}
