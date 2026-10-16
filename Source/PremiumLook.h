#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class PremiumLook : public juce::LookAndFeel_V4 {
public:
 PremiumLook(){
  setColour(juce::Slider::textBoxTextColourId,juce::Colour(0xffe2e9e8));
  setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);
  setColour(juce::Slider::textBoxBackgroundColourId,juce::Colour(0xff111b20));
  setColour(juce::ComboBox::backgroundColourId,juce::Colour(0xff152329));
  setColour(juce::ComboBox::outlineColourId,juce::Colour(0xff405354));
  setColour(juce::ComboBox::textColourId,juce::Colour(0xffedf1ed));
  setColour(juce::PopupMenu::backgroundColourId,juce::Colour(0xff142126));
  setColour(juce::PopupMenu::highlightedBackgroundColourId,juce::Colour(0xff355953));
  setColour(juce::TextButton::textColourOffId,juce::Colour(0xffd7e1de));
  setColour(juce::TextButton::textColourOnId,juce::Colour(0xfff3e4bc));
  setColour(juce::ToggleButton::textColourId,juce::Colour(0xffd7e1de));
  setColour(juce::ToggleButton::tickColourId,juce::Colour(0xffc9b685));
 }
 void drawLinearSlider(juce::Graphics&g,int x,int y,int width,int height,float pos,float,float,
                       const juce::Slider::SliderStyle,juce::Slider&s)override{
  const float cx=x+width*.5f, top=float(y)+8.f, bottom=float(y+height)-8.f;
  const auto accent=s.findColour(juce::Slider::trackColourId);
  // Engraved graduations and a recessed, softly illuminated rail.
  for(int i=0;i<=10;++i){float yy=top+(bottom-top)*i/10.f;float len=i%5==0?13.f:7.f;
   g.setColour(juce::Colour(0xff50605f).withAlpha(i%5==0?.85f:.45f));
   g.drawHorizontalLine(int(yy),cx-25.f-len,cx-25.f);g.drawHorizontalLine(int(yy),cx+25.f,cx+25.f+len);
  }
  g.setColour(juce::Colour(0xff03090c));g.fillRoundedRectangle(cx-5,top-3,10,bottom-top+6,5);
  g.setColour(juce::Colour(0xff344348));g.drawRoundedRectangle(cx-5,top-3,10,bottom-top+6,5,1);
  float capY=juce::jlimit(top,bottom,pos);
  g.setColour(accent.withAlpha(.12f));g.fillRoundedRectangle(cx-7,capY,14,juce::jmax(1.f,bottom-capY),6);
  g.setColour(accent.withAlpha(.75f));g.fillRoundedRectangle(cx-1.5f,capY,3,juce::jmax(1.f,bottom-capY),1.5f);
  juce::Rectangle<float> cap(cx-24,capY-13,48,26);
  g.setColour(juce::Colours::black.withAlpha(.55f));g.fillRoundedRectangle(cap.translated(0,4).expanded(2),4);
  g.setGradientFill(juce::ColourGradient(juce::Colour(0xffa4aead),0,cap.getY(),juce::Colour(0xff38464b),0,cap.getBottom(),false));
  g.fillRoundedRectangle(cap,3);g.setColour(juce::Colour(0xffc1cac5).withAlpha(.55f));g.drawRoundedRectangle(cap.reduced(.5f),3,1);
  for(int i=-1;i<=1;++i){g.setColour(juce::Colours::black.withAlpha(.35f));g.drawHorizontalLine(int(capY+i*4),cx-16,cx+16);}
  g.setColour(accent);g.fillRoundedRectangle(cx-18,capY-1,36,2,1);
 }
 void drawButtonBackground(juce::Graphics&g,juce::Button&b,const juce::Colour&,bool hover,bool down)override{
  auto r=b.getLocalBounds().toFloat().reduced(1.f);bool active=b.getToggleState();
  r.removeFromBottom(3.f);
  g.setColour(juce::Colour(0xff020608));g.fillRoundedRectangle(r.translated(0,3),5);
  if(down)r=r.translated(0,2);
  auto top=juce::Colour(active?0xff355e56:0xff25363c), bottom=juce::Colour(active?0xff1c3934:0xff121e24);
  if(hover){top=top.brighter(.15f);bottom=bottom.brighter(.12f);}if(down)bottom=bottom.darker(.25f);
  g.setGradientFill(juce::ColourGradient(top,0,r.getY(),bottom,0,r.getBottom(),false));g.fillRoundedRectangle(r,5);
  g.setColour(juce::Colour(active?0xffbda778:0xff47585b));g.drawRoundedRectangle(r,5,1);
  g.setColour(juce::Colours::white.withAlpha(active?.14f:.08f));g.drawHorizontalLine(int(r.getY()+2),r.getX()+5,r.getRight()-5);
  if(active){g.setColour(juce::Colour(0xffd4be89));g.fillRoundedRectangle(r.getCentreX()-10,r.getBottom()-6,20,2,1);}
 }
 juce::Font getTextButtonFont(juce::TextButton&,int)override{return juce::Font(juce::FontOptions(12.f));}
};
