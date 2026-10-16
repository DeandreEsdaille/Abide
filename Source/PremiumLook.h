#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class PremiumLook : public juce::LookAndFeel_V4 {
public:
 PremiumLook(){
  setColour(juce::Slider::textBoxTextColourId,juce::Colour(0xffeee5ff));
  setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);
  setColour(juce::Slider::textBoxBackgroundColourId,juce::Colour(0xff100d19));
  setColour(juce::ComboBox::backgroundColourId,juce::Colour(0xff191125));
  setColour(juce::ComboBox::outlineColourId,juce::Colour(0xff614575));
  setColour(juce::ComboBox::textColourId,juce::Colour(0xfff1e9ff));
  setColour(juce::PopupMenu::backgroundColourId,juce::Colour(0xff170e24));
  setColour(juce::PopupMenu::highlightedBackgroundColourId,juce::Colour(0xff593478));
  setColour(juce::TextButton::textColourOffId,juce::Colour(0xffdfd0ef));
  setColour(juce::TextButton::textColourOnId,juce::Colour(0xfff4e8ff));
  setColour(juce::ToggleButton::textColourId,juce::Colour(0xffdfd0ef));
  setColour(juce::ToggleButton::tickColourId,juce::Colour(0xffcd9bff));
 }
 void drawLinearSlider(juce::Graphics&g,int x,int y,int width,int height,float pos,float,float,
                       const juce::Slider::SliderStyle style,juce::Slider&s)override{
  if(style==juce::Slider::LinearHorizontal){
   float cy=y+height*.5f;juce::Rectangle<float> rail(float(x),cy-6,float(width),12);
   g.setColour(juce::Colour(0xff020105));g.fillRoundedRectangle(rail.expanded(3),9);
   g.setColour(juce::Colour(0xff44374e));g.drawRoundedRectangle(rail.expanded(2),8,1);
   g.setGradientFill(juce::ColourGradient(juce::Colour(0xff100b17),0,cy-6,juce::Colour(0xff292032),0,cy+6,false));g.fillRoundedRectangle(rail,6);
   float fill=juce::jlimit(1.f,float(width),pos-x);g.setColour(juce::Colour(0xffad64f2).withAlpha(.18f));g.fillRoundedRectangle(float(x),cy-10,fill,20,9);
   g.setGradientFill(juce::ColourGradient(juce::Colour(0xff9b55d9),float(x),cy,juce::Colour(0xffebc7ff),pos,cy,false));g.fillRoundedRectangle(float(x),cy-2.5f,fill,5,2.5f);
   g.setColour(juce::Colour(0xfff1ddff));g.fillRoundedRectangle(pos-3,cy-5,6,10,3);return;
  }
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
  auto top=juce::Colour(active?0xff78469b:0xff3c254f), bottom=juce::Colour(active?0xff3c175c:0xff180e25);
  if(hover){top=top.brighter(.15f);bottom=bottom.brighter(.12f);}if(down)bottom=bottom.darker(.25f);
  g.setGradientFill(juce::ColourGradient(top,0,r.getY(),bottom,0,r.getBottom(),false));g.fillRoundedRectangle(r,5);
  g.setColour(juce::Colour(active?0xffe2bcff:0xff73518a));g.drawRoundedRectangle(r,5,1);
  g.setColour(juce::Colours::white.withAlpha(active?.14f:.08f));g.drawHorizontalLine(int(r.getY()+2),r.getX()+5,r.getRight()-5);
  if(active){g.setColour(juce::Colour(0xffe3b8ff));g.fillRoundedRectangle(r.getCentreX()-10,r.getBottom()-6,20,2,1);}
 }
 void drawRotarySlider(juce::Graphics&g,int x,int y,int w,int h,float value,float start,float end,juce::Slider&)override{
  float diameter=juce::jmin(float(w),float(h))-12.f;auto r=juce::Rectangle<float>(diameter,diameter).withCentre({x+w*.5f,y+h*.5f});auto c=r.getCentre();float rad=diameter*.5f;
  g.setColour(juce::Colours::black.withAlpha(.75f));g.fillEllipse(r.expanded(4).translated(0,5));
  g.setGradientFill(juce::ColourGradient(juce::Colour(0xff78717e),0,r.getY(),juce::Colour(0xff100d15),0,r.getBottom(),false));g.fillEllipse(r);
  g.setColour(juce::Colour(0xff07060b));g.fillEllipse(r.reduced(4));
  juce::Path track;track.addCentredArc(c.x,c.y,rad-10,rad-10,0,start,end,true);g.setColour(juce::Colour(0xff39303f));g.strokePath(track,juce::PathStrokeType(3));
  juce::Path arc;arc.addCentredArc(c.x,c.y,rad-10,rad-10,0,start,start+value*(end-start),true);g.setColour(juce::Colour(0xffd7a6ff));g.strokePath(arc,juce::PathStrokeType(3));
  auto face=r.reduced(diameter*.19f);g.setGradientFill(juce::ColourGradient(juce::Colour(0xff514957),face.getX(),face.getY(),juce::Colour(0xff151019),face.getRight(),face.getBottom(),false));g.fillEllipse(face);
  g.setColour(juce::Colour(0xff786b82).withAlpha(.5f));g.drawEllipse(face,1);auto a=start+value*(end-start);auto tip=c+juce::Point<float>(std::sin(a),-std::cos(a))*(rad*.47f);auto base=c+juce::Point<float>(std::sin(a),-std::cos(a))*(rad*.23f);
  g.setColour(juce::Colour(0xffeed7ff));g.drawLine({base,tip},diameter>100?4.f:2.5f);
 }
 juce::Font getTextButtonFont(juce::TextButton&,int)override{return juce::Font(juce::FontOptions(12.f));}
};
