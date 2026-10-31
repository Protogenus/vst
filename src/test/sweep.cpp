// Sweep filter cutoff to find best A/B match.
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
int main(int argc, char** argv){
    double cutoff = argc>1 ? atof(argv[1]) : 800.0;
    const int SR=48000; SynthEngine eng; eng.prepare(SR);
    Params p; p.detuneCents=20.0; p.cutoffHz=cutoff; p.resonance=0.0; p.slope24=true;
    p.attackMs=8; p.releaseMs=90; p.chorusDepth=0.0; p.chorusMix=0.0; // dry for clean compare
    p.panSpread=0.85; p.forward=1.0; p.level=0.8;
    eng.setParams(p);
    std::vector<float> L,R; L.reserve(SR*4); R.reserve(SR*4);
    const int BS=512; std::vector<float> bL(BS),bR(BS);
    int total=SR*3, done=0;
    while(done<total){
        int n=BS<(total-done)?BS:(total-done);
        MidiEvent ev[1]; int nev=0;
        if(done==0){ ev[0]={true,54,0}; nev=1; }
        eng.process(bL.data(),bR.data(),n,ev,nev);
        L.insert(L.end(),bL.begin(),bL.begin()+n); R.insert(R.end(),bR.begin(),bR.begin()+n);
        done+=n;
    }
    char path[256]; snprintf(path,256,"/home/hatch/workspace/vst3-absolute-reason/ab/sweep_%.0f.wav",cutoff);
    writeWav(path,L,R,SR);
    return 0;
}
