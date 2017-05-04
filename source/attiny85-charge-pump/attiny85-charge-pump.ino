/*
  Dickson Charge Pump Driver
  Based on the initial work of Wayne Holder at
  https://sites.google.com/site/wayneholder/12-volt-charge-pump

  Description:
  This project is a regulated adjustable-voltage charge pump driver based on the
  Dickson Charge Pump circuit and using the Atmel attiny85 microcontroller. It can
  convert 5V to 15V as-configured, or higher if one adds additional intermediate
  stages.

  Assuming an 8 MHz system clock, such as the internal RC oscillator,
  and making certain compromises (e.g. using the ADC in free running, 8-bit mode
  with a 1 MHz ADC clock, not using interrupts, and dedicating the attiny85 solely
  as a charge pump controller) the attiny85 can switch at nearly 200 kHz. At a 1
  MHz system clock with a 125 kHz ADC clock, it can switch around 25 kHz.
  Occasional pulses are missed due to interrupts (e.g. for the millis timer) being
  handled, but this doesn't disrupt regulation in any meaningful way.


  Regulation is achieved using a pulse-skipping method: if the high voltage is
  above the threshold set by the potentiometer, the charge pump stops pumping and
  the voltage is drawn down by the load. If the high voltage drops below the
  threshold, the charge pump starts pumping more charge up to the high voltage
  stage until the threshold is reached.

  Although this code is written in AVR
  C and directly accesses registers for the highest performance, it is written for
  and easily compiled and deployed using the Arduino IDE and a compatible AVR
  programmer (I use the USBtinyISP). It can be easily and quickly adapted for
  Atmel Studio. It's quite small: in the Arduino IDE, it compiles to 350 bytes of
  flash and uses 10 bytes of RAM for global memory. In Atmel Studio, it uses 134
  bytes of flash and 1 byte for global memory. Even using the Arduino IDE, it's
  hardly taxing.

  Motivation:
  I had screwed up the fuses on a few AVR chips and couldn't program them with my
  ISP. To fix things, I needed to perform High Voltage Programming to reset the
  fuses. In addition, I wanted to use some N-channel MOSFETs on the high side and
  needed an easy way to generate higher voltages so I could turn the gate on.

  However, HVP requires a 12V supply and my N-channel MOSFETs need voltages ~10V
  above the drain to turn on. I could use a boost converter module or a bunch of
  batteries, but that's chunky and a pain for a small board. One can get small
  5V-to-12V boost converter modules on eBay and AliExpress for a buck or two, and
  they'd likely work well, but that's too easy and they're not (easily)
  adjustable for different voltages. For hobbyists, such modules are probably the
  easiest and cheapest bet as they come ready-to-go.

  Commercially, the MAX622A fits the bill: one chip, four capacitors, and you
  get regulated 12V/30mA out with much lower ripple. Perfect for HVP, op-amps, and
  MOSFET drivers, but I didn't want to wait a few days to have it delivered.
  They're also $5.50-$7.50 for DIP versions from DigiKey. Ouch. The attiny85 only
  costs about $1.25-$1.50 on DigiKey, and diodes and capacitors are dirt cheap. I
  had a bunch of those things lying around, so I figured I could make something
  similar. What could go wrong?

  Instructions:
  1. Set your attiny85 to run at 8 MHz on the internal oscillator with the
  following fuses: Low=0xE2, High=0xDF, Extended=0xFF.

  2. From the wiring diagram found at the link above, connect the D2, D3, and A0
  lines to ports 6, 3, and 7 on the attiny85, as depicted below. D5 in the
  original diagram is not used.

  3. Construct the rest of the circuit. I use 1N5819 Schottky diodes rather than
  the 1N4148 high-speed silicon diodes, since the Schottky's switch faster and
  have lower forward voltage drop.
    Larger values for C1, C2, and C3 are fine, so long as C4 has >10x the
  capacitance of the capacitors used for C1, C2, and C3. Try to minimize ESR (e.g.
  use ceramic or tantalum instead of aluminum electrolytics).
    I use pairs of 0.1uF ceramics and 10uF tantalums on C1, C2, and C3, and an
  0.1uF ceramic and 100uF aluminum electrolytic on the output stage and it works
  well. Your mileage may vary.
    Although the voltage ratings of C1, C2, and C3 are relatively minor (they
  carry 5V nominally), make sure the output capacitor (C4) has a voltage rating
  (and safety margin) sufficient to handle the output voltage relative to ground.

  4. Connect the D4 line in the original wiring diagram to VCC (presumably 5V)
  instead of an output pin. Unlike the original project, this project is intended
  to run continuiously and doesn't need an output pin to switch it on and off.

  5. Ensure that the power supply of the attiny85 is sufficiently bypassed to
  minimize noise. High-speed switching of the D2/D3 lines and the bursts of
  current into the D4 line makes a bunch of noise that needs to be suppressed.

  6. Instead of the fixed voltage divider in the original circuit, connect a
  potentiometer (100k ohm works well for me) with one terminal to the "12V" output
  stage of the voltage multiplier, the other terminal to ground, and the wiper to
  pin 7.

  7. Compile and load this code onto the attiny85. You'll probably need an AVR
  programmer. I like the USBtinyISP, or if you have an Arduino you could use the
  "Arduino as ISP" programmer.

  8. If all goes well, it should start pumping and you should measure a higher
  voltage on the output of the voltage multipler.

  9. Adjust the potentiometer to set the output voltage. If the voltage sags
  while under load, keep in mind that this charge pump is only designed to supply
  a few milliamps. Beyond a certain point it can't pump enough charge to maintain
  regulation.

  10. There is no step 10.

  Optional, but handy:
  1. It probably wouldn't be a bad idea to include a reverse protection diode (I
  have gobs of 1N5819 Schottky's lying around, so I used one of those.), either in
  series or as a crowbar (be sure to use a fuse!).

  2. Overload and short-circuit protection, in the form of a polyfuse in series
  with VCC, probably isn't a bad idea. It won't work fast enough to save the
  attiny85 if you put the chip in backwards (ask how I know!), but it'll prevent a
  fire or other catastrophic damage. The circuit draws about 10 mA with no load on
  the high voltage output, plus whatever your load is. At 1 MHz, it draws 3.75mA.
  In my case, I'm driving a voltage reference chip that requires 1mA at 15V, so a
  polyfuse with a trip current of 50mA works well for protection.

  3. An LC low-pass filter on the attiny85's VCC line and on the high voltage
  output could reduce the high-frequency switching noise considerably.


  Troubleshooting:
  - Check your wiring. Check it again.

  - No high voltage? Check the pump lines (D2 and D3) with an oscilloscope to
  make sure they're toggling.

  - If the high voltage is too low, adjust the potentiometer. If it's still too
  low, check that the load isn't too much.

  Notes:
  - The choice for which pins are used is fairly arbitrary, so long as the "A0
  Pump" pin is assigned to a pin with an ADC (PB2, PB3, or PB4). One might
  consider using PB3 and PB4 as the D2/D3 pump pins, so to avoid potential issues
  with the voltage multiplier interfering with ISP programmer signals on PB0 (MOSI)
  and PB1 (MISO).

  - Don't reverse the VCC and GND pins on the attiny85 when connected to a low
  impedance, unfused power supply. It will release the magic smoke and die
  horribly. Trust me on this one.

  - Although the load isn't drawn from a pin on the attiny85, doing so would
  make it possible to shut off the load as needed by driving the respective pin
  low. However, keep in mind that the attiny85 has a absolute maximum per-pin
  current of 40 mA, with 20 Ma being the recommended maximum. A ~250 ohm or higher
  resistor in series with such a pin would limit the current to a safe value in
  all conditions, at the cost of lower maximum current at the high voltage side.
  Omitting this resistor and drawing too much current will damage the pin or,
  possibly, the whole chip. This is why I'm not using PB0 in my design below; I
  accidentally blew the pin by drawing too much current.

                        +--\_/--+
                RESET  1|       |8  VCC
                  PB3  2|       |7  PB2 ADC1 [A0 Pump]
        [D3 Pump] PB4  3|       |6  PB1      [D2 Pump]
                  GND  4|       |5  PB0
                        +-------+
*/
#define P1  0x02  // PB1 - Line D2 (one of the pump lines)
#define P2  0x10  // PB4 - Line D3 (the other pump line)
#define REF 128   /* ADC is operating in 8-bit mode, so it ranges from 0-255.
  128 is the halfway point of the scale and is mostly arbitrary since the output
  voltage is controlled using the potentiometer. Alternatively, one could omit the
  potentiometer in favor of a fixed voltage divider and set the desired ADC level
  in software. */

void setup() {
  // Set pump pins as outputs.
  DDRB |= P1 | P2;

  //Set ADC1 (PB2) as the input, ADC reference as Vcc, left-adjust output.
  ADMUX |= (0 << REFS1)  |
           (0 << REFS0)  |
           (1 << MUX0)   |
           (1 << ADLAR);

  // Disable digital input on ADC pin 1. This slightly saves power.
  DIDR0 |= (1 << AIN1D);

  /* Enable the ADC's free-running mode. After each conversion, it saves the
    measured value to the ADCH and ADCL registers, then starts a new conversion
    automatically. */
  ADCSRB |= (0 << ADTS2) |
            (0 << ADTS1) |
            (0 << ADTS0);

  /* Enable the ADC. Set ADC in free-running mode, prescaler 8 (1 MHz ADC clock
    from an 8 MHz system clock, or 125kHz ADC clock from a 1 MHz system clock).
    Start free-running ADC conversions. */
  ADCSRA |= (1 << ADEN)  |
            (1 << ADSC)  |
            (1 << ADATE) |
            (0 << ADPS2) |
            (1 << ADPS1) |
            (1 << ADPS0) |
            (1 << ADSC);
}

void loop() {
  // Initialize the phase variable. This starts with pin P2 high.
  static char phase = P1;
  static char togglePort = P1^P2;

  // Read most recent analog voltage with 8 bit precision.
  int volts = ADCH;

  /* If the voltage is less than the threshold, start pumping. Otherwise, skip
    this cycle without pumping. */
  if (volts < REF) {

    // Toggle between the two phases.
    PORTB = phase;
    phase ^= togglePort;
  }
}
