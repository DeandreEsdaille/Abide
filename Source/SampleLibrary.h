#pragma once
#include <juce_audio_formats/juce_audio_formats.h>
#include "PadEngine.h"

// Sample buffers are immutable after publication. Ownership stays on the loader
// until processor destruction, so the audio callback never frees sample memory.
class SampleLibrary {
public:
 static constexpr int factoryCount=2;
 static constexpr int count = factoryCount+8;
 struct Description { juce::String name, path, error; bool loading=false, ready=false; int root=0; };
 explicit SampleLibrary(juce::File folder = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("Gimme That Juice/Abide/Samples")) : cache(std::move(folder)), pool(1) {}
 ~SampleLibrary() { pool.removeAllJobs(true, -1); }
 const abide::SampleData* get(int index) const { return valid(index) ? slots[index].ready.load(std::memory_order_acquire) : nullptr; }
 int root(int index) const { return valid(index) ? slots[index].root.load() : 0; }
 void setRoot(int index,int value) { if(valid(index))slots[index].root=juce::jlimit(0,11,value); }
 Description describe(int index) const { const juce::ScopedLock lock(metadata);const auto&s=slots[index];return {s.name,s.path,s.error,s.loading.load(),get(index)!=nullptr,s.root.load()}; }
 static int guessRoot(juce::String name) {
  const juce::StringArray notes{"C#","Db","D#","Eb","F#","Gb","G#","Ab","A#","Bb","C","D","E","F","G","A","B"};
  const int values[]{1,1,3,3,6,6,8,8,10,10,0,2,4,5,7,9,11};
  for(int i=0;i<notes.size();++i)if(name.equalsIgnoreCase(notes[i])||name.startsWithIgnoreCase(notes[i]+"-")||name.startsWithIgnoreCase(notes[i]+"_")||name.startsWithIgnoreCase(notes[i]+" "))return values[i];
  return 0;
 }
 int importFile(const juce::File& file) {
  for(int i=factoryCount;i<count;++i) { auto d=describe(i);if(!d.loading&&d.path==file.getFullPathName()&&d.ready)return i; }
  for(int i=factoryCount;i<count;++i)if(!get(i)&&!slots[i].loading.load()){request(i,file,file.getFileNameWithoutExtension(),guessRoot(file.getFileNameWithoutExtension()),true);return i;}
  return -1;
 }
 void addFactory(int index,const void* bytes,size_t size,juce::String name) {
  if(index<0||index>=factoryCount)return;{const juce::ScopedLock lock(metadata);slots[index].name=name;}slots[index].loading=true;
  pool.addJob(std::function<void()>([this,index,bytes,size]{juce::AudioFormatManager formats;formats.registerBasicFormats();juce::String error;std::unique_ptr<juce::AudioFormatReader> reader(formats.createReaderFor(std::make_unique<juce::MemoryInputStream>(bytes,size,false)));auto data=decode(std::move(reader),error);if(data){this->bytes+=(data->left.size()+data->right.size())*sizeof(float);auto*ptr=data.get();owned.push_back(std::move(data));slots[index].ready.store(ptr,std::memory_order_release);}{const juce::ScopedLock lock(metadata);slots[index].error=error;}slots[index].loading=false;}));
 }
 juce::ValueTree save() const {
  juce::ValueTree out("DroneLibrary");const juce::ScopedLock lock(metadata);
  for(int i=0;i<count;++i)if(slots[i].path.isNotEmpty()){juce::ValueTree item("Sample");item.setProperty("slot",i,nullptr);item.setProperty("path",slots[i].path,nullptr);item.setProperty("name",slots[i].name,nullptr);item.setProperty("root",slots[i].root.load(),nullptr);out.addChild(item,-1,nullptr);}return out;
 }
 void restore(const juce::ValueTree& saved) {
  for(auto item:saved){const int i=int(item["slot"]);if(!valid(i)||i<factoryCount)continue;const juce::String path=item["path"].toString();if(path.isEmpty())continue;auto d=describe(i);setRoot(i,int(item["root"]));if(d.path==path&&(d.ready||d.loading))continue;request(i,juce::File(path),item["name"].toString(),int(item["root"]),false);}
 }
 bool isLoading() const { for(const auto&s:slots)if(s.loading)return true;return false; }
private:
 struct Slot {std::atomic<const abide::SampleData*> ready{nullptr};std::atomic<bool> loading{false};std::atomic<int> root{0},generation{0};juce::String name,path,error;};
 std::array<Slot,count> slots;juce::File cache;juce::ThreadPool pool;mutable juce::CriticalSection metadata;
 std::vector<std::unique_ptr<abide::SampleData>> owned;size_t bytes=0;
 static bool valid(int i){return i>=0&&i<count;}
 void request(int index,juce::File file,juce::String name,int rootNote,bool copyToCache) {
  auto&s=slots[index];const int generation=++s.generation;s.loading=true;s.root=juce::jlimit(0,11,rootNote);
  {const juce::ScopedLock lock(metadata);s.name=name;s.path=file.getFullPathName();s.error.clear();}
  pool.addJob(std::function<void()>([this,index,generation,file,name,copyToCache]{
   juce::String error;auto data=read(file,error);juce::File savedFile=file;
   if(data&&copyToCache){if(!cache.createDirectory()){error="Cannot create the local sample folder.";data.reset();}else{savedFile=cache.getChildFile(juce::Uuid().toString()+".wav");auto stream=savedFile.createOutputStream();juce::WavAudioFormat format;std::unique_ptr<juce::AudioFormatWriter> writer;if(stream)writer.reset(format.createWriterFor(stream.get(),data->sampleRate,2,24,{},0));if(writer)stream.release();const float* channels[]{data->left.data(),data->right.data()};if(!writer||!writer->writeFromFloatArrays(channels,2,int(data->left.size()))){error="Could not save this drone locally.";data.reset();}}}
   if(slots[index].generation!=generation)return;
   if(data&&bytes+(data->left.size()+data->right.size())*sizeof(float)>256u*1024u*1024u){error="Sample memory is full. Open a new instance to load more sounds.";data.reset();}
   if(data){bytes+=(data->left.size()+data->right.size())*sizeof(float);auto* ptr=data.get();owned.push_back(std::move(data));{const juce::ScopedLock lock(metadata);slots[index].path=savedFile.getFullPathName();slots[index].name=name;}slots[index].ready.store(ptr,std::memory_order_release);}
   {const juce::ScopedLock lock(metadata);slots[index].error=error;}slots[index].loading=false;
  }));
 }
 static std::unique_ptr<abide::SampleData> read(const juce::File& file,juce::String& error) {
  juce::AudioFormatManager formats;formats.registerBasicFormats();return decode(std::unique_ptr<juce::AudioFormatReader>(formats.createReaderFor(file)),error);
 }
 static std::unique_ptr<abide::SampleData> decode(std::unique_ptr<juce::AudioFormatReader> reader,juce::String&error) {
  if(!reader){error="Cannot read this audio file. Choose WAV, MP3, FLAC or AIFF.";return {};}
  if(reader->sampleRate<8000||reader->sampleRate>192000||reader->numChannels<1||reader->lengthInSamples<reader->sampleRate*.25){error="Choose an audio file at least 0.25 seconds long (8-192 kHz).";return {};}
  const auto start=reader->lengthInSamples>reader->sampleRate*90?juce::int64(reader->sampleRate*5):0;
  const int frames=int(std::min(reader->lengthInSamples-start,juce::int64(reader->sampleRate*60)));
  juce::AudioBuffer<float> decoded(2,frames);if(!reader->read(&decoded,0,frames,start,true,true)){error="Audio decoding failed.";return {};}
  if(reader->numChannels==1)decoded.copyFrom(1,0,decoded,0,0,frames);
  auto data=std::make_unique<abide::SampleData>();data->sampleRate=32000;const int size=int(frames*data->sampleRate/reader->sampleRate);data->left.resize(size);data->right.resize(size);
  float peak=0;for(int i=0;i<size;++i){double pos=i*reader->sampleRate/data->sampleRate;const int a=std::min(int(pos),frames-1),b=std::min(a+1,frames-1);const float frac=float(pos-a);for(int ch=0;ch<2;++ch){float value=decoded.getSample(ch,a)*(1-frac)+decoded.getSample(ch,b)*frac;if(!std::isfinite(value)){error="The audio contains invalid samples.";return {};}auto&out=ch==0?data->left:data->right;out[i]=value;peak=std::max(peak,std::abs(value));}}
  if(peak<.000001f){error="This audio file is silent.";return {};}
  const float scale=.4f/peak;for(auto*channel:{&data->left,&data->right})for(auto&v:*channel)v*=scale;
  return data;
 }
};
