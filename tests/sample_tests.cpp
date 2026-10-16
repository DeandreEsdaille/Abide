#include "../Source/PluginProcessor.h"
#include <iostream>
#include <stdexcept>
void require(bool value,const char*message){if(!value)throw std::runtime_error(message);}
void wait(SampleLibrary& library){for(int n=0;n<1000&&library.isLoading();++n)juce::Thread::sleep(10);require(!library.isLoading(),"sample load timeout");}
int runSampleTests(){
 juce::ScopedJuceInitialiser_GUI gui;
 auto temp=juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("abide-test-"+juce::Uuid().toString());require(temp.createDirectory().wasOk(),"temp folder");
 auto file=temp.getChildFile("F#-Test.wav");
 {juce::WavAudioFormat format;auto out=file.createOutputStream();std::unique_ptr<juce::AudioFormatWriter> writer(format.createWriterFor(out.get(),44100,1,24,{},0));require(writer!=nullptr,"wav writer");out.release();juce::AudioBuffer<float> data(1,44100*3);for(int i=0;i<data.getNumSamples();++i)data.setSample(0,i,.25f*std::sin(float(2*abide::pi*220*i/44100)));require(writer->writeFromAudioSampleBuffer(data,0,data.getNumSamples()),"wav write");}
 SampleLibrary bank(temp.getChildFile("cache"));int slot=bank.importFile(file);require(slot==SampleLibrary::factoryCount,"first slot");wait(bank);require(bank.get(slot)!=nullptr,"sample missing");require(bank.root(slot)==6,"filename root detection");require(bank.get(slot)->sampleRate==32000,"import resampling");require(bank.get(slot)->left==bank.get(slot)->right,"mono import stereo conversion");auto desc=bank.describe(slot);require(juce::File(desc.path).existsAsFile(),"cached file missing");require(desc.path!=file.getFullPathName(),"original file not cached");
 auto metadata=bank.save();SampleLibrary restored(temp.getChildFile("cache-2"));restored.restore(metadata);wait(restored);require(restored.get(slot)!=nullptr&&restored.root(slot)==6,"library recall");
 abide::PadEngine engine;engine.prepare(48000);abide::Controls c;c.attack=.01;c.release=.1;c.motion=0;c.delayMix=0;c.tone=1;c.gain=1;engine.trigger(60,bank.get(slot),48);float l=0,r=0,last=0;double energy=0,maxJump=0;int crossings=0;
 for(int i=0;i<48000*8;++i){engine.tick(c,l,r);require(std::isfinite(l)&&std::isfinite(r),"sample produced invalid audio");if(i>48000){energy+=l*l;maxJump=std::max(maxJump,double(std::abs(l-last)));}if(i>=48000&&i<96000&&last<=0&&l>0)++crossings;last=l;}
 require(energy>100,"sample silent");require(crossings>435&&crossings<445,"octave transposition");require(maxJump<.1,"sample loop discontinuity");engine.reset();engine.tick(c,l,r);require(l==0&&r==0,"sample panic");
 {auto p=std::make_unique<AbideProcessor>();p->library.restore(metadata);wait(p->library);p->set("drone",float(slot+1));p->set("attack",.05f);p->prepareToPlay(48000,256);juce::AudioBuffer<float> buffer(2,256);juce::MidiBuffer midi;energy=0;
  for(int b=0;b<300;++b){if(b==0)midi.addEvent(juce::MidiMessage::noteOn(1,54,juce::uint8(100)),0);if(b==100)p->set("drone",0);if(b==200)p->set("drone",float(slot+1));p->processBlock(buffer,midi);for(int ch=0;ch<2;++ch)for(int i=0;i<256;++i){auto v=buffer.getSample(ch,i);require(std::isfinite(v)&&std::abs(v)<=1,"sound switching output");energy+=v*v;}}
  require(energy>1,"native sample playback silent");juce::MemoryBlock state;p->getStateInformation(state);auto next=std::make_unique<AbideProcessor>();next->setStateInformation(state.getData(),int(state.getSize()));next->handleAsyncUpdate();wait(next->library);require(next->get("drone")==float(slot+1)&&next->library.get(slot)!=nullptr,"processor sample project recall");
  p->set("latch",0);midi.addEvent(juce::MidiMessage::noteOn(1,127,juce::uint8(100)),0);p->processBlock(buffer,midi);midi.addEvent(juce::MidiMessage::noteOff(1,127),0);p->processBlock(buffer,midi);require(!p->sounding,"clamped MIDI note stuck");
 }
 int mp3Slot=bank.importFile(juce::File(__FILE__).getSiblingFile("data").getChildFile("A-Test.mp3"));wait(bank);require(bank.get(mp3Slot)!=nullptr,"MP3 import failed");
 auto missing=metadata.createCopy();missing.getChild(0).setProperty("path",temp.getChildFile("gone.wav").getFullPathName(),nullptr);SampleLibrary absent(temp.getChildFile("absent"));absent.restore(missing);wait(absent);require(absent.get(slot)==nullptr&&absent.describe(slot).error.isNotEmpty(),"missing file not reported");
 auto bad=temp.getChildFile("Invalid.wav");bad.replaceWithText("not an audio file");int badSlot=bank.importFile(bad);wait(bank);require(bank.get(badSlot)==nullptr&&bank.describe(badSlot).error.isNotEmpty(),"invalid file not handled");
 temp.deleteRecursively();std::cout<<"PASS samples: WAV/MP3 import, resampling, root detection, local cache, loops, transposition, sound switching, project recall, missing/invalid file and clamped MIDI release\n";return 0;
}
