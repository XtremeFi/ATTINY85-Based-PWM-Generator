from pathlib import Path
from html import escape
import cairosvg

root=Path(__file__).parent
parts=['''<svg xmlns="http://www.w3.org/2000/svg" width="1600" height="1500" viewBox="0 0 1600 1500"><style>text{font-family:DejaVu Sans,sans-serif;fill:#163047;font-size:20px} .small{font-size:17px} .title{font-size:30px;font-weight:bold} .head{font-size:24px;font-weight:bold} path,line,rect,circle{stroke:#244963;stroke-width:2.5;fill:none} .wire{stroke:#167c82;stroke-width:3} .node{fill:#167c82;stroke:none}</style><rect x="0" y="0" width="1600" height="1500" style="fill:#fff;stroke:none"/>''']
def text(x,y,s,cls='',anchor='start'):
 parts.append(f'<text x="{x}" y="{y}" class="{cls}" text-anchor="{anchor}">{escape(s)}</text>')
def line(x1,y1,x2,y2):
 parts.append(f'<line class="wire" x1="{x1}" y1="{y1}" x2="{x2}" y2="{y2}"/>')
def box(x,y,w,h):parts.append(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="4"/>')
def dot(x,y):parts.append(f'<circle class="node" cx="{x}" cy="{y}" r="5"/>')
def ground(x,y):
 line(x,y,x,y+12)
 for i,w in enumerate([18,12,6]):parts.append(f'<line x1="{x-w}" y1="{y+12+i*7}" x2="{x+w}" y2="{y+12+i*7}"/>')
def cap(x,y,label,top,bottom):
 text(x,y-36,top,'small','middle');line(x,y-25,x,y)
 parts.append(f'<line x1="{x-18}" y1="{y}" x2="{x+18}" y2="{y}"/><line x1="{x-18}" y1="{y+9}" x2="{x+18}" y2="{y+9}"/>')
 line(x,y+9,x,y+40);text(x,y+65,bottom,'small','middle');text(x+30,y+8,label,'small')
def resistor_h(x,y,label):
 line(x,y,x+15,y);box(x+15,y-10,65,20);line(x+80,y,x+95,y);text(x+47,y-23,label,'small','middle')
def resistor_v(x,y,label):
 line(x,y,x,y+12);box(x-10,y+12,20,50);line(x,y+62,x,y+75);text(x+24,y+43,label,'small')
def pot(y,title,pin):
 text(190,y-10,title,'head');text(140,y-32,'+5V','small','middle');line(140,y-20,140,y)
 box(130,y,20,60);line(140,y+60,140,y+85);ground(140,y+85)
 line(150,y+30,400 if pin.startswith('P3') else 490,y+30)
 parts.append(f'<path d="M 168 {y+22} L 150 {y+30} L 168 {y+38}"/>')
 text(175,y+15,'wiper','small');text(505,y+37,pin)
 dot(345,y+30);line(345,y+30,345,y+58)
 parts.append(f'<line x1="327" y1="{y+58}" x2="363" y2="{y+58}"/><line x1="327" y1="{y+67}" x2="363" y2="{y+67}"/>')
 line(345,y+67,345,y+85);ground(345,y+85);text(375,y+75,'10nF','small')

text(50,48,'Digispark PWM + IRS20752 + CSD18540Q5B','title')
text(50,80,'Reference wiring | 100Hz–500kHz at IN | requested duty 5%–95% | floating 12V supply','small')
box(30,105,1540,475);text(55,140,'1. Controls — regulated +5V','head')
box(490,185,310,310);text(645,275,'Digispark','head','middle');text(645,307,'ATtiny85 / 16MHz','small','middle')
pot(205,'VR1 10k linear','P2 / ADC1');pot(395,'VR2 10k linear','P3 / ADC3')
# S2 isolates the chip-side PB3 node from the ENTIRE original USB branch.
line(450,425,490,425)
parts.append('<circle cx="450" cy="425" r="4"/><circle cx="400" cy="425" r="4"/><circle cx="400" cy="360" r="4"/>')
line(450,425,405,425)
line(400,360,245,360)
text(245,345,'Original USB branch','small')
text(420,455,'S2','small')
line(650,185,650,163);text(650,154,'+5V','small','middle')
line(650,495,650,520);ground(650,520);text(690,525,'GND','small')
text(785,357,'P1','small','end');line(800,350,910,350)
# Manual open switch and input resistor.
line(910,350,950,350);parts.append('<circle cx="950" cy="350" r="4"/><circle cx="1010" cy="350" r="4"/>');line(950,350,1000,325);line(1010,350,1080,350)
text(975,300,'S1 RUN','head','middle');text(975,395,'OPEN during boot / upload','small','middle')
resistor_h(1080,350,'Rin 1k');line(1175,350,1280,350);text(1300,357,'PWM_IN','head')
dot(1230,350);resistor_v(1230,350,'Rin_PD 10k');ground(1230,425)
cap(900,480,'100nF + 10uF','+5V','GND')
text(55,555,'S2: UPLOAD = USB branch; RUN = VR2 (USB unplugged). P5 unused. See README before cutting any traces.','small')

box(30,605,1540,750);text(55,642,'2. High-side driver — isolated supply for VB–VS; no bootstrap diode','head')
box(100,700,330,140);text(265,732,'Isolated regulated DC/DC','','middle');text(265,764,'12V in / 12V out, >=100mA','small','middle')
line(60,750,100,750);text(60,690,'+12V','small');line(60,810,100,810);ground(60,810)
line(430,750,490,750);text(443,735,'OUT+ → VB','small');line(430,810,490,810);text(443,835,'OUT− → VS','small')
text(100,868,'Do NOT connect OUT− to GND.','small')
# Driver function box, not physical package layout.
box(510,920,300,245);text(660,954,'IRS20752LTRPBF','head','middle')
for y,label in [(990,'1 VCC'),(1060,'3 IN'),(1130,'2 COM')]:text(525,y+7,label);line(470,y,510,y)
text(415,997,'+12V','small','end');text(457,1067,'PWM_IN','small','end');ground(470,1130)
for y,label in [(990,'VB 6'),(1060,'HO 5'),(1130,'VS 4')]:text(795,y+7,label,'','end');line(810,y,850,y)
text(870,997,'VB','small')
# Gate wiring and simplified MOSFET.
line(850,1060,930,1060);resistor_h(930,1060,'Rg 4.7Ω');line(1025,1060,1170,1060)
parts.append('<line x1="1170" y1="1030" x2="1170" y2="1090"/><line x1="1185" y1="1030" x2="1185" y2="1090"/>')
line(1185,1030,1230,1030);line(1230,1030,1230,950);line(1185,1090,1230,1090);line(1230,1090,1230,1130)
text(1255,1030,'Q1 CSD18540Q5B','head');text(1255,1058,'N-MOSFET, high side','small');text(1140,1050,'G','small');text(1240,978,'D','small');text(1240,1120,'S','small')
text(1230,927,'Adjustable load supply +','small','middle');line(1230,940,1230,950)
line(850,1130,1230,1130);dot(1230,1130);text(930,1161,'VS / switching node','small')
# Gate pull down fits between gate and source.
dot(1090,1060);resistor_v(1090,1060,'Rgs 10k');line(1090,1135,1090,1130);dot(1090,1130)
line(1230,1130,1230,1185);resistor_v(1230,1185,'Load 0.5–8Ω');ground(1230,1260)
# Decoupling shown using net labels.
cap(160,1010,'C5 100nF','+12V','GND');cap(160,1190,'C6 1uF / 25V','+12V','GND')
cap(1020,750,'C7 100nF','VB','VS');cap(1350,750,'C8 1uF / 25V','VB','VS')
text(55,1318,'GND = Digispark GND = driver COM = load supply negative. VS is NOT ground.','small')

text(50,1395,'500kHz + 5% / 95% means ~100ns pulses: Gate fidelity is NOT guaranteed with this driver.','head')
text(50,1430,'Measure VGS with a suitable differential probe. Never clip an earth-ground scope lead to VS.','small')
text(50,1460,'Same net names are connected. Functional pin placement shown; this is not a package top view. See README for setup.','small')
parts.append('</svg>')
svg=''.join(parts)
(root/'wiring.svg').write_text(svg)
cairosvg.svg2png(bytestring=svg.encode(),write_to=str(root/'wiring.png'))
cairosvg.svg2pdf(bytestring=svg.encode(),write_to=str(root/'wiring.pdf'))
print('Created wiring.svg, wiring.png, wiring.pdf')
