#include <Fsm.h>
#include <Bounce2.h>
#include "limits.h"

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
#define A_ZONE_BB_PIN A1 // Connected
#define B_ZONE_BB_PIN A5
#define C_ZONE_BB_PIN A6
#define D_ZONE_BB_PIN A7

// Track Beam Break
#define P_TRAK_BB_PIN A8
#define Q_TRAK_BB_PIN A9

// Feeder Beam Break
#define P1_FEED_BB_PIN A2 // Connected
#define P2_FEED_BB_PIN A11
#define Q1_FEED_BB_PIN A3 // Connected
#define Q2_FEED_BB_PIN A13

// TTL
#define TTL_PIN 20

#define feeder_speed 127

// FSM - EVENTS
#define EVENT_READY_STIM 2 
#define EVENT_STIM_RESET 3
#define EVENT_STIM_OPEN 4
#define EVENT_OPEN_RESET 5
#define EVENT_OPEN_REW 6
#define EVENT_REW_RESET 7
#define EVENT_RESET_READY 8
#define EVENT_READY_RESET 9

// FSM - LED PINS (TO DISPLAY CURRENT STATE)
#define READY_STATE_PIN 34
#define STIM_STATE_PIN 36
#define OPEN_STATE_PIN 38
#define REW_STATE_PIN 40

unsigned long ttl_timestamp;
unsigned long prev_ttl_val = 0;

int val_bb_stim;
int val_bb_sniff;
int val_bb_rew;

unsigned long stim_start_timestamp = 0;
unsigned long target_reward_cue_delay = ULONG_MAX;

String err;

String target_feeder;

// Beam Break Switch De-bounce wrappers
Bounce bounce_A_STIM_BB = Bounce();
Bounce bounce_A_ZONE_BB = Bounce();
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


// FSM States
State state_config(&on_config_enter, NULL, &on_config_exit); // Configuration state where user controls session type (training, testing, etc...) via Python interface

// Training Paradigms
State state_training_manual_feed(&on_training_manual_feed_enter, NULL, &on_training_manual_feed_exit); // State for training paradigm - manual feed from feeders to habituate bat to feeding from feeders

// Testing Paradigm
State state_reset(&on_reset_enter, NULL, &on_reset_exit); // Reset state where user resets test bat to interaction zone 
State state_ready(&on_ready_enter, on_ready_state, &on_ready_exit); // Ready state where test bat is ready to go and user can present stimulus bat
State state_stim(&on_stim_enter, NULL, &on_stim_exit); // Stimulus state where stimulus bat has been presented and bats are both in interaction zone
State state_open(&on_open_enter, NULL, &on_open_exit); // Open arena state where test bat is allowed to move around to collect its reward (if correct)
State state_rew(&on_rew_enter, NULL, &on_rew_exit); // Reward state where test bat decision is checked and reward is delivered (or not delivered)

// FSM Instance
Fsm fsm(&state_config);

// FSM Lifecycle Methods
void on_training_manual_feed_enter() {
  Serial.print("TRAIN_MANUAL_FEED_ENTER:"+String(millis())+"|");
}

void on_training_manual_feed_exit() {
  Serial.print("TRAIN_MANUAL_FEED_EXIT:"+String(millis())+"|");
}

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
  stim_start_timestamp = millis(); // Timestamp start of interaction (test bat in interaction zone and stimulus bat presented)
  target_reward_cue_delay = 10000; // Should be randomized
  Serial.print("STIM_ENTER:"+String(millis())+"|");
}

void on_config_enter() {
  Serial.print("CONFIG_ENTER:"+String(millis())+"|");
}

void on_config_exit() {
  Serial.print("CONFIG_EXIT:"+String(millis())+"|");
}

void on_open_enter() {
  Serial.print("OPEN_ENTER:"+String(millis())+"|");
}

void on_open_exit() {
  Serial.print("OPEN_EXIT:"+String(millis())+"|");
}

void on_stim_exit() {
  digitalWrite(STIM_STATE_PIN, LOW);
  Serial.print("STIM_EXIT:"+String(millis())+"|");
}

void on_rew_enter() {
  Serial.print("REW_ENTER:"+String(millis())+"|");
  digitalWrite(REW_STATE_PIN, HIGH);
  if (target_feeder = "P1") {
    deliver_reward_P1();
  } else if (target_feeder = "Q1") {
    deliver_reward_Q1();
  }
}

void on_rew_exit() {
  digitalWrite(REW_STATE_PIN, LOW);
  Serial.print("REW_EXIT:"+String(millis())+"|");
}

void on_reset_enter() {
  Serial.print("RESET:"+String(millis())+"|"+"ERR:"+err+"\n");
  target_feeder = "";
}

void on_reset_exit() {
  
}

void on_ready_to_reset() {
  err = "Manual reset from STATE_READY";
}

void on_stim_to_reset() {
  err = "Manual reset from STATE_STIM";
}

void on_open_to_reset() {
  err = "Manual reset from STATE_OPEN";
}

void log_TTL() {
  Serial.print("|" + String(millis()) + "|");
}

// Setup
void setup() {
 Serial.begin(115200);
 Serial.setTimeout(1);

 //pinMode(RESET_STATE_PIN, OUTPUT);
 pinMode(READY_STATE_PIN, OUTPUT);
 pinMode(STIM_STATE_PIN, OUTPUT);
 pinMode(REW_STATE_PIN, OUTPUT);

 pinMode(TTL_PIN, INPUT);
 attachInterrupt(digitalPinToInterrupt(TTL_PIN), log_TTL, CHANGE);
 
 //pinMode(BB_READY_PIN, INPUT);
 bounce_A_STIM_BB.attach(A_STIM_BB_PIN, INPUT_PULLUP);
 //bounce.attach(BB_REW_PIN, INPUT);
 bounce_A_STIM_BB.interval(500);

 bounce_A_ZONE_BB.attach(A_ZONE_BB_PIN, INPUT_PULLUP);
 bounce_A_ZONE_BB.interval(500);

 bounce_P1_FEED_BB.attach(P1_FEED_BB_PIN, INPUT_PULLUP);
 bounce_P1_FEED_BB.interval(250);

 bounce_Q1_FEED_BB.attach(Q1_FEED_BB_PIN, INPUT_PULLUP);
 bounce_Q1_FEED_BB.interval(250);

 // Task step transitions
 fsm.add_transition(&state_reset, &state_ready,
                    EVENT_RESET_READY,
                    NULL);           
 fsm.add_transition(&state_ready, &state_stim,
                    EVENT_READY_STIM,
                    NULL);
 fsm.add_transition(&state_stim, &state_open,
                    EVENT_STIM_OPEN,
                    NULL);  
 fsm.add_transition(&state_open, &state_rew,
                    EVENT_OPEN_REW,
                    NULL);

 // RESET transitions                   
 fsm.add_transition(&state_ready, &state_reset,
                    EVENT_READY_RESET,
                    &on_ready_to_reset);                                                         
 fsm.add_transition(&state_stim, &state_reset,
                    EVENT_STIM_RESET,
                    &on_stim_to_reset);
 fsm.add_transition(&state_open, &state_reset,
                    EVENT_OPEN_RESET,
                    &on_open_to_reset);       
                                         
                          
 //fsm.add_timed_transition(&state_reset, &state_ready,
 //                         2000,
 //                         NULL);
                          
                          
 fsm.add_timed_transition(&state_rew, &state_reset,
                          1000,
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
 bounce_A_ZONE_BB.update();
 bounce_P1_FEED_BB.update();
 bounce_Q1_FEED_BB.update();

 // If zone A BB triggered, then test bat has been placed in interaction zone
 // Transition from RESET to READY State
 if(bounce_A_ZONE_BB.changed()){
  if(digitalRead(A_ZONE_BB_PIN) == LOW){
    fsm.trigger(EVENT_RESET_READY);
  }
 }

 // If stumulis A BB triggered, then stimulus has been presented
 // Transition from READY to Stimulus
 if(bounce_A_STIM_BB.changed()){
   if(digitalRead(A_STIM_BB_PIN) == LOW){
    fsm.trigger(EVENT_READY_STIM);
   }
 }

 // After randomized delay after stimulus presentation, open door
 if(millis() - stim_start_timestamp >= target_reward_cue_delay) {
   fsm.trigger(EVENT_STIM_OPEN);
   target_reward_cue_delay = ULONG_MAX;
 }
 

 if(bounce_P1_FEED_BB.changed()){
  if(digitalRead(P1_FEED_BB_PIN) == LOW){
    if(target_feeder == "P1" || target_feeder == "*"){
        fsm.trigger(EVENT_OPEN_REW);
      } else {
        err = "Wrong feeder. Expected: Q1, Triggerd: P1";
        fsm.trigger(EVENT_OPEN_RESET);
      }
  }

  if(bounce_Q1_FEED_BB.changed()) {
    if(digitalRead(Q1_FEED_BB_PIN) == LOW){
      if(target_feeder == "Q1" || target_feeder == "*"){
        fsm.trigger(EVENT_OPEN_REW);
      } else {
        err = "Wrong feeder. Expected: P1, Triggerd: Q1";
        fsm.trigger(EVENT_OPEN_RESET);
      }
    }
  }
 }

 
 if(Serial.available()){
  String inStr = Serial.readStringUntil('\n');
  //Serial.print(inStr);
  if(inStr == "P1"){
    target_feeder = "P1";
  } else if (inStr == "Q1") {
    target_feeder = "Q1";
  } else if (inStr == "*") {
    target_feeder = "*";
  } else if (inStr == "next") {
    fsm.trigger(EVENT_OPEN_REW);
    fsm.trigger(EVENT_STIM_OPEN);
    fsm.trigger(EVENT_READY_STIM);
  } else if (inStr == "reset") {
    err = "Received reset command";
    fsm.trigger(EVENT_READY_RESET);
    fsm.trigger(EVENT_STIM_RESET);
    fsm.trigger(EVENT_OPEN_RESET);
    fsm.trigger(EVENT_REW_RESET);
  } else if (inStr == "retract") {
    retract_P1();
    retract_Q1();
  }
  
 }
}
