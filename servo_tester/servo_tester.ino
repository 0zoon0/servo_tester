uint8_t  MODE_BTN    = PB0; // Button to switch between modes 
uint8_t  LED1_PIN    = PB3; // LED1 output
uint8_t  LED2_PIN    = PB4; // LED2 output

volatile uint8_t mode = 3; // current operating mode
static volatile uint8_t speed = 1; // servo speed

volatile unsigned int tick;  // pulse counter
volatile unsigned int s_pulse; // Servo pulse counter

static unsigned long last_interrupt_time = 0;

ISR (PCINT0_vect) { 
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 500ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 500) 
  {
    setNextMode();
  }
  
  last_interrupt_time = interrupt_time;    
}

ISR(TIM0_COMPA_vect)  // 100 KHz interrupt frequency
{
  if(tick >= 2000)  // One servo frame (20ms) completed
  {
    tick = 0;
  }

  tick = tick + 1;
  if(tick <= s_pulse)  // Generate servo pulse
  {
    PORTB |= (1<<PB1);  // Servo pulse high
  }
  else
  {
    PORTB &= ~(1<<PB1); // Servo pulse low
  }
} 


void setup() {

    // remove system clock prescaler so that it runs at 9.6Mhz
    //1. Write the Clock Prescaler Change Enable (CLKPCE) bit to one and all other bits in CLKPR to zero.
    //2. Within four cycles, write the desired value to CLKPS while writing a zero to CLKPCE. 
    // set CLKPCE to 1 and other bits to zero
    CLKPR = 1<<CLKPCE; //0b10000000;
    // set prescaler to 1
    CLKPR = 0b00000000;
    //when compiling we have to use the optimization flags -0s, otherwise the process might take more than four cycles

    OSCCAL=0x78;  // use adjustment register to fine tune the oscilator so that we get exacly 20ms PWM cycle

    DDRB |= (1<<PB1) | (1<<LED1_PIN) | (1<<LED2_PIN) ; // PB1 and LED pins as outputs
    
    // Configure timer 1 for CTC mode
    TCCR0A |= (1<<WGM01);
    TCCR0A &= ~(1<<WGM00);
    TCCR0B &= ~(1<<WGM02);
    
    TIMSK0 |= (1<<OCIE0A); // Enable CTC interrupt
    OCR0A = 95; // Set CTC compare value, so that at 9.6 MHz we interupt every 96 (counting from 0) counts we get 100kHz 
    // No prescaler for timer
    TCCR0B |= (1<<CS00); 
    TCCR0B &= ~((1<<CS01) | (1<<CS02));

    tick = 0;
    s_pulse = 100; // Valid range 100-200 as servo valid pulses are 20ms cycle with 1ms to 2ms pulse

    PORTB = (1<<MODE_BTN); // set pull-up to button pin
    DDRB &= ~(1<<MODE_BTN);

    GIMSK = 1<<PCIE; // turns on pin change interrupts 
    PCMSK = 1<<PCINT0; // turn on interrupts on pins PB0

    sei(); //  Enable global interrupts

    // turn off leds
    writeDigital(LED1_PIN, LOW);
    writeDigital(LED2_PIN, LOW);
}

// move servo on entire range continously
void loop() 
{
  if (speed > 1)
  {
    writeDigital(LED1_PIN, LOW);
    writeDigital(LED2_PIN, HIGH);

    for(int i=0; i<100; i++) 
    {
      s_pulse = s_pulse + 1;
      delay(speed);
    }

    writeDigital(LED1_PIN, HIGH);
    writeDigital(LED2_PIN, LOW);
    
    for(int i=0; i<100; i++)
    {
      s_pulse = s_pulse - 1;
      delay(speed);
    }
  }
  else
  {
    writeDigital(LED1_PIN, LOW);
    writeDigital(LED2_PIN, LOW);
  }
}

void writeDigital(uint8_t pin, bool value)
{
    if (value == HIGH) PORTB |= (1<<pin); 
    else PORTB &= ~(1<<pin);
}

void setNextMode()
{
  mode++;
  
  if (mode >= 4 || mode <= 0)
  {
    mode = 1;
  }
 
  switch(mode)
  {
    case 1:
      speed = 5;
      break;
    case 2:
      speed = 2;
      break;
    case 3:
      speed = 1;
      break;
    default:
      speed = 1;
      // should never happen
      break;
  }
}