# Copyright 2012 Google Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
    # http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Basename of the project.
PROJECT=pancake

# Location of EAGLE binary
EAGLE=/usr/local/google/ghohpe/eagle/bin/eagle

GERBER_FILES=fab/$(PROJECT)-sol-bot-copper.grb.sol \
             fab/$(PROJECT)-sol-bot-stop.grb.sts \
             fab/$(PROJECT)-sol-bot-silk.grb.pls \
             fab/$(PROJECT)-cmp-top-copper.grb.cmp \
             fab/$(PROJECT)-cmp-top-stop.grb.stc \
             fab/$(PROJECT)-cmp-top-silk.grb.plc \
             fab/$(PROJECT)-cut.grb
EXCELLON_FILES=fab/$(PROJECT).drd

EPS_FILES=docu/$(PROJECT)-sol-bot-copper.eps \
	  docu/$(PROJECT)-cmp-top-copper.eps \
          docu/$(PROJECT)-cmp-top-silk.eps
PS_PANELS=docu/$(PROJECT)-sol-bot-copper.panel.ps \
	  docu/$(PROJECT)-cmp-top-copper.panel.ps \
	  docu/$(PROJECT)-cmp-top-silk.panel.ps \
	  docu/$(PROJECT)-cmp-top-silk.panel-mirror.ps
PDF_PANELS=docu/$(PROJECT)-sol-bot-copper.panel.pdf \
	   docu/$(PROJECT)-cmp-top-copper.panel.pdf \
	   docu/$(PROJECT)-cmp-top-silk.panel.pdf \
	   docu/$(PROJECT)-cmp-top-silk.panel-mirror.pdf
SCHEMATIC=docu/$(PROJECT)-schematic.pdf \
	  docu/$(PROJECT)-schematic.png
CHEAT_SHEET=docu/$(PROJECT)-cheat-sheet.pdf \
	    docu/$(PROJECT)-cheat-sheet.png

CAM_PROCESS=$(EAGLE) -N+ -c+ -X
GERB_CAM=$(CAM_PROCESS) -dGERBER_RS274X
EXCELLON_CAM=$(CAM_PROCESS) -dEXCELLON -E0.025
EPS_CAM=$(CAM_PROCESS) -dEPS

# Gerber mirroring on solder side
GRB_MIRROR=-m-

# Mirror for EPS depending on if toner towards board.
# '-' for toner transfer and transparent photo exposure film.
EPS_SOL_MIRROR=-m-

# Mirror for component side:
# '-' to be readable
# '+' for toner transfer
EPS_CMP_MIRROR=-m-

all : gerber excellon docu fab-clean

excellon : prep $(EXCELLON_FILES)
gerber : prep $(GERBER_FILES)
postscript : prep $(EPS_FILES) $(SCHEMATIC) $(CHEAT_SHEET)
docu : prep $(SCHEMATIC) $(CHEAT_SHEET)
panels : $(PDF_PANELS)

clean :
	rm -f $(GERBER_FILES) $(EXCELLON_FILES) \
	      $(EPS_FILES) $(PS_PANELS) $(PDF_PANELS) \
	      $(CHEAT_SHEET) $(SCHEMATIC)

# remove files that are not needed by Olimex
fab-clean:
	rm -f fab/*.dri fab/*.gpi

prep :
	mkdir -p fab docu

%.drd: $(PROJECT).brd
	$(EXCELLON_CAM) -o$@ $< drills holes

### Schematic
docu/%-schematic.eps: $(PROJECT).sch
	$(EPS_CAM) -o$@ -f- $< nets busses symbols names values

docu/%-schematic.png: $(PROJECT).sch
	rm -f $@
	$(EAGLE) -C'export image $@ 300;QUIT' $<

### Solder side
%-sol-bot-copper.grb.sol: $(PROJECT).brd
	$(GERB_CAM) -o$@ $(GRB_MIRROR) $< pad via bottom

%-sol-bot-copper.eps: $(PROJECT).brd
	$(EPS_CAM) -o$@ $(EPS_MIRROR) -f- $< pad via bottom dimension

%-sol-bot-stop.grb.sts: $(PROJECT).brd
	$(GERB_CAM) -o$@ $(GRB_MIRROR) $< bstop

%-sol-bot-silk.grb.pls: $(PROJECT).brd
	$(GERB_CAM) -o$@ $(GRB_MIRROR) $< dimension bplace bnames

### Component side
%-cmp-top-copper.grb.cmp: $(PROJECT).brd
	$(GERB_CAM) -o$@ $(GRB_CMP_MIRROR) $< pad via top

%-cmp-top-copper.eps: $(PROJECT).brd
	$(EPS_CAM) -o$@ $(EPS_CMP_MIRROR) -f- $< pad via top dimension

%-cmp-top-stop.grb.stc: $(PROJECT).brd
	$(GERB_CAM) -o$@ $(GRB_CMP_MIRROR) $< tstop

%-cmp-top-silk.grb.plc: $(PROJECT).brd
	$(GERB_CAM) -o$@ $< $(GRB_CMP_MIRROR) dimension tplace tnames tvalues

%-cmp-top-silk.eps: $(PROJECT).brd
	$(EPS_CAM) -o$@ $(EPS_CMP_MIRROR) $< dimension tplace tnames tvalues

### Outline
%-cut.grb: $(PROJECT).brd
	$(GERB_CAM) -o$@ $< $(GRB_CMP_MIRROR) dimension

### Docu
%-cheat-sheet.eps: $(PROJECT).brd
	$(EPS_CAM) -s 3 -o$@ $< dimension tplace tnames tDocu pad

%-cheat-sheet.png: $(PROJECT).brd
	rm -f $@
	$(EAGLE) -C'DISPLAY none Dimension tDocu tPlace bPlace tNames pads; EXPORT image $@ 300;QUIT' $<

# Its easier to center the manual drill if the holes are small.
%.smallhole-eps : %.eps
	sed 's|/h {|/h { pop 3000|' < $< > $@

%.panel.ps : %.smallhole-eps ~/bin/panelize.sh
	~/bin/panelize.sh $< > $@

%.panel-mirror.ps : %.eps ~/bin/panelize.sh
	~/bin/panelize.sh -m $< > $@

%.pdf : %.ps
	ps2pdf $< $*.pdf

%.pdf : %.eps
	ps2pdf $< $*.pdf

%.png : %.pov
	povray +L ~/eagle/ulp/eagle3d/povray/ +L /usr/lib/cinelerra/fonts/ +W1024 +H768 +Q11 +A0.05 $<
