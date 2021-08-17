#include <Fsm.h>
#include <Bounce2.h>

#define LED_PIN 13

//#define BB_READY_PIN A0
#define BB_STIM_PIN A0
#define BB_SNIFF_PIN A1
#define BB_REW_PIN A3

#define EVENT_READY_STIM 2 
#define EVENT_STIM_RESET 3
#define EVENT_STIM_SNIFF 4
#define EVENT_SNIFF_RESET 5
#define EVENT_SNIFF_REW 6
#define EVENT_REW_RESET 7
#define EVENT_RESET_READY 8

#define READY_STATE_PIN 2
#define STIM_STATE_PIN 4
#define SNIFF_STATE_PIN 7
#define REW_STATE_PIN 8

int val_bb_stim;
int val_bb_sniff;
int val_bb_rew;

int sniff_duration;
String err;

Bounce bounce_BB_STIM = Bounce();
Bounce bounce_BB_SNIFF = Bounce();
Bounce bounce_BB_REW = Bounce();
//Bounce bounce_BB_STIM = Bounce();

State state_reset(&on_reset_enter, NULL, &on_reset_exit);
State state_ready(&on_ready_enter, on_ready_state, &on_ready_exit);
State state_stim(&on_stim_enter, NULL, &on_stim_exit);
State state_sniff(&on_sniff_enter, NULL, &on_sniff_exit);
State state_rew(&on_rew_enter, NULL, &on_rew_exit);
Fsm fsm(&state_ready);

void on_ready_enter() {
 digitalWrite(READY_STATE_PIN, HIGH);
 Serial.print("READY_ENTER:"+String(millis())+"|");
 err = "";
}

void on_ready_state() {
  digitalWrite(READY_STATE_PIN, HIGH);
}

void on_ready_exit() {
  digitalWrite(READY_STATE_PIN, LOW);
  Serial.print("READY_EXIT:"+String(millis())+"|");
}

void on_stim_enter() {
  digitalWrite(STIM_STATE_PIN, HIGH);
  Serial.print("STIM_ENTER:"+String(millis())+"|");
}

void on_stim_exit() {
  digitalWrite(STIM_STATE_PIN, LOW);
  Serial.print("STIM_EXIT:"+String(millis())+"|");
}

void on_sniff_enter() {
  digitalWrite(SNIFF_STATE_PIN, HIGH);
  Serial.print("SNIFF_ENTER:"+String(millis())+"|");
}

void on_sniff_exit() {
  digitalWrite(SNIFF_STATE_PIN, LOW);
  Serial.print("SNIFF_EXIT:"+String(millis())+"|");
}

void on_rew_enter() {
  digitalWrite(REW_STATE_PIN, HIGH);
  Serial.print("REW_ENTER:"+String(millis())+"|");
}

void on_rew_exit() {
  digitalWrite(REW_STATE_PIN, LOW);
  Serial.print("REW_EXIT:"+String(millis())+"|");
}

void on_reset_enter() {
  Serial.print("RESET:"+String(millis())+"|"+"ERR:"+err+"\n");
}

void on_reset_exit() {
  
}

void on_stim_to_reset() {
  err = "stimuli beam deactivated before sniff started";
}

void on_sniff_to_reset() {
  if(err == ""){
    err = "stimuli beam deactivated before sniff completed";
  }
}

void on_stim_to_reset_timed() {
  err = "no sniff beam break triggered within time limit";
}


void setup() {
 Serial.begin(115200);
 Serial.setTimeout(1);

 //pinMode(RESET_STATE_PIN, OUTPUT);
 pinMode(READY_STATE_PIN, OUTPUT);
 pinMode(STIM_STATE_PIN, OUTPUT);
 pinMode(SNIFF_STATE_PIN, OUTPUT);
 pinMode(REW_STATE_PIN, OUTPUT);
 
 //pinMode(BB_READY_PIN, INPUT);
 bounce_BB_STIM.attach(BB_STIM_PIN, INPUT_PULLUP);
 //bounce.attach(BB_SNIFF_PIN, INPUT);
 //bounce.attach(BB_REW_PIN, INPUT);
 bounce_BB_STIM.interval(500);

 bounce_BB_SNIFF.attach(BB_SNIFF_PIN, INPUT);
 bounce_BB_SNIFF.interval(500);

 bounce_BB_REW.attach(BB_REW_PIN, INPUT);
 bounce_BB_REW.interval(250);

 fsm.add_transition(&state_ready, &state_stim,
                    EVENT_READY_STIM,
                    NULL);
 fsm.add_transition(&state_stim, &state_reset,
                    EVENT_STIM_RESET,
                    &on_stim_to_reset);
 fsm.add_transition(&state_stim, &state_sniff,
                    EVENT_STIM_SNIFF,
                    NULL);
 fsm.add_transition(&state_sniff, &state_reset,
                    EVENT_SNIFF_RESET,
                    &on_sniff_to_reset);
 fsm.add_transition(&state_sniff, &state_rew,
                    EVENT_SNIFF_REW,
                    NULL);
 fsm.add_transition(&state_rew, &state_reset,
                    EVENT_REW_RESET,
                    NULL);     
 fsm.add_transition(&state_reset, &state_ready,
                    EVENT_RESET_READY,
                    NULL);           
                    

 fsm.add_timed_transition(&state_stim, &state_reset,
                          4000,
                          &on_stim_to_reset_timed);
                          
 //fsm.add_timed_transition(&state_reset, &state_ready,
  //                        5000,
   //                       NULL);
                          
 fsm.add_timed_transition(&state_sniff, &state_rew,
                          5000,
                          NULL);
                          
 fsm.add_timed_transition(&state_rew, &state_reset,
                          60000,
                          NULL);                         
}

void loop() {
 fsm.run_machine();
 
 bounce_BB_STIM.update();
 bounce_BB_SNIFF.update();
 bounce_BB_REW.update();
 Serial.println(digitalRead(BB_STIM_PIN));
 if(bounce_BB_STIM.changed()){
   val_bb_stim = digitalRead(BB_STIM_PIN);
   
   if(val_bb_stim == LOW){
    fsm.trigger(EVENT_READY_STIM);
   }
   if(val_bb_stim == HIGH){
    fsm.trigger(EVENT_STIM_RESET);
    fsm.trigger(EVENT_SNIFF_RESET);
   }
 }

 if(bounce_BB_SNIFF.changed()){
  val_bb_sniff = digitalRead(BB_SNIFF_PIN);
  if(val_bb_sniff == HIGH){
    fsm.trigger(EVENT_STIM_SNIFF);
  }
  if(val_bb_sniff == LOW){
    if(sniff_duration > 1500){
      fsm.trigger(EVENT_SNIFF_REW);
    } else {
      err = "sniff_duration too short:"+String(sniff_duration);
      fsm.trigger(EVENT_SNIFF_RESET);
    }
    sniff_duration = 0;
  }
 } else {
  sniff_duration = bounce_BB_SNIFF.duration();
 }

 if(bounce_BB_REW.changed()){
  val_bb_rew = digitalRead(BB_REW_PIN);
  if(val_bb_rew == HIGH){
    fsm.trigger(EVENT_REW_RESET);
  }
 }

 if(Serial.available()){
  char inByte = Serial.read();
  fsm.trigger(EVENT_RESET_READY);  
 }

 
}
