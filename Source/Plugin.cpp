#include "PluginProcessor.h"
#include "BinaryData.h"
#include "PremiumLook.h"
void AbideProcessor::initializeFactorySounds(){library.addFactory(0,BinaryData::HeavenlyChoir_flac,BinaryData::HeavenlyChoir_flacSize,"Heavenly Choir");library.addFactory(1,BinaryData::CloudDrift_flac,BinaryData::CloudDrift_flacSize,"Cloud Drift");}
class AbideEditor : public juce::AudioProcessorEditor, private juce::Timer {
 AbideProcessor&p;juce::Image art;PremiumLook skin;
 std::array<juce::TextButton,4> pads;
 std::array<juce::TextButton,12> keys;std::array<juce::Slider,11> knobs;std::array<juce::Label,11> labels;
 std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> attachments;
 juce::TextButton stop{"FADE OUT"},panic{"PANIC"},importButton{"IMPORT DRONES..."},pixabay{"SAMPLE SOURCES"};
 juce::ToggleButton latch{"LATCH"};juce::ComboBox presets,texture,drone,root;juce::Label status,info,rootLabel,droneLabel;
 std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> latchLink;
 std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> textureLink,droneLink;
 std::unique_ptr<juce::FileChooser> chooser;int pendingSelection=-1;juce::String importError;
 const juce::StringArray names{"C","C#","D","Eb","E","F","F#","G","Ab","A","Bb","B"};
public:
 AbideEditor(AbideProcessor&proc):AudioProcessorEditor(proc),p(proc){
  makeScene();
  setLookAndFeel(&skin);
  for(int i=0;i<12;++i){auto&b=keys[i];b.setButtonText(names[i]);b.onClick=[this,i]{p.command=48+i;};addAndMakeVisible(b);}
  const char* ids[]={"gain","tone","motion","width","attack","release","reverb","size","delay","time","feedback"};const char* titles[]={"LEVEL","TONE","MOTION","WIDTH","ATTACK / s","RELEASE / s","REVERB","SIZE","DELAY","TIME / ms","FEEDBACK"};
  for(int i=0;i<11;++i){auto&k=knobs[i];k.setSliderStyle(i==6||i==8?juce::Slider::LinearHorizontal:juce::Slider::RotaryHorizontalVerticalDrag);k.setTextBoxStyle(i==6||i==8?juce::Slider::TextBoxRight:juce::Slider::TextBoxBelow,false,66,22);k.setColour(juce::Slider::trackColourId,juce::Colour(i==0?0xffd0b985:i<6?0xff9ec7b9:0xffa8b9cf));addAndMakeVisible(k);labels[i].setText(titles[i],juce::dontSendNotification);labels[i].setJustificationType(juce::Justification::centred);labels[i].setFont(juce::FontOptions(12.f));addAndMakeVisible(labels[i]);attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.state,ids[i],k));}
  presets.addItemList({"Still Waters","Prayer Room","Open Heaven","Morning Light"},1);presets.setTextWhenNothingSelected("Preset / custom");presets.onChange=[this]{if(presets.getSelectedId()>0){p.preset(presets.getSelectedId()-1);for(int i=0;i<4;++i)pads[i].setToggleState(i==presets.getSelectedId()-1,juce::dontSendNotification);}};addAndMakeVisible(presets);
  const char* padNames[]={"STILL WATERS","PRAYER ROOM","OPEN HEAVEN","MORNING LIGHT"};for(int i=0;i<4;++i){pads[i].setButtonText(padNames[i]);pads[i].onClick=[this,i]{presets.setSelectedId(i+1,juce::sendNotificationSync);};addAndMakeVisible(pads[i]);}
  texture.addItemList({"Velvet","Radiance","Halo"},1);addAndMakeVisible(texture);textureLink=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.state,"colour",texture);
  drone.addItem("Built-in synthesizer",1);for(int i=0;i<SampleLibrary::count;++i)drone.addItem(i<SampleLibrary::factoryCount?juce::String("Loading..."):"Imported "+juce::String(i-SampleLibrary::factoryCount+1),i+2);addAndMakeVisible(drone);droneLink=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.state,"drone",drone);
  root.addItemList(names,1);root.setTooltip("The original key of the imported recording. Match this to the file before choosing a playback key.");root.onChange=[this]{int slot=int(p.get("drone"))-1;if(slot>=SampleLibrary::factoryCount)p.library.setRoot(slot,root.getSelectedId()-1);};addAndMakeVisible(root);
  rootLabel.setText("SAMPLE ROOT",juce::dontSendNotification);rootLabel.setFont(juce::FontOptions(11.f));addAndMakeVisible(rootLabel);
  droneLabel.setText("DRONE SOUND",juce::dontSendNotification);droneLabel.setFont(juce::FontOptions(11.f));addAndMakeVisible(droneLabel);
  importButton.onClick=[this]{chooseFiles();};addAndMakeVisible(importButton);pixabay.onClick=[]{juce::URL("https://github.com/DeandreEsdaille/Abide/blob/main/SAMPLE-CREDITS.md").launchInDefaultBrowser();};addAndMakeVisible(pixabay);
  latchLink=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.state,"latch",latch);addAndMakeVisible(latch);stop.onClick=[this]{p.command=-2;};panic.onClick=[this]{p.command=-3;};addAndMakeVisible(stop);addAndMakeVisible(panic);addAndMakeVisible(status);addAndMakeVisible(info);status.setJustificationType(juce::Justification::centred);status.setFont(juce::FontOptions(13.f));info.setFont(juce::FontOptions(12.f));setSize(1120,800);timerCallback();startTimerHz(10);
 }
 ~AbideEditor()override{setLookAndFeel(nullptr);}
 void chooseFiles(){
  chooser=std::make_unique<juce::FileChooser>("Import drone recordings (up to 8 sounds)",juce::File{},"*.wav;*.mp3;*.flac;*.aiff;*.aif");
  const juce::Component::SafePointer<AbideEditor> safe(this);
  chooser->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles|juce::FileBrowserComponent::canSelectMultipleItems,[safe](const juce::FileChooser&fc){if(!safe)return;safe->importError.clear();for(auto file:fc.getResults()){int slot=safe->p.library.importFile(file);if(slot<0){safe->importError="All 8 slots are full. Open a new instance to import more.";break;}if(safe->pendingSelection<0)safe->pendingSelection=slot;}});
 }
 void timerCallback()override{
  for(int i=0;i<SampleLibrary::count;++i){auto d=p.library.describe(i);drone.changeItemText(i+2,d.name.isEmpty()?"Imported "+juce::String(i-SampleLibrary::factoryCount+1):d.name+(d.loading?" (loading)":""));drone.setItemEnabled(i+2,d.ready);}
  if(pendingSelection>=0){auto d=p.library.describe(pendingSelection);if(!d.loading){if(d.ready)p.set("drone",float(pendingSelection+1));else importError=d.error;pendingSelection=-1;}}
  int selected=int(p.get("drone"))-1;root.setEnabled(selected>=SampleLibrary::factoryCount);texture.setEnabled(selected<0);root.setSelectedId(selected>=0?p.library.root(selected)+1:1,juce::dontSendNotification);
  drone.setSelectedId(selected+2,juce::dontSendNotification);
  juce::String message="Choose Heavenly Choir or Cloud Drift, or import your own recordings.";
  if(selected>=0){auto d=p.library.describe(selected);message=d.loading?"Loading "+d.name+"...":d.error.isNotEmpty()?d.error:d.ready?d.name+(selected<SampleLibrary::factoryCount?" | Factory recording, tuned to C.":" | Set SAMPLE ROOT to the recording's original key."):"Sample unavailable. Import the audio file again.";}
  if(p.library.isLoading())message="Preparing your drone recording...";if(importError.isNotEmpty())message=importError;info.setText(message,juce::dontSendNotification);info.setTooltip(message);
  int note=(p.shownNote.load()%12+12)%12;for(int i=0;i<12;++i)keys[i].setToggleState(p.sounding&&i==note,juce::dontSendNotification);status.setText(p.sounding?names[note]+" / DRONE ACTIVE":"Choose a key / MIDI ready",juce::dontSendNotification);
 }
 void makeScene(){
  art=juce::Image(juce::Image::RGB,1120,486,true);juce::Graphics g(art);
  g.fillAll(juce::Colour(0xff080410));
  g.setGradientFill(juce::ColourGradient(juce::Colour(0xff4b176c),570,210,juce::Colour(0xff080410),1000,440,true));g.fillAll();
  juce::Random random(4269);
  for(int i=0;i<700;++i){float x=random.nextFloat()*1120,y=60+random.nextFloat()*420;g.setColour(juce::Colour(0xffc1a1ec).withAlpha(random.nextFloat()*.22f));g.fillEllipse(x,y,1,1);}
  // A tilted constellation of elliptical trails, rendered once for low CPU use.
  for(int ring=0;ring<5;++ring){float radius=116.f+ring*29.f;
   juce::Path path;
   for(int i=0;i<=380;++i){float a=i/380.f*juce::MathConstants<float>::twoPi;float r=radius+4*std::sin(a*3+ring);float xx=std::cos(a)*r,yy=std::sin(a)*r*.68f;float x=560+xx*.94f-yy*.32f,y=228+xx*.32f+yy*.94f;if(i==0)path.startNewSubPath(x,y);else path.lineTo(x,y);}
   g.setColour(juce::Colour(0xffa76bff).withAlpha(.08f));g.strokePath(path,juce::PathStrokeType(13));g.setColour(juce::Colour(0xffb77bff).withAlpha(.14f));g.strokePath(path,juce::PathStrokeType(4));g.setColour(juce::Colour(0xffdab1ff).withAlpha(.35f));g.strokePath(path,juce::PathStrokeType(.7f));
   for(int i=0;i<900;++i){float a=random.nextFloat()*juce::MathConstants<float>::twoPi;float r=radius+4*std::sin(a*3+ring)+(random.nextFloat()-.5f)*9;float xx=std::cos(a)*r,yy=std::sin(a)*r*.68f;float x=560+xx*.94f-yy*.32f,y=228+xx*.32f+yy*.94f;float size=.5f+random.nextFloat()*1.7f;g.setColour(juce::Colour(0xfff0caff).withAlpha(.15f+random.nextFloat()*.7f));g.fillEllipse(x,y,size,size);}
  }
 }
 void paint(juce::Graphics&g)override{
  g.fillAll(juce::Colour(0xff08060d));g.drawImageAt(art,0,0);
  auto frame=juce::Rectangle<float>(7,7,1106,786);g.setColour(juce::Colour(0xff735d84));g.drawRoundedRectangle(frame,12,2);g.setColour(juce::Colour(0xff20132e));g.drawRoundedRectangle(frame.reduced(3),10,2);
  panel(g,{18,16,1084,48});
  caption(g,"GIMME THAT JUICE",38,29,210);g.setColour(juce::Colour(0xffdcb8ff));g.setFont(juce::FontOptions(26.f));g.drawText("A B I D E",875,24,202,30,juce::Justification::right);
  caption(g,"D R O N E   C O L L E C T I O N",37,101,230);caption(g,"SOUND DESIGN",908,101,176);
  caption(g,"TEXTURE",908,128,180);caption(g,"SAMPLE ROOT",908,204,176);
  g.setColour(juce::Colour(0xffe3c9ff));g.setFont(juce::FontOptions(13.f));g.drawText("ONE NOTE. INFINITE SPACE.",330,98,460,24,juce::Justification::centred);
  caption(g,"CHOOSE YOUR ATMOSPHERE",360,345,400);
  panel(g,{18,480,1084,302});
  g.setColour(juce::Colour(0xff5a426e));g.drawHorizontalLine(481,28,1092);
  caption(g,"VOLUME",55,698,140);caption(g,"TONE",925,698,140);
  g.setColour(juce::Colour(0xff766484));g.drawHorizontalLine(634,238,882);
  caption(g,"SHAPE & SPACE",242,637,300);
  g.setColour(juce::Colour(0xff84728f));g.setFont(juce::FontOptions(10.f));g.drawText("ABIDE / 0.4     ·     GIMME THAT JUICE",38,765,450,16,juce::Justification::left);
 }
 void caption(juce::Graphics&g,const juce::String&text,int x,int y,int w){g.setColour(juce::Colour(0xffc0a6d2));g.setFont(juce::FontOptions(11.f));g.drawText(text,x,y,w,22,juce::Justification::centred);}
 void panel(juce::Graphics&g,juce::Rectangle<int> bounds){auto r=bounds.toFloat();g.setGradientFill(juce::ColourGradient(juce::Colour(0xff29232f),0,r.getY(),juce::Colour(0xff0c0a10),0,r.getBottom(),false));g.fillRoundedRectangle(r,9);g.setColour(juce::Colour(0xff4c3b58));g.drawRoundedRectangle(r,9,1);g.setColour(juce::Colours::white.withAlpha(.035f));g.fillRoundedRectangle(r.withHeight(r.getHeight()*.43f).reduced(2),7);}
 void resized()override{
  presets.setBounds(370,24,380,32);droneLabel.setBounds(38,128,196,22);drone.setBounds(38,154,196,34);importButton.setBounds(38,202,196,34);
  texture.setBounds(908,154,174,34);rootLabel.setBounds(908,204,0,0);root.setBounds(950,232,88,32);latch.setBounds(931,289,130,30);
  info.setBounds(38,425,1044,34);status.setBounds(355,188,410,28);
  const int whites[]={0,2,4,5,7,9,11};for(int i=0;i<7;++i)keys[whites[i]].setBounds(262+i*86,260,82,68);
  const int blacks[]={1,3,6,8,10},positions[]={1,2,4,5,6};for(int i=0;i<5;++i){keys[blacks[i]].setBounds(262+positions[i]*86-22,236,42,51);keys[blacks[i]].toFront(false);}
  for(int i=0;i<4;++i)pads[i].setBounds(270+i*148,375,136,58);
  for(int i: {0,1}){knobs[i].setBounds(i==0?46:914,514,160,176);labels[i].setBounds(0,0,0,0);}
  labels[6].setBounds(258,520,90,24);knobs[6].setBounds(354,510,482,46);labels[8].setBounds(258,578,90,24);knobs[8].setBounds(354,568,482,46);
  const int small[]={2,3,4,5,7,9,10};for(int i=0;i<7;++i){int x=236+i*94;labels[small[i]].setBounds(x,660,90,20);knobs[small[i]].setBounds(x+4,681,82,77);}
  stop.setBounds(38,733,165,29);panic.setBounds(918,733,164,29);pixabay.setBounds(914,445,168,26);
 }
};
juce::AudioProcessorEditor* AbideProcessor::createEditor(){return new AbideEditor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new AbideProcessor;}
