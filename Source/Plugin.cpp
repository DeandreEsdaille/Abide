#include "PluginProcessor.h"
#include "BinaryData.h"
void AbideProcessor::initializeFactorySounds(){library.addFactory(0,BinaryData::HeavenlyChoir_flac,BinaryData::HeavenlyChoir_flacSize,"Heavenly Choir");library.addFactory(1,BinaryData::CloudDrift_flac,BinaryData::CloudDrift_flacSize,"Cloud Drift");}
class AbideEditor : public juce::AudioProcessorEditor, private juce::Timer {
 AbideProcessor&p;juce::Image art;juce::LookAndFeel_V4 skin;
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
  skin.setColour(juce::Slider::rotarySliderFillColourId,juce::Colour(0xffb9d8ce));skin.setColour(juce::Slider::thumbColourId,juce::Colours::white);skin.setColour(juce::Slider::textBoxTextColourId,juce::Colours::white);skin.setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);skin.setColour(juce::TextButton::buttonColourId,juce::Colour(0xff1b2c39));skin.setColour(juce::TextButton::buttonOnColourId,juce::Colour(0xff527c79));setLookAndFeel(&skin);
  for(int i=0;i<12;++i){auto&b=keys[i];b.setButtonText(names[i]);b.onClick=[this,i]{p.command=48+i;};addAndMakeVisible(b);}
  const char* ids[]={"gain","tone","motion","width","attack","release","reverb","size","delay","time","feedback"};const char* titles[]={"VOLUME","TONE","MOVEMENT","WIDTH","ATTACK (s)","RELEASE (s)","REVERB","ROOM SIZE","DELAY","TIME (ms)","FEEDBACK"};
  for(int i=0;i<11;++i){auto&k=knobs[i];k.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);k.setTextBoxStyle(juce::Slider::TextBoxBelow,false,80,20);addAndMakeVisible(k);labels[i].setText(titles[i],juce::dontSendNotification);labels[i].setJustificationType(juce::Justification::centred);labels[i].setFont(juce::FontOptions(12.f));addAndMakeVisible(labels[i]);attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.state,ids[i],k));}
  presets.addItemList({"Still Waters","Prayer Room","Open Heaven","Morning Light"},1);presets.setTextWhenNothingSelected("Preset / custom");presets.onChange=[this]{if(presets.getSelectedId()>0)p.preset(presets.getSelectedId()-1);};addAndMakeVisible(presets);
  texture.addItemList({"Velvet","Radiance","Halo"},1);addAndMakeVisible(texture);textureLink=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.state,"colour",texture);
  drone.addItem("Built-in synthesizer",1);for(int i=0;i<SampleLibrary::count;++i)drone.addItem(i<SampleLibrary::factoryCount?juce::String("Loading..."):"Imported "+juce::String(i-SampleLibrary::factoryCount+1),i+2);addAndMakeVisible(drone);droneLink=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.state,"drone",drone);
  root.addItemList(names,1);root.setTooltip("The original key of the imported recording. Match this to the file before choosing a playback key.");root.onChange=[this]{int slot=int(p.get("drone"))-1;if(slot>=SampleLibrary::factoryCount)p.library.setRoot(slot,root.getSelectedId()-1);};addAndMakeVisible(root);
  rootLabel.setText("SAMPLE ROOT",juce::dontSendNotification);rootLabel.setFont(juce::FontOptions(11.f));addAndMakeVisible(rootLabel);
  droneLabel.setText("DRONE SOUND",juce::dontSendNotification);droneLabel.setFont(juce::FontOptions(11.f));addAndMakeVisible(droneLabel);
  importButton.onClick=[this]{chooseFiles();};addAndMakeVisible(importButton);pixabay.onClick=[]{juce::URL("https://github.com/DeandreEsdaille/Abide/blob/main/SAMPLE-CREDITS.md").launchInDefaultBrowser();};addAndMakeVisible(pixabay);
  latchLink=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.state,"latch",latch);addAndMakeVisible(latch);stop.onClick=[this]{p.command=-2;};panic.onClick=[this]{p.command=-3;};addAndMakeVisible(stop);addAndMakeVisible(panic);addAndMakeVisible(status);addAndMakeVisible(info);status.setJustificationType(juce::Justification::centred);status.setFont(juce::FontOptions(16.f));info.setFont(juce::FontOptions(12.f));setSize(1040,800);timerCallback();startTimerHz(10);
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
  g.fillAll(juce::Colour(0xff101a27));if(art.isValid())g.drawImage(art,getLocalBounds().toFloat(),juce::RectanglePlacement::fillDestination);g.setColour(juce::Colour(0xff081421).withAlpha(.62f));g.fillAll();g.setColour(juce::Colours::white);g.setFont(juce::FontOptions(48.f));g.drawText("ABIDE",40,28,400,60,juce::Justification::left);g.setFont(juce::FontOptions(12.f));g.drawText("G I M M E  T H A T  J U I C E  /  A T M O S P H E R I C  P A D S",43,90,700,25,juce::Justification::left);g.setColour(juce::Colour(0xff081421).withAlpha(.78f));g.fillRoundedRectangle(30,290,980,438,20);g.setColour(juce::Colours::white.withAlpha(.55f));g.setFont(juce::FontOptions(12.f));g.drawText("One note. Room to worship.     |     Latch holds the drone after MIDI release.",40,758,960,22,juce::Justification::centred);
 }
 void resized()override{
  presets.setBounds(770,40,230,32);droneLabel.setBounds(648,88,115,32);drone.setBounds(770,88,230,32);texture.setBounds(770,140,230,32);importButton.setBounds(40,140,170,32);rootLabel.setBounds(233,140,92,32);root.setBounds(327,140,82,32);info.setBounds(40,181,650,30);status.setBounds(694,181,306,30);
  for(int i=0;i<12;++i)keys[i].setBounds(40+i*80,224,72,48);
  for(int i=0;i<11;++i){int row=i<6?0:1,col=i<6?i:i-6;int x=48+col*158;labels[i].setBounds(x,312+row*158,150,24);knobs[i].setBounds(x+13,339+row*158,124,122);}
  latch.setBounds(850,507,120,28);stop.setBounds(847,550,130,36);panic.setBounds(847,599,130,30);pixabay.setBounds(835,650,154,30);
 }
};
juce::AudioProcessorEditor* AbideProcessor::createEditor(){return new AbideEditor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new AbideProcessor;}
