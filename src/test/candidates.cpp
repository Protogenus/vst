// Renders candidate oscillator models for A/B against target.
#include "../dsp/SawOsc.h"
#include "../dsp/LowpassFilter.h"
#include <cstdio>
#include <cstdint>
#include <vector>
#include <cmath>
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
// Candidate renderers: each renders 3s of F#3 (186.77Hz) stereo.
static void render(const char* path, int model, double sr){
    const double f0=186.77; const int N=(int)(3*sr);
    std::vector<float> L(N),R(N);
    SawOsc a,b,c; a.setSampleRate(sr); b.setSampleRate(sr); c.setSampleRate(sr);
    auto det = [&](double cents){ return std::pow(2.0,cents/1200.0); };
    // model params
    double dA=0,dB=0,dC=0,invB=1.0,pan=0.85;
    if(model==1){ dA=-10;dB=10; }                 // 2 saws +-10c (current)
    if(model==2){ dA=-3;dB=3; }                   // 2 saws +-3c (tiny)
    if(model==3){ dA=-10;dB=0;dC=10; }            // 3 saws
    if(model==4){ dA=-10;dB=10;invB=-1.0; }       // 2 saws, one inverted
    if(model==5){ dA=-25;dB=25; }                 // 2 saws +-25c (wide)
    a.setFrequency(f0*det(dA)); b.setFrequency(f0*det(dB)); c.setFrequency(f0*det(dC));
    LowpassFilter fL,fR; fL.setSampleRate(sr); fR.setSampleRate(sr);
    fL.setCutoff(6000); fR.setCutoff(6000); fL.setQ(0.7); fR.setQ(0.7);
    for(int i=0;i<N;i++){
        double sa=a.process(), sb=b.process()*invB;
        double l,m,r;
        if(model==3){ double sc=c.process(); l=(sa*0.75+sb*0.5+sc*0.25); r=(sc*0.75+sb*0.5+sa*0.25); }
        else { l=sa*(0.5+0.5*pan)+sb*(0.5-0.5*pan); r=sb*(0.5+0.5*pan)+sa*(0.5-0.5*pan); }
        // simple attack ramp 10ms to avoid click
        double g = i<480 ? (double)i/480 : 1.0;
        L[i]=(float)(fL.process(l)*g*0.5); R[i]=(float)(fR.process(r)*g*0.5);
    }
    writeWav(path,L,R,(int)sr);
    printf("model %d -> %s\n",model,path);
}
int main(){
    render("/home/hatch/workspace/vst3-absolute-reason/ab/cand1.wav",1,48000);
    render("/home/hatch/workspace/vst3-absolute-reason/ab/cand2.wav",2,48000);
    render("/home/hatch/workspace/vst3-absolute-reason/ab/cand3.wav",3,48000);
    render("/home/hatch/workspace/vst3-absolute-reason/ab/cand4.wav",4,48000);
    render("/home/hatch/workspace/vst3-absolute-reason/ab/cand5.wav",5,48000);
    return 0;
}
