// Final default-preset render for A/B (full preset incl. chorus).
#include "../dsp/SynthEngine.h"
#include <cstdio>
#include <cstdint>
#include <vector>
using namespace absynth;
static void writeWav(const char* path, const std::vector<float>& L, const std::vector<float>& R, int sr){
    FILE* f=std::fopen(path,"wb"); uint32_t n=(uint32_t)L.size();
    uint32_t db=n*2*4, cs=36+db;
    fwrite("RIFF",1,4,f); fwrite(&cs,4,1,f); fwrite("WAVE",1,4,f);
    fwrite("fmt ",1,4,f); uint32_t fl=16; fwrite(&fl,4,1,f);
    uint16_t af=3; fwrite(&af,2,1,f); uint16_t ch=2; fwrite(&ch,2,1,f);
    uint32_t sru=sr; fwrite(&sru,4,1,f); uint32_t br=sru*2*4; fwrite(&br,4,1,f);
    uint16_t ba=8; fwrite(&ba,2,1,f); uint16_t bits=32; fwrite(&bits,2,1,f);
    fwrite("data",1,4,f); fwrite(&db,4,1,f);
    for(uint32_t i=0;i<n;i++){ fwrite(&L[i],4,1,f); fwrite(&R[i],4,1,f); }
    fclose(f);
}
int main(){
    const int SR=48000; SynthEngine eng; eng.prepare(SR);
    Params p; // FINAL default preset — "Absolute Reason Growl" (loud/forward state)
    p.detuneCents=20.0; p.cutoffHz=800.0; p.resonance=0.0; p.slope24=true;
    p.attackMs=8; p.releaseMs=90; p.chorusDepth=0.35; p.chorusRateHz=0.35;
    p.chorusMix=0.22; p.panSpread=0.85; p.forward=1.0; p.level=0.8;
    eng.setParams(p);
    std::vector<float> L,R; L.reserve(SR*12); R.reserve(SR*12);
    const int BS=512; std::vector<float> bL(BS),bR(BS);
    struct N{int m;double s;}; std::vector<N> riff={{54,2.5},{63,0.4},{54,2.5},{63,0.4},{54,3.0}};
    for(auto nt:riff){
        int total=(int)(nt.s*SR),done=0;
        while(done<total){ int n=BS<(total-done)?BS:(total-done);
            MidiEvent ev[1]; int nev=0; if(done==0){ev[0]={true,nt.m,0};nev=1;}
            eng.process(bL.data(),bR.data(),n,ev,nev);
            L.insert(L.end(),bL.begin(),bL.begin()+n); R.insert(R.end(),bR.begin(),bR.begin()+n); done+=n; }
        int rel=(int)(0.3*SR); done=0;
        while(done<rel){ int n=BS<(rel-done)?BS:(rel-done);
            MidiEvent ev[1]; int nev=0; if(done==0){ev[0]={false,nt.m,0};nev=1;}
            eng.process(bL.data(),bR.data(),n,ev,nev);
            L.insert(L.end(),bL.begin(),bL.begin()+n); R.insert(R.end(),bR.begin(),bR.begin()+n); done+=n; }
    }
    writeWav("/home/hatch/workspace/vst3-absolute-reason/ab/final_preset.wav",L,R,SR);
    printf("done %zu samples\n",L.size());
    return 0;
}
