The Pancake PCB is designed to fit into a [Takachi LC-115N case](http://takachi-enclosure.com/data/pdf/en2011_017.pdf). It is mounted inside the top cover with the NFC module mounted on the PCB-c back. USB connection is through the front panel. Indicator lights can be mounted to the panel (for charge status and NFC active) and/or the top cover (NFC Active).



A Pancake PCB setup for no battery (USB power) and top indicator light:

http://nfc-smart-tag.googlecode.com/git/docs/lc115n/01_PCB_Front.JPG

The PCB back side does not contain anything. The reverse mount LED peeks through the opening:

http://nfc-smart-tag.googlecode.com/git/docs/lc115n/02_PCB_Back.JPG

# Mounting the NFC Module to the PCB #

  * The NFC module is spaced from the board by the collar of the through hole studs, but it's safer to add some electric tape in between in case the NFC module bends:

http://nfc-smart-tag.googlecode.com/git/docs/lc115n/03_Tape.JPG

  * Mount the NFC module to the back of the PCB (solder side) with (4) M2x3 screws:

http://nfc-smart-tag.googlecode.com/git/docs/lc115n/04_RC_S620.JPG

  * Insert the FFC cable:

http://nfc-smart-tag.googlecode.com/git/docs/lc115n/05_Cable_Back.JPG

  * And connect the other side to the FCC connector on the front of the PCB.

http://nfc-smart-tag.googlecode.com/git/docs/lc115n/06_Cable_Front.JPG

It should look like this:

http://nfc-smart-tag.googlecode.com/git/docs/lc115n/07_Cable_Edge.JPG

# Mounting Top Light PCB into Case #

  * If a top light is equipped, insert the light pipe into the top cover of the case. Because the light pipe overlaps with the metal frame of the NFC module, one corner needs to be cut off. We will fix this issue in a future revision:
http://nfc-smart-tag.googlecode.com/git/docs/lc115n/08_Case_Cover_Inside.JPG

  * Now insert the PCB (with NFC module behind) into the top cover, fitting the light pipe "legs" into the drill holes of the PCB.
  * Insert the front panel at the same time (_not shown here_) as it will not fit once the PCB is mounted.
  * Secure the PCB with (4) EM-2.3 tapping screws:

http://nfc-smart-tag.googlecode.com/git/docs/lc115n/09_PCB_Case.JPG

The reverse mount LED shines through the light pipe:

http://nfc-smart-tag.googlecode.com/git/docs/lc115n/10_Case_Cover_Outside.JPG

# Mounting Battery and Charge Indicator #

If a battery is equipped, tape it into the bottom cover and connect it to JP2 (JST Connector):

http://nfc-smart-tag.googlecode.com/git/docs/lc115n/11_Battery.JPG

The front panel light pipes and the wake-up switch (optional) penetrate through the
machined front panel:

http://nfc-smart-tag.googlecode.com/git/docs/lc115n/12_Front_Panel_Interior.JPG

**Note**: Insert the panel over the light pipes before you screw in the PCB. It is not possible to slide in the panel afterwards.

Now everything looks neat from the outside:
http://nfc-smart-tag.googlecode.com/git/docs/lc115n/13_Front_Panel_Exterior.JPG