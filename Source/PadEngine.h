#pragma once
#include <array>
#include <vector>
#include <cmath>
#include <algorithm>
namespace abide {
constexpr double pi=3.14159265358979323846;
struct Controls { float gain=.5f,tone=.45f,motion=.3f,width=.8f,attack=2.f,release=5.f,delayMix=.16f,delayMs=450.f,feedback=.3f; int colour=0; };
struct SampleData { std::vector<float> left,right; double sampleRate=32000; };
class PadEngine {
 struct Voice {std::array<double,12> phase{}; float envelope=0; int note=48,colour=0; bool active=false; const SampleData* sample=nullptr; double samplePosition=0,sampleStep=1;};
 std::array<Voice,16> voices{}; std::vector<float> dl,dr; size_t pos=0; double sr=48000,lfo=0; float lpL=0,lpR=0,level=0,delaySamples=21600; int root=48;
public:
 void prepare(double rate){sr=rate;dl.assign(size_t(sr*2.1),0);dr.assign(dl.size(),0); reset();delaySamples=float(sr*.45);}
 void reset(){for(auto&v:voices)v=Voice{};std::fill(dl.begin(),dl.end(),0);std::fill(dr.begin(),dr.end(),0);lpL=lpR=0;level=0;pos=0;lfo=0;}
 void stop(){for(auto&v:voices)v.active=false;}
 void trigger(int note,const SampleData* sample=nullptr,int sampleRoot=48,int colour=0){root=std::clamp(note,24,84);stop();auto it=std::min_element(voices.begin(),voices.end(),[](auto&a,auto&b){return a.envelope<b.envelope;});it->note=root;it->active=true;it->sample=sample;it->samplePosition=0;it->colour=colour;if(sample)it->sampleStep=sample->sampleRate/sr*std::pow(2.,(root-sampleRoot)/12.);}
 int currentNote()const{return root;}
 void tick(const Controls& c,float& left,float& right){
  float l=0,r=0; const float aa=1.f-std::exp(-1.f/float(sr*std::max(.01f,c.attack))),rr=1.f-std::exp(-1.f/float(sr*std::max(.03f,c.release)));
  lfo+=2*pi*.11/sr;if(lfo>2*pi)lfo-=2*pi;
  for(auto& v:voices){v.envelope+=((v.active?1.f:0.f)-v.envelope)*(v.active?aa:rr);if(v.envelope<1.e-6f&&!v.active)continue;
   if(v.sample){const auto& d=*v.sample;const double n=double(d.left.size());const double fade=std::min(d.sampleRate,n*.25);auto read=[&](const std::vector<float>& ch,double p){auto a=size_t(p),b=std::min(a+1,ch.size()-1);float f=float(p-a);return ch[a]*(1-f)+ch[b]*f;};
    auto loop=[&](const std::vector<float>& ch){float value=read(ch,v.samplePosition);if(v.samplePosition>=n-fade){float mix=float((v.samplePosition-(n-fade))/fade);value=value*(1-mix)+read(ch,v.samplePosition-(n-fade))*mix;}return value;};
    float breathe=1.f-c.motion*.18f+float(std::sin(lfo))*.18f*c.motion;l+=loop(d.left)*v.envelope*breathe;r+=loop(d.right)*v.envelope*breathe;v.samplePosition+=v.sampleStep;while(v.samplePosition>=n)v.samplePosition-=n-fade;continue;
   }
   const double base=440*std::pow(2.,(v.note-69)/12.);float vl=0,vr=0;
   for(int i=0;i<12;++i){int octave=i/4;double cents=(i%4-1.5)*5.5;double f=base*std::pow(2.,octave+cents/1200.);v.phase[i]+=2*pi*f/sr;if(v.phase[i]>2*pi)v.phase[i]-=2*pi;
    float s=float(std::sin(v.phase[i]));if(v.colour==1)s+=.22f*float(std::sin(v.phase[i]*3));if(v.colour==2)s+=.16f*float(std::sin(v.phase[i]*2));
    const float amp=(octave==0?.11f:octave==1?.065f:.035f)*v.envelope;float pan=(i%4)/3.f;vl+=s*amp*(1-pan*.7f);vr+=s*amp*(.3f+pan*.7f);
   }float breathe=1.f-c.motion*.18f+float(std::sin(lfo))*.18f*c.motion;l+=vl*breathe;r+=vr*breathe;
  }
  const float cutoff=250.f*std::pow(40.f,std::clamp(c.tone,0.f,1.f));float a=1-std::exp(-2*float(pi)*cutoff/float(sr));lpL+=a*(l-lpL);lpR+=a*(r-lpR);
  l=lpL;r=lpR;const float mid=(l+r)*.5f,side=(l-r)*.5f*std::clamp(c.width,0.f,1.5f);l=mid+side;r=mid-side;
  float target=float(sr)*std::clamp(c.delayMs,40.f,1800.f)/1000;delaySamples+=(target-delaySamples)*.0005f;
  double read=double(pos)+double(dl.size())-delaySamples;while(read<0)read+=double(dl.size());auto i0=size_t(read)%dl.size(),i1=(i0+1)%dl.size();float frac=float(read-std::floor(read));float el=dl[i0]+frac*(dl[i1]-dl[i0]),er=dr[i0]+frac*(dr[i1]-dr[i0]);
  float fb=std::clamp(c.feedback,0.f,.85f);dl[pos]=std::tanh(l+er*fb);dr[pos]=std::tanh(r+el*fb);pos=(pos+1)%dl.size();
  level+=(std::clamp(c.gain,0.f,1.f)-level)*.002f;left=(l+el*c.delayMix)*level;right=(r+er*c.delayMix)*level;
 }
};
}
