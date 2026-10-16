#include "../Source/PadEngine.h"
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <cstdint>
void check(bool ok,const char* msg){if(!ok)throw std::runtime_error(msg);}
int main(){for(double sr:{44100.,48000.,96000.}){abide::PadEngine e;e.prepare(sr);abide::Controls c;c.attack=.05;c.release=.1;c.delayMix=0;float l,r;double power=0;for(int i=0;i<1000;++i){e.tick(c,l,r);check(l==0&&r==0,"startup not silent");}e.trigger(48);for(int i=0;i<int(sr*2);++i){e.tick(c,l,r);check(std::isfinite(l)&&std::isfinite(r),"non-finite sample");power+=l*l;}check(power>1,"silent voice");e.stop();for(int i=0;i<int(sr*3);++i)e.tick(c,l,r);check(std::abs(l)<1e-5,"release did not fade");c.delayMix=1;c.feedback=.85;for(int n=0;n<100;++n){e.trigger(24+n%61);for(int i=0;i<1024;++i){e.tick(c,l,r);check(std::abs(l)<2&&std::abs(r)<2,"unstable output");}}e.reset();e.tick(c,l,r);check(l==0&&r==0,"panic left audio");}
// Twenty-second stereo demonstration, 48 kHz, PCM 16 bit.
abide::PadEngine e;e.prepare(48000);abide::Controls c;c.gain=.7;std::ofstream f("Abide-Audio-Demo.wav",std::ios::binary);auto w=[&](uint32_t v,int n){for(int i=0;i<n;++i)f.put(char(v>>(i*8)));};int samples=48000*20;f.write("RIFF",4);w(36+samples*4,4);f.write("WAVEfmt ",8);w(16,4);w(1,2);w(2,2);w(48000,4);w(192000,4);w(4,2);w(16,2);f.write("data",4);w(samples*4,4);e.trigger(48);for(int i=0;i<samples;++i){if(i==48000*7)e.trigger(53);if(i==48000*13)e.trigger(55);if(i==48000*16)e.stop();float l,r;e.tick(c,l,r);w(uint16_t(int16_t(std::tanh(l)*32767)),2);w(uint16_t(int16_t(std::tanh(r)*32767)),2);}std::cout<<"PASS: silence, audio generation, finite samples, release, rapid key changes, maximum feedback, panic; 44.1/48/96 kHz\n";}
