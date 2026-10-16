#include "PluginProcessor.h"
#include "BinaryData.h"
#include "PremiumLook.h"
void AbideProcessor::initializeFactorySounds(){library.addFactory(0,BinaryData::HeavenlyChoir_flac,BinaryData::HeavenlyChoir_flacSize,"Heavenly Choir");library.addFactory(1,BinaryData::CloudDrift_flac,BinaryData::CloudDrift_flacSize,"Cloud Drift");}
class AbideEditor : public juce::AudioProcessorEditor, private juce::Timer {
 AbideProcessor&p;juce::Image art;PremiumLook skin;
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
  art=juce::ImageCache::getFromMemory(BinaryData::atmosphere_jpg,BinaryData::atmosphere_jpgSize);
  setLookAndFeel(&skin);
  for(int i=0;i<12;++i){auto&b=keys[i];b.setButtonText(names[i]);b.onClick=[this,i]{p.command=48+i;};addAndMakeVisible(b);}
  const char* ids[]={"gain","tone","motion","width","attack","release","reverb","size","delay","time","feedback"};const char* titles[]={"LEVEL","TONE","MOTION","WIDTH","ATTACK / s","RELEASE / s","REVERB","SIZE","DELAY","TIME / ms","FEEDBACK"};
  for(int i=0;i<11;++i){auto&k=knobs[i];k.setSliderStyle(juce::Slider::LinearVertical);k.setTextBoxStyle(juce::Slider::TextBoxBelow,false,66,24);k.setColour(juce::Slider::trackColourId,juce::Colour(i==0?0xffd0b985:i<6?0xff9ec7b9:0xffa8b9cf));addAndMakeVisible(k);labels[i].setText(titles[i],juce::dontSendNotification);labels[i].setJustificationType(juce::Justification::centred);labels[i].setFont(juce::FontOptions(12.f));addAndMakeVisible(labels[i]);attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.state,ids[i],k));}
  presets.addItemList({"Still Waters","Prayer Room","Open Heaven","Morning Light"},1);presets.setTextWhenNothingSelected("Preset / custom");presets.onChange=[this]{if(presets.getSelectedId()>0)p.preset(presets.getSelectedId()-1);};addAndMakeVisible(presets);
  texture.addItemList({"Velvet","Radiance","Halo"},1);addAndMakeVisible(texture);textureLink=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.state,"colour",texture);
  drone.addItem("Built-in synthesizer",1);for(int i=0;i<SampleLibrary::count;++i)drone.addItem(i<SampleLibrary::factoryCount?juce::String("Loading..."):"Imported "+juce::String(i-SampleLibrary::factoryCount+1),i+2);addAndMakeVisible(drone);droneLink=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.state,"drone",drone);
  root.addItemList(names,1);root.setTooltip("The original key of the imported recording. Match this to the file before choosing a playback key.");root.onChange=[this]{int slot=int(p.get("drone"))-1;if(slot>=SampleLibrary::factoryCount)p.library.setRoot(slot,root.getSelectedId()-1);};addAndMakeVisible(root);
  rootLabel.setText("SAMPLE ROOT",juce::dontSendNotification);rootLabel.setFont(juce::FontOptions(11.f));addAndMakeVisible(rootLabel);
  droneLabel.setText("DRONE SOUND",juce::dontSendNotification);droneLabel.setFont(juce::FontOptions(11.f));addAndMakeVisible(droneLabel);
  importButton.onClick=[this]{chooseFiles();};addAndMakeVisible(importButton);pixabay.onClick=[]{juce::URL("https://github.com/DeandreEsdaille/Abide/blob/main/SAMPLE-CREDITS.md").launchInDefaultBrowser();};addAndMakeVisible(pixabay);
  latchLink=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.state,"latch",latch);addAndMakeVisible(latch);stop.onClick=[this]{p.command=-2;};panic.onClick=[this]{p.command=-3;};addAndMakeVisible(stop);addAndMakeVisible(panic);addAndMakeVisible(status);addAndMakeVisible(info);status.setJustificationType(juce::Justification::centred);status.setFont(juce::FontOptions(13.f));info.setFont(juce::FontOptions(12.f));setSize(1120,780);timerCallback();startTimerHz(10);
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
 void paint(juce::Graphics&g)override{
  g.fillAll(juce::Colour(0xff0b1217));
  if(art.isValid())g.drawImage(art,juce::Rectangle<float>(0,0,1120,355),juce::RectanglePlacement::fillDestination);
  g.setGradientFill(juce::ColourGradient(juce::Colour(0xff0b151c).withAlpha(.24f),0,0,juce::Colour(0xff0b1217),0,350,false));g.fillRect(0,0,1120,355);
  g.setColour(juce::Colour(0xffd4bf8f));g.setFont(juce::FontOptions(11.f));g.drawText("G I M M E  T H A T  J U I C E",40,22,400,22,juce::Justification::left);
  g.setColour(juce::Colour(0xfff0f2ec));g.setFont(juce::FontOptions(57.f));g.drawText("A B I D E",36,45,450,74,juce::Justification::left);
  g.setColour(juce::Colour(0xffadbfbd));g.setFont(juce::FontOptions(12.f));g.drawText("A T M O S P H E R I C   I N S T R U M E N T",40,118,400,24,juce::Justification::left);
  panel(g,{364,112,728,112});
  caption(g,"TEXTURE",690,122,150);caption(g,"PRESET",820,20,250);
  caption(g,"PLAY / ROOT KEY",40,238,300);
  panel(g,{28,354,456,342});panel(g,{498,354,456,342});panel(g,{968,354,124,342});
  caption(g,"01   /   SCULPT",48,366,250);caption(g,"02   /   SPACE",518,366,250);caption(g,"MASTER",986,366,100);
  g.setColour(juce::Colour(0xff314145));g.drawHorizontalLine(397,48,464);g.drawHorizontalLine(397,518,934);g.drawHorizontalLine(397,986,1074);
  // Machined enclosure edges and corner fasteners.
  for(auto pt: {juce::Point<float>(16,16),juce::Point<float>(1104,16),juce::Point<float>(16,764),juce::Point<float>(1104,764)}){
   g.setColour(juce::Colour(0xff03090c));g.fillEllipse(pt.x-4,pt.y-3,8,8);g.setColour(juce::Colour(0xff566066));g.drawEllipse(pt.x-4,pt.y-4,8,8,1);g.drawLine(pt.x-2,pt.y+1,pt.x+2,pt.y-2,1);
  }
  g.setColour(juce::Colour(0xff29383d));g.drawRect(getLocalBounds().reduced(1),1);
  g.setColour(juce::Colour(0xff7c9090));g.setFont(juce::FontOptions(11.f));g.drawText("One note. Room to worship.",40,747,400,20,juce::Justification::left);
  g.drawText("GIMME THAT JUICE   /   ABIDE 0.3",720,747,360,20,juce::Justification::right);
 }
 void caption(juce::Graphics&g,const juce::String&text,int x,int y,int w){g.setColour(juce::Colour(0xffbcae8c));g.setFont(juce::FontOptions(11.f));g.drawText(text,x,y,w,22,juce::Justification::left);}
 void panel(juce::Graphics&g,juce::Rectangle<int> bounds){
  auto r=bounds.toFloat();g.setColour(juce::Colours::black.withAlpha(.5f));g.fillRoundedRectangle(r.translated(0,4),9);
  g.setGradientFill(juce::ColourGradient(juce::Colour(0xff253238).withAlpha(.94f),0,r.getY(),juce::Colour(0xff101c22),0,r.getBottom(),false));g.fillRoundedRectangle(r,9);
  g.setColour(juce::Colour(0xff465458).withAlpha(.7f));g.drawRoundedRectangle(r.reduced(.5f),9,1);
  g.setColour(juce::Colours::white.withAlpha(.035f));for(int y=bounds.getY()+3;y<bounds.getBottom()-3;y+=3)g.drawHorizontalLine(y,r.getX()+5,r.getRight()-5);
 }
 void resized()override{
  presets.setBounds(820,46,272,34);droneLabel.setBounds(384,122,286,22);drone.setBounds(384,148,286,34);texture.setBounds(690,148,174,34);
  importButton.setBounds(40,164,188,38);rootLabel.setBounds(886,122,180,22);root.setBounds(886,148,70,34);
  info.setBounds(384,188,688,26);status.setBounds(780,232,312,28);
  for(int i=0;i<12;++i)keys[i].setBounds(40+i*87,270,80,54);
  for(int i=1;i<11;++i){int x=i<6?44+(i-1)*88:514+(i-6)*88;labels[i].setBounds(x,407,80,25);knobs[i].setBounds(x,438,80,239);}
  labels[0].setBounds(990,407,80,25);knobs[0].setBounds(990,438,80,239);
  latch.setBounds(40,710,180,28);stop.setBounds(816,706,160,34);panic.setBounds(990,706,102,34);pixabay.setBounds(642,706,160,34);
 }
};
juce::AudioProcessorEditor* AbideProcessor::createEditor(){return new AbideEditor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new AbideProcessor;}
