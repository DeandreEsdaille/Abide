#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include "BinaryData.h"
#include "PadEngine.h"
class AbideEditor;
class AbideProcessor : public juce::AudioProcessor {
public:
 juce::AudioProcessorValueTreeState state;
 abide::PadEngine engine; juce::Reverb reverb;juce::SmoothedValue<float> masterGain;std::atomic<int> command{-1},shownNote{48};std::atomic<bool> sounding{false};
 AbideProcessor():AudioProcessor(BusesProperties().withOutput("Output",juce::AudioChannelSet::stereo(),true)),state(*this,nullptr,"Abide",layout()){}
 static juce::AudioProcessorValueTreeState::ParameterLayout layout(){
  juce::AudioProcessorValueTreeState::ParameterLayout l;
  auto add=[&](const char* id,const char* name,float min,float max,float def){const int decimals=juce::String(id)=="time"?0:2;l.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{id,1},name,juce::NormalisableRange<float>(min,max),def,juce::AudioParameterFloatAttributes{}.withStringFromValueFunction([decimals](float value,int){return juce::String(value,decimals);})));};
  add("gain","Volume",0,1,.5);add("tone","Tone",0,1,.45);add("motion","Movement",0,1,.3);add("width","Width",0,1.5,.8);add("attack","Attack",.05,10,2);add("release","Release",.1,20,5);add("reverb","Reverb Mix",0,1,.4);add("size","Reverb Size",0,1,.8);add("delay","Delay Mix",0,1,.16);add("time","Delay Time (ms)",40,1800,450);add("feedback","Feedback",0,.85,.3);
  l.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"latch",1},"Latch",true));l.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"colour",1},"Texture",juce::StringArray{"Velvet","Radiance","Halo"},0));return l;
 }
 float get(const char* id)const{return state.getRawParameterValue(id)->load();}
 void set(const char* id,float v){auto*p=state.getParameter(id);p->beginChangeGesture();p->setValueNotifyingHost(p->convertTo0to1(v));p->endChangeGesture();}
 void prepareToPlay(double sr,int)override{engine.prepare(sr);reverb.setSampleRate(sr);reverb.reset();masterGain.reset(sr,.03);masterGain.setCurrentAndTargetValue(get("gain"));sounding=false;command=-1;}
 void releaseResources()override{}
 bool isBusesLayoutSupported(const BusesLayout& b)const override{return b.getMainOutputChannelSet()==juce::AudioChannelSet::stereo();}
 void processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer& midi)override{
  juce::ScopedNoDenormals guard;buffer.clear();abide::Controls c;c.gain=1.f;masterGain.setTargetValue(get("gain"));c.tone=get("tone");c.motion=get("motion");c.width=get("width");c.attack=get("attack");c.release=get("release");c.delayMix=get("delay");c.delayMs=get("time");c.feedback=get("feedback");c.colour=int(get("colour"));
  auto trigger=[&](int n){engine.trigger(n);shownNote=engine.currentNote();sounding=true;};int cmd=command.exchange(-1);if(cmd>=0)trigger(cmd);else if(cmd==-2){engine.stop();sounding=false;}else if(cmd==-3){engine.reset();reverb.reset();sounding=false;}
  auto it=midi.begin();auto*l=buffer.getWritePointer(0);auto*r=buffer.getWritePointer(1);
  for(int i=0;i<buffer.getNumSamples();++i){while(it!=midi.end()&&(*it).samplePosition<=i){auto m=(*it).getMessage();if(m.isNoteOn())trigger(m.getNoteNumber());else if(m.isAllSoundOff()){engine.reset();reverb.reset();sounding=false;}else if(m.isAllNotesOff()||(m.isNoteOff()&&get("latch")<.5&&m.getNoteNumber()==engine.currentNote())){engine.stop();sounding=false;}++it;}engine.tick(c,l[i],r[i]);}
  juce::Reverb::Parameters rp;rp.roomSize=get("size");rp.damping=.55f;rp.wetLevel=get("reverb")*.6f;rp.dryLevel=1.f-get("reverb")*.35f;rp.width=juce::jlimit(0.f,1.f,c.width);reverb.setParameters(rp);reverb.processStereo(l,r,buffer.getNumSamples());
  for(int i=0;i<buffer.getNumSamples();++i){float gain=masterGain.getNextValue();for(int ch=0;ch<2;++ch)buffer.setSample(ch,i,std::tanh(buffer.getSample(ch,i)*gain));}midi.clear();
 }
 void preset(int n){const float values[4][11]={{.5,.45,.3,.8,2,5,.4,.8,.16,450,.3},{.5,.28,.2,1,4,9,.65,.95,.12,600,.35},{.45,.7,.55,1.3,1.5,7,.5,.85,.3,750,.5},{.5,.55,.4,.9,.5,3,.28,.65,.2,375,.3}};const char* ids[]={"gain","tone","motion","width","attack","release","reverb","size","delay","time","feedback"};for(int i=0;i<11;++i)set(ids[i],values[n][i]);set("colour",float(n==2?2:n==3?1:0));}
 juce::AudioProcessorEditor* createEditor()override;bool hasEditor()const override{return true;}
 const juce::String getName()const override{return "Abide";}bool acceptsMidi()const override{return true;}bool producesMidi()const override{return false;}double getTailLengthSeconds()const override{return 60;}int getNumPrograms()override{return 1;}int getCurrentProgram()override{return 0;}void setCurrentProgram(int)override{}const juce::String getProgramName(int)override{return "Default";}void changeProgramName(int,const juce::String&)override{}
 void getStateInformation(juce::MemoryBlock& b)override{auto xml=state.copyState().createXml();copyXmlToBinary(*xml,b);}
 void setStateInformation(const void* data,int size)override{auto xml=getXmlFromBinary(data,size);if(xml&&xml->hasTagName(state.state.getType()))state.replaceState(juce::ValueTree::fromXml(*xml));}
};
class AbideEditor : public juce::AudioProcessorEditor, private juce::Timer {
 AbideProcessor&p;juce::Image art;juce::LookAndFeel_V4 skin;std::array<juce::TextButton,12> keys;std::array<juce::Slider,11> knobs;std::array<juce::Label,11> labels;std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> attachments;
 juce::TextButton stop{"FADE OUT"},panic{"PANIC"};juce::ToggleButton latch{"LATCH"};juce::ComboBox presets,texture;juce::Label status;std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> latchLink;std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> textureLink;
 const juce::StringArray names{"C","C#","D","Eb","E","F","F#","G","Ab","A","Bb","B"};
public:
 AbideEditor(AbideProcessor&proc):AudioProcessorEditor(proc),p(proc){
  art=juce::ImageCache::getFromMemory(BinaryData::atmosphere_jpg,BinaryData::atmosphere_jpgSize);skin.setColour(juce::Slider::rotarySliderFillColourId,juce::Colour(0xffb9d8ce));skin.setColour(juce::Slider::thumbColourId,juce::Colours::white);skin.setColour(juce::Slider::textBoxTextColourId,juce::Colours::white);skin.setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);skin.setColour(juce::TextButton::buttonColourId,juce::Colour(0xff1b2c39));skin.setColour(juce::TextButton::buttonOnColourId,juce::Colour(0xff527c79));setLookAndFeel(&skin);
  for(int i=0;i<12;++i){auto&b=keys[i];b.setButtonText(names[i]);b.onClick=[this,i]{p.command=48+i;};addAndMakeVisible(b);}
  const char* ids[]={"gain","tone","motion","width","attack","release","reverb","size","delay","time","feedback"};const char* titles[]={"VOLUME","TONE","MOVEMENT","WIDTH","ATTACK (s)","RELEASE (s)","REVERB","ROOM SIZE","DELAY","TIME (ms)","FEEDBACK"};
  for(int i=0;i<11;++i){auto&k=knobs[i];k.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);k.setTextBoxStyle(juce::Slider::TextBoxBelow,false,80,20);k.setNumDecimalPlacesToDisplay(i==9?0:2);addAndMakeVisible(k);labels[i].setText(titles[i],juce::dontSendNotification);labels[i].setJustificationType(juce::Justification::centred);labels[i].setFont(12.f);addAndMakeVisible(labels[i]);attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.state,ids[i],k));}
  presets.addItemList({"Still Waters","Prayer Room","Open Heaven","Morning Light"},1);presets.setSelectedId(0,juce::dontSendNotification);presets.setTextWhenNothingSelected("Preset / custom");presets.onChange=[this]{p.preset(presets.getSelectedId()-1);};addAndMakeVisible(presets);
  texture.addItemList({"Velvet","Radiance","Halo"},1);addAndMakeVisible(texture);textureLink=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.state,"colour",texture);
  latchLink=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.state,"latch",latch);addAndMakeVisible(latch);stop.onClick=[this]{p.command=-2;};panic.onClick=[this]{p.command=-3;};addAndMakeVisible(stop);addAndMakeVisible(panic);addAndMakeVisible(status);status.setJustificationType(juce::Justification::centred);status.setFont(18.f);status.setText("Choose a key / MIDI ready",juce::dontSendNotification);setSize(1040,740);startTimerHz(15);
 }
 ~AbideEditor()override{setLookAndFeel(nullptr);}
 void timerCallback()override{int root=(p.shownNote.load()%12+12)%12;for(int i=0;i<12;++i)keys[i].setToggleState(p.sounding&&i==root,juce::dontSendNotification);status.setText(p.sounding?names[root]+"  /  DRONE ACTIVE":"Choose a key / MIDI ready",juce::dontSendNotification);}
 void paint(juce::Graphics&g)override{g.fillAll(juce::Colour(0xff101a27));if(art.isValid())g.drawImage(art,getLocalBounds().toFloat(),juce::RectanglePlacement::fillDestination);g.setColour(juce::Colour(0xff081421).withAlpha(.62f));g.fillAll();g.setColour(juce::Colours::white);g.setFont(48.f);g.drawText("ABIDE",40,38,400,60,juce::Justification::left);g.setFont(12.f);g.drawText("G I M M E  T H A T  J U I C E  /  A T M O S P H E R I C  P A D S",43,100,700,25,juce::Justification::left);g.setColour(juce::Colour(0xff081421).withAlpha(.78f));g.fillRoundedRectangle(30,270,980,408,20);g.setColour(juce::Colours::white.withAlpha(.55f));g.setFont(12.f);g.drawText("One note. Room to worship.     |     MIDI selects the root; Latch holds it after release.",40,696,960,22,juce::Justification::centred);}
 void resized()override{presets.setBounds(770,48,230,32);texture.setBounds(770,92,230,30);status.setBounds(300,146,440,35);for(int i=0;i<12;++i)keys[i].setBounds(40+i*80,204,72,48);for(int i=0;i<11;++i){int row=i<6?0:1,col=i<6?i:i-6;int x=48+col*158;labels[i].setBounds(x,292+row*158,150,24);knobs[i].setBounds(x+13,319+row*158,124,122);}latch.setBounds(850,481,120,28);stop.setBounds(847,527,130,36);panic.setBounds(847,577,130,30);}
};
juce::AudioProcessorEditor* AbideProcessor::createEditor(){return new AbideEditor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new AbideProcessor;}
