#include <Fsm.h>
#include <Bounce2.h>

// Motor 1
#define MD1_IN1 26
#define MD1_IN2 27
#define MD1_ENA 2

// Motor 2
#define MD1_IN3 28
#define MD1_IN4 29
#define MD1_ENB 3

// Motor 3
#define MD2_IN1 30
#define MD2_IN2 31
#define MD2_ENA 4

// Motor 4
#define MD2_IN3 32
#define MD2_IN4 33
#define MD2_ENB 5

// Stimuli Beam Break
#define A_STIM_BB_PIN A0 // Connected
#define B_STIM_BB_PIN A1
#define C_STIM_BB_PIN A2
#define D_STIM_BB_PIN A3

// Sniff Confirmation Beam Break
#define A_SNIF_BB_PIN A1 // Connected
#define B_SNIF_BB_PIN A5
#define C_SNIF_BB_PIN A6
#define D_SNIF_BB_PIN A7

// Track Beam Break
#define P_TRAK_BB_PIN A8
#define Q_TRAK_BB_PIN A9

// Feeder Beam Break
#define P1_FEED_BB_PIN A2 // Connected
#define P2_FEED_BB_PIN A11
#define Q1_FEED_BB_PIN A3 // Connected
#define Q2_FEED_BB_PIN A13

// TTL
#define TTL_PIN 51

#define feeder_speed 127

// FSM - EVENTS
#define EVENT_READY_STIM 2 
#define EVENT_STIM_RESET 3
#define EVENT_STIM_SNIFF 4
#define EVENT_SNIFF_RESET 5
#define EVENT_SNIFF_REW 6
#define EVENT_REW_RESET 7
#define EVENT_RESET_READY 8

// FSM - LED PINS (TO DISPLAY CURRENT STATE)
#define READY_STATE_PIN 34
#define STIM_STATE_PIN 36
#define SNIFF_STATE_PIN 38
#define REW_STATE_PIN 40

int ttl_timestamp;

int val_bb_stim;
int val_bb_sniff;
int val_bb_rew;

int sniff_duration;
String err;

// Beam Break Switch De-bounce wrappers
Bounce bounce_A_STIM_BB = Bounce();
Bounce bounce_A_SNIF_BB = Bounce();
Bounce bounce_P1_FEED_BB = Bounce();
Bounce bounce_Q1_FEED_BB = Bounce();


// Feeder Methods
void retract_P1(){
  digitalWrite(MD1_IN1, LOW);
  digitalWrite(MD1_IN2, HIGH);
  analogWrite(MD1_ENA, 255);
  delay(10000);
  digitalWrite(MD1_IN1, LOW);
  digitalWrite(MD1_IN2, LOW);
  analogWrite(MD1_ENA, 0);
}

void retract_Q1(){
  digitalWrite(MD1_IN3, LOW);
  digitalWrite(MD1_IN4, HIGH);
  analogWrite(MD1_ENB, 255);
  delay(10000);
  digitalWrite(MD1_IN3, LOW);
  digitalWrite(MD1_IN4, LOW);
  analogWrite(MD1_ENB, 0);
}

void deliver_reward_P1() {
  digitalWrite(MD1_IN1, HIGH);
  digitalWrite(MD1_IN2, LOW);
  analogWrite(MD1_ENA, feeder_speed);
  delay(1000);
  digitalWrite(MD1_IN1, LOW);
  digitalWrite(MD1_IN2, LOW);
  analogWrite(MD1_ENA, 0);
}

void deliver_reward_Q1() {
  digitalWrite(MD1_IN3, HIGH);
  digitalWrite(MD1_IN4, LOW);
  analogWrite(MD1_ENB, feeder_speed);
  delay(1000);
  digitalWrite(MD1_IN3, LOW);
  digitalWrite(MD1_IN4, LOW);
  analogWrite(MD1_ENB, 0);
}


// FSM Methods
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

void log_TTL() {
  //Serial.send("!" + String(millis()) + "!");
}

// Setup
void setup() {
 Serial.begin(115200);
 Serial.setTimeout(1);

 //pinMode(RESET_STATE_PIN, OUTPUT);
 pinMode(READY_STATE_PIN, OUTPUT);
 pinMode(STIM_STATE_PIN, OUTPUT);
 pinMode(SNIFF_STATE_PIN, OUTPUT);
 pinMode(REW_STATE_PIN, OUTPUT);

 pinMode(TTL_PIN, INPUT);
 attachInterrupt(TTL_PIN, log_TTL, RISING);
 
 //pinMode(BB_READY_PIN, INPUT);
 bounce_A_STIM_BB.attach(A_STIM_BB_PIN, INPUT_PULLUP);
 //bounce.attach(BB_SNIFF_PIN, INPUT);
 //bounce.attach(BB_REW_PIN, INPUT);
 bounce_A_STIM_BB.interval(500);

 bounce_A_SNIF_BB.attach(A_SNIF_BB_PIN, INPUT_PULLUP);
 bounce_A_SNIF_BB.interval(500);

 bounce_P1_FEED_BB.attach(P1_FEED_BB_PIN, INPUT_PULLUP);
 bounce_P1_FEED_BB.interval(250);

 bounce_Q1_FEED_BB.attach(Q1_FEED_BB_PIN, INPUT_PULLUP);
 bounce_Q1_FEED_BB.interval(250);

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
                          60000,
                          &on_stim_to_reset_timed);
                          
 fsm.add_timed_transition(&state_reset, &state_ready,
                          2000,
                          NULL);
                          
 fsm.add_timed_transition(&state_sniff, &state_rew,
                          5000,
                          NULL);
                          
 fsm.add_timed_transition(&state_rew, &state_reset,
                          60000,
                          NULL);      
                          
  pinMode(MD1_IN1, OUTPUT);
  pinMode(MD1_IN2, OUTPUT);
  pinMode(MD1_IN3, OUTPUT);
  pinMode(MD1_IN4, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  //reward_P1();
  //reward_Q1();
  //delay(1000);
 fsm.run_machine();
 
 bounce_A_STIM_BB.update();
 bounce_A_SNIF_BB.update();
 bounce_P1_FEED_BB.update();
 bounce_Q1_FEED_BB.update();
 
 //Serial.println(digitalRead(A_STIM_BB_PIN));
 if(bounce_A_STIM_BB.changed()){
   val_bb_stim = digitalRead(A_STIM_BB_PIN);
   
   if(val_bb_stim == LOW){
    fsm.trigger(EVENT_READY_STIM);
   }
   if(val_bb_stim == HIGH){
    fsm.trigger(EVENT_STIM_RESET);
    fsm.trigger(EVENT_SNIFF_RESET);
   }
 }

 if(bounce_A_SNIF_BB.changed()){
  val_bb_sniff = digitalRead(A_SNIF_BB_PIN);
  if(val_bb_sniff == LOW){
    fsm.trigger(EVENT_STIM_SNIFF);
  }
  if(val_bb_sniff == HIGH){
    if(sniff_duration > 1000){
      fsm.trigger(EVENT_SNIFF_REW);
    } else {
      err = "sniff_duration too short:"+String(sniff_duration);
      fsm.trigger(EVENT_SNIFF_RESET);
    }
    sniff_duration = 0;
  }
 } else {
  sniff_duration = bounce_A_SNIF_BB.duration();
 }

 if(bounce_P1_FEED_BB.changed() || bounce_Q1_FEED_BB.changed()){
  val_bb_rew = digitalRead(P1_FEED_BB_PIN);
  if(val_bb_rew == LOW){
    fsm.trigger(EVENT_REW_RESET);
  } else {
    val_bb_rew = digitalRead(Q1_FEED_BB_PIN);
    if(val_bb_rew == LOW){
      fsm.trigger(EVENT_REW_RESET);
    }
  }
 }

 if(Serial.available()){
  char inByte = Serial.read();
  fsm.trigger(EVENT_RESET_READY);  
 }
}
