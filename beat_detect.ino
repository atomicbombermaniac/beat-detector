/*
 * This code is mostly taken from
 * https://damian.pecke.tt/2015/03/02/beat-detection-on-the-arduino.html
 * 
 * It originally used an external pot to set the threshold for the beat detection.
 * Now it tries to automagically do it based on an average of past states compared to the current state.
 * 
 * The modifications and construction were made in  major-hurry.
 * Sorry for the low quality, but it's free and it's still better than nothing.
 * Use at own risk.
 * 
 */

// Our Global Sample Rate, 5000hz
#define SAMPLEPERIODUS 200

// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

void setup() {
  // put your setup code here, to run once:
  
  // Set ADC to 77khz, max for 10bit
    sbi(ADCSRA,ADPS2);
    cbi(ADCSRA,ADPS1);
    cbi(ADCSRA,ADPS0);

    //The pin with the LED
    pinMode(2, OUTPUT);
    pinMode(3, OUTPUT);
    pinMode(4, OUTPUT);
    pinMode(5, OUTPUT);
    pinMode(6, OUTPUT);
    pinMode(13, OUTPUT);
    Serial.begin(115200); // open the serial port at 115200 bps:

}


// 20 - 200hz Single Pole Bandpass IIR Filter
float bassFilter(float sample) {
    static float xv[3] = {0,0,0}, yv[3] = {0,0,0};
    xv[0] = xv[1]; xv[1] = xv[2]; 
    xv[2] = sample / 9.1f;
    yv[0] = yv[1]; yv[1] = yv[2]; 
    yv[2] = (xv[2] - xv[0])
        + (-0.7960060012f * yv[0]) + (1.7903124146f * yv[1]);
    return yv[2];
}

// 10hz Single Pole Lowpass IIR Filter
float envelopeFilter(float sample) { //10hz low pass
    static float xv[2] = {0,0}, yv[2] = {0,0};
    xv[0] = xv[1]; 
    xv[1] = sample / 160.f;
    yv[0] = yv[1]; 
    yv[1] = (xv[0] + xv[1]) + (0.9875119299f * yv[0]);
    return yv[1];
}

// 1.7 - 3.0hz Single Pole Bandpass IIR Filter
float beatFilter(float sample) {
    static float xv[3] = {0,0,0}, yv[3] = {0,0,0};
    xv[0] = xv[1]; xv[1] = xv[2]; 
    xv[2] = sample / 7.015f;
    yv[0] = yv[1]; yv[1] = yv[2]; 
    yv[2] = (xv[2] - xv[0])
        + (-0.7169861741f * yv[0]) + (1.4453653501f * yv[1]);
    return yv[2];
}


//This function does 2 things:
// 1. creates a string of '=' to be set over uart for devel purposes.
//       VU-meter style. the length is proportional to the value of the final filter.
// 2. as an afterthought, 5 leds were added/controlled as a real-life vu-meter for quick debugging at the music venue.

void create_string(float size, char * string){

    if(size > 5 )
      digitalWrite(2, HIGH);
      else
      digitalWrite(2, LOW);

    if(size > 10 )
      digitalWrite(3, HIGH);
      else
      digitalWrite(3, LOW);

    if(size > 20 )
      digitalWrite(4, HIGH);
      else
      digitalWrite(4, LOW);

    if(size > 30 )
      digitalWrite(5, HIGH);
      else
      digitalWrite(5, LOW);

    if(size > 40 )
      digitalWrite(6, HIGH);
      else
      digitalWrite(6, LOW);


    //offset and integerify size
    int val = 15 + ((int)size);

    //hard limit to 0-29
    if(val < 0) val = 0;
    if(val > 29) val = 29;

    //build string
    int i = 0;
    for(i = 0; i< val ; i++){
          *string = '=';
          string++;
      }
      *string = 0;//end with null
  }


//this function decides if a "valid" beat is detected
//it does the "magic" of averaging the last 9-ish samples from the
// last filter and comparing it to the current sample
//this code was all written and adjusted from head and real-life tests
// so excuse the quality of the late night ballmer's peak result
int beat_judge(float val){
  static float history[10];
  int i = 0;
  float avg =0;

  //compute average of last 9 samples (hopefully)
  for(i = 9; i >= 0; i--){
    avg += history[i];
  }
  avg = avg/9;


  //write history (heh, see what I did there? no? nevermind. Just pushing newest value on FIFO)
  for(i = 0; i< 8; i++){
    history[i] = history[i+1];
  }
  history[9] = val;

  //debug stuff
  //Serial.println("Avg:");
  //Serial.println(avg);
  //Serial.println(val);


  //decide
  if((avg * 145) < (val-45)){ //"magic" (adapt this garbage to something that works better, if possible)
    //basically we got fast rise in low freq volume
    return 1;
  }else{
    //fake news
    return 0;
  }
  
}

void loop() {
  // put your main code here, to run repeatedly:

    unsigned long time = micros(); // Used to track rate
    float sample, value, envelope, beat, thresh;
    unsigned char i;
    char buff[35];
    
    for(i = 0;;++i){
        // Read ADC and center so +-512
        sample = (float)analogRead(1)-358.f; //the 358 float value was experimentally obtained for my particular case.
                                             //basically, just see what approximate value you get from analogRead() when no signal present.
                                             //you want "sample" to be as close to 0 (zero) as possible when no signal (music) comes in.

        //use this to determine the offset from above (358 in my case)
        //Serial.println(sample);
    
        // Filter only bass component
        value = bassFilter(sample);
    
        // Take signal amplitude and filter (basically get the envelope of the low freq signals)
        if(value < 0)value=-value;
        envelope = envelopeFilter(value);
    
        // Every 200 samples (25hz) filter the envelope 
        if(i == 200) {
                // Filter out repeating bass sounds 100 - 180bpm
                beat = beatFilter(envelope);
                //Serial.println(beat);
                create_string(beat, buff);
                //Serial.println(buff);

                //control the MAIN output led here
                //low and high were inversed because the entire filter chain is so slow (phase delay),
                //  it better matches the actual time you hear the beats.
                //the downside is that it "fails" the first 1-2 beats, until it syncs
                if(beat_judge(beat)){
                  digitalWrite(13, LOW);
                  //Serial.println(">>>");
                }
                else digitalWrite(13, HIGH);
        
                //Reset sample counter
                i = 0;
        }
    
        // Consume excess clock cycles, to keep at 5000 hz
        for(unsigned long up = time+SAMPLEPERIODUS; time > 20 && time < up; 
    time = micros());
    }  

}
