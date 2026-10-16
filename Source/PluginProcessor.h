#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include "PadEngine.h"
#include "SampleLibrary.h"
class AbideProcessor : public juce::AudioProcessor, private juce::AsyncUpdater {
public:
 juce::AudioProcessorValueTreeState state;
 SampleLibrary library;
 juce::CriticalSection restoreLock;juce::ValueTree pendingLibrary;
 int activeDrone=-1,activeColour=-1,activeRoot=-1,heldInputNote=48;const abide::SampleData* activeSample=nullptr;
 abide::PadEngine engine; juce::Reverb reverb;juce::SmoothedValue<float> masterGain;std::atomic<int> command{-1},shownNote{48};std::atomic<bool> sounding{false};
 AbideProcessor():AudioProcessor(BusesProperties().withOutput("Output",juce::AudioChannelSet::stereo(),true)),state(*this,nullptr,"Abide",layout()){initializeFactorySounds();}
 void initializeFactorySounds();
 ~AbideProcessor()override{cancelPendingUpdate();}
 void handleAsyncUpdate()override{juce::ValueTree saved;{const juce::ScopedLock lock(restoreLock);saved=pendingLibrary;pendingLibrary={};}if(saved.isValid())library.restore(saved);}
 static juce::AudioProcessorValueTreeState::ParameterLayout layout(){
  juce::AudioProcessorValueTreeState::ParameterLayout l;
  auto add=[&](const char* id,const char* name,float min,float max,float def){const int decimals=juce::String(id)=="time"?0:2;l.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{id,1},name,juce::NormalisableRange<float>(min,max),def,juce::AudioParameterFloatAttributes{}.withStringFromValueFunction([decimals](float value,int){return juce::String(value,decimals);})));};
  add("gain","Volume",0,1,.5);add("tone","Tone",0,1,.45);add("motion","Movement",0,1,.3);add("width","Width",0,1.5,.8);add("attack","Attack",.05,10,2);add("release","Release",.1,20,5);add("reverb","Reverb Mix",0,1,.4);add("size","Reverb Size",0,1,.8);add("delay","Delay Mix",0,1,.16);add("time","Delay Time (ms)",40,1800,450);add("feedback","Feedback",0,.85,.3);
  l.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"latch",1},"Latch",true));l.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"colour",1},"Texture",juce::StringArray{"Velvet","Radiance","Halo"},0));
  l.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"drone",1},"Drone Sound",juce::StringArray{"Built-in synthesizer","Heavenly Choir","Cloud Drift","Imported 1","Imported 2","Imported 3","Imported 4","Imported 5","Imported 6","Imported 7","Imported 8"},1));return l;
 }
 float get(const char* id)const{return state.getRawParameterValue(id)->load();}
 void set(const char* id,float v){auto*p=state.getParameter(id);p->beginChangeGesture();p->setValueNotifyingHost(p->convertTo0to1(v));p->endChangeGesture();}
 void prepareToPlay(double sr,int)override{engine.prepare(sr);reverb.setSampleRate(sr);reverb.reset();masterGain.reset(sr,.03);masterGain.setCurrentAndTargetValue(get("gain"));sounding=false;command=-1;activeDrone=-1;activeSample=nullptr;}
 void releaseResources()override{}
 bool isBusesLayoutSupported(const BusesLayout& b)const override{return b.getMainOutputChannelSet()==juce::AudioChannelSet::stereo();}
 void processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer& midi)override{
  juce::ScopedNoDenormals guard;buffer.clear();abide::Controls c;c.gain=1.f;masterGain.setTargetValue(get("gain"));c.tone=get("tone");c.motion=get("motion");c.width=get("width");c.attack=get("attack");c.release=get("release");c.delayMix=get("delay");c.delayMs=get("time");c.feedback=get("feedback");c.colour=int(get("colour"));
  auto trigger=[&](int n,bool updateHeld=true){int drone=int(get("drone"));auto*sample=drone>0?library.get(drone-1):nullptr;int sampleRoot=drone>0?library.root(drone-1):0;activeDrone=drone;activeSample=sample;activeRoot=sampleRoot;activeColour=c.colour;if(updateHeld)heldInputNote=n;if(drone>0&&!sample){engine.stop();sounding=false;return;}engine.trigger(n,sample,48+sampleRoot,c.colour);shownNote=engine.currentNote();sounding=true;};
  int drone=int(get("drone"));if(sounding&&(drone!=activeDrone||(drone==0&&activeColour!=c.colour)||(drone>0&&(library.get(drone-1)!=activeSample||library.root(drone-1)!=activeRoot))))trigger(engine.currentNote(),false);
  int cmd=command.exchange(-1);if(cmd>=0)trigger(cmd);else if(cmd==-2){engine.stop();sounding=false;}else if(cmd==-3){engine.reset();reverb.reset();sounding=false;}
  auto it=midi.begin();auto*l=buffer.getWritePointer(0);auto*r=buffer.getWritePointer(1);
  for(int i=0;i<buffer.getNumSamples();++i){while(it!=midi.end()&&(*it).samplePosition<=i){auto m=(*it).getMessage();if(m.isNoteOn())trigger(m.getNoteNumber());else if(m.isAllSoundOff()){engine.reset();reverb.reset();sounding=false;}else if(m.isAllNotesOff()||(m.isNoteOff()&&get("latch")<.5&&m.getNoteNumber()==heldInputNote)){engine.stop();sounding=false;}++it;}engine.tick(c,l[i],r[i]);}
  juce::Reverb::Parameters rp;rp.roomSize=get("size");rp.damping=.55f;rp.wetLevel=get("reverb")*.6f;rp.dryLevel=1.f-get("reverb")*.35f;rp.width=juce::jlimit(0.f,1.f,c.width);reverb.setParameters(rp);reverb.processStereo(l,r,buffer.getNumSamples());
  for(int i=0;i<buffer.getNumSamples();++i){float gain=masterGain.getNextValue();for(int ch=0;ch<2;++ch)buffer.setSample(ch,i,std::tanh(buffer.getSample(ch,i)*gain));}midi.clear();
 }
 void preset(int n){const float values[4][11]={{.5,.45,.3,.8,2,5,.4,.8,.16,450,.3},{.5,.28,.2,1,4,9,.65,.95,.12,600,.35},{.45,.7,.55,1.3,1.5,7,.5,.85,.3,750,.5},{.5,.55,.4,.9,.5,3,.28,.65,.2,375,.3}};const char* ids[]={"gain","tone","motion","width","attack","release","reverb","size","delay","time","feedback"};for(int i=0;i<11;++i)set(ids[i],values[n][i]);set("colour",float(n==2?2:n==3?1:0));}
 juce::AudioProcessorEditor* createEditor()override;bool hasEditor()const override{return true;}
 const juce::String getName()const override{return "Abide";}bool acceptsMidi()const override{return true;}bool producesMidi()const override{return false;}double getTailLengthSeconds()const override{return 60;}int getNumPrograms()override{return 1;}int getCurrentProgram()override{return 0;}void setCurrentProgram(int)override{}const juce::String getProgramName(int)override{return "Default";}void changeProgramName(int,const juce::String&)override{}
 void getStateInformation(juce::MemoryBlock& b)override{auto saved=state.copyState();auto previous=saved.getChildWithName("DroneLibrary");if(previous.isValid())saved.removeChild(previous,nullptr);saved.addChild(library.save(),-1,nullptr);auto xml=saved.createXml();copyXmlToBinary(*xml,b);}
 void setStateInformation(const void* data,int size)override{auto xml=getXmlFromBinary(data,size);if(xml&&xml->hasTagName(state.state.getType())){auto saved=juce::ValueTree::fromXml(*xml);state.replaceState(saved);auto lib=saved.getChildWithName("DroneLibrary");if(lib.isValid()){const juce::ScopedLock lock(restoreLock);pendingLibrary=lib.createCopy();triggerAsyncUpdate();}}}
};
