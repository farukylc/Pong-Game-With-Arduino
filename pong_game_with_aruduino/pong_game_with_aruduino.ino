#include <SPI.h>
#include <SD.h>
#include <LedControl.h>
#include <virtuabotixRTC.h>


LedControl field = LedControl(6, 5, 4, 2);


virtuabotixRTC myRTC(2,3,7);
File sdkart;
int pinCS = 10; //SD card CS pin
int stopper = 1;

const int p1Pin = A1; //potentiometer for player 1
const int p2Pin = A2; //potentiometer for player 2

int x; // Ball x coordinate
int y; // Ball y coordinate
int fieldId; //we have two 8x8 matrix, left Id/player 1 side is 0 and right Id/player 2 side is 1

boolean rightDir; //TRUE if ball moves from left to right (screen 0 to screen 1
boolean upDir;    //TRUE if ball is moving in up direction (positive slope, row numbers changes ++)

//keep track of player positions - a player occupy two pixels - left and right
int p1Left;
int p1Right;
int p2Left;
int p2Right;

const int initGamePace = 300; //each play in msec. lower the speed, faster th game is played
int gamePace; //game pace is decremented by 1 in each loop/play and gets reset to initGamePace in kickoff

//keep track of score
int p1Score = 0;
int p2Score = 0;

//for debugging: goal line becomes a WALL
boolean p1auto = false;
boolean p2auto = false;

// the setup routine runs once when you press reset:
void setup() {
  pinMode(8,OUTPUT);
  field.shutdown(0, false);
  field.setIntensity(0, 2); //was 3. change to 2
  field.clearDisplay(0);

  field.shutdown(1, false);
  field.setIntensity(1, 2); //was 3. change to 2
  field.clearDisplay(1);

  // initialize serial communication at 9600 bits per second:
  
  Serial.begin(9600);
  pinMode(pinCS, OUTPUT);
  if(SD.begin()){
    Serial.println("Sd kart tanındı");
    
    
    }
   else{

    Serial.println("SD kart bulunamadı");
    return;
    
       
    
    }





  
  randomSeed(analogRead(0)); // nothing connected to 0 so read sees noise

  displayPong();
  delay(2000);

  
  // start the game
  kickoff(1, p1Score, p2Score);  //
}



// the loop routine runs over and over again forever:
void loop() {

  while ((p1Score < 5) && (p2Score < 5)) {

    

    displayBall();

    displayPlayers();

    evaluatePlay();

    moveBall(); //this doesn't actually "move" the ball but set the cooridinates and directions of a ball

    //in each play, gamePace is decremented by 1 to speed up the game until zero
    if (gamePace > 0) {
      gamePace--;
    }    delay(gamePace);

    //clear field before new play
    field.clearDisplay(0);
    field.clearDisplay(1);

  }

  //end the game - reset to play gain
  displayEnd();
  delay(1000);

  myRTC.updateTime();
  Serial.print(myRTC.dayofmonth);
  Serial.print("/");
  Serial.print(myRTC.month);
  Serial.print("/");
  Serial.print(myRTC.year);
  Serial.print("  ");
  Serial.print(myRTC.hours);
  Serial.print(":");
  Serial.print(myRTC.minutes);
  Serial.print(":");
  Serial.println(myRTC.seconds);
  delay(1000);

  sdkart = SD.open("data.txt", FILE_WRITE);
  if(sdkart && stopper  == 1){

    
    sdkart.print("******************************************************************************* \n");
    sdkart.println("Oyunun Oynandığı Tarih  "  +String(myRTC.dayofmonth)+"." +String(myRTC.month)+"."  + String(myRTC.year)+"\n");
    sdkart.println("Oyunun Oynandığı Saat  " + String(myRTC.hours)+ ":"  + String(myRTC.minutes)+ ":" +String(myRTC.seconds));
    sdkart.println("Skor: " + String(p1Score)+"-"+String(p2Score)); 
    sdkart.println("*******************************************************************************");
    sdkart.close();
    
    stopper = stopper +1;
    



  }

  else{

    Serial.print("bitti");
  }

 
  
}

void displayPong () {
  const byte PO[8] = {
    B01111110, //PO
    B00010010,
    B00001100,
    B00000000,
    B00111100,
    B01000010,
    B00111100,
    B00000000,
  };

    const byte NG[8] = {
    B01111110, //NG
    B00001100,
    B00110000,
    B01111110,
    B00000000,
    B00111100,
    B01010010,
    B01110100
  };


  for (int i = 0; i < 8; i++) {
    field.setRow(0, i, PO[i]);
  }

    for (int i = 0; i < 8; i++) {
    field.setRow(1, i, NG[i]);
  }
  

}

void displayEnd () {
  const byte PO[8] = {
    B00000000, //EN
    B01111110,
    B01001010,
    B01001010,
    B01000010,
    B00000000,
    B01111110, 
    B00001100,
  };

    const byte NG[8] = {
    B00110000,
    B01111110,
    B00000000,
    B01111110,
    B01000010,
    B00100100,
    B00011000,
  };


  for (int i = 0; i < 8; i++) {
    field.setRow(0, i, PO[i]);
  }

    for (int i = 0; i < 8; i++) {
    field.setRow(1, i, NG[i]);
  }
  

}


void showScore (int fieldId, int score) {

  //numbers 0 to 9 on 8x8 pixel
  const byte number[10][8] = {
    {
      B00000000, //0
      B00000000,
      B00111100,
      B01000010,
      B01000010,
      B00111100,
      B00000000,
      B00000000
    }, {
      B00000000,  //1
      B00000000,
      B00000000,
      B01000100,
      B01111110,
      B00000000,
      B00000000,
      B00000000
    }, {
      B00000000,  //2
      B00000000,
      B01000100,
      B01100010,
      B01010010,
      B01001100,
      B00000000,
      B00000000
    }, {
      B00000000,  //3
      B00000000,
      B00100010,
      B01001010,
      B01010110,
      B00100010,
      B00000000,
      B00000000
    }, {
      B00000000,  //4
      B00000000,
      B00010000,
      B00011000,
      B00010100,
      B01111110,
      B00000000,
      B00000000
    }, {
      B00000000,  //5
      B00000000,
      B00101110,
      B01001010,
      B01001010,
      B00110010,
      B00000000,
      B00000000
    }, {
      B00000000,  //6
      B00000000,
      B00111100,
      B01001010,
      B01001010,
      B00110010,
      B00000000,
      B00000000
    }, {
      B00000000,  //7
      B00000000,
      B00000010,
      B00000010,
      B01110010,
      B00001110,
      B00000000,
      B00000000
    }, {
      B00000000,  //8
      B00000000,
      B00110100,
      B01001010,
      B01001010,
      B00110100,
      B00000000,
      B00000000
    }, {
      B00000000,  //9
      B00000000,
      B00001100,
      B01010010,
      B01010010,
      B00111100,
      B00000000,
      B00000000
    }
    
  };

  for (int i = 0; i < 8; i++) {
    field.setRow(fieldId, i, number[score][i]);

  }
}



void kickoff(int player, int p1Score, int p2Score) {

  delay(500);
  //  Serial.print("@kickoff: player =");
  //  Serial.println(player);
  //  Serial.print("score: ");
  //  Serial.print(p1Score);
  //  Serial.print("-");
  //  Serial.println(p2Score);

  showScore(0, p1Score);
  showScore(1, p2Score);

  delay(1500);

  field.clearDisplay(0);
  field.clearDisplay(1);

  //set kick-off ball position
  if (player == 1) { //player 1 kick-off
    x = 7;
    y = 3;
    fieldId = 0;
    rightDir = true; //ball moves from left to right
  } else {  //player 2 kick off
    x = 0;
    y = 4;
    fieldId = 1;
    rightDir = false; //ball moves from left to right
  }

  //boolean upDir; //TRUE if ball is moving in up direction (positive slope)
  upDir = random(0, 2); // generate random number between 1 & 2

  //reset gamePace to initialGamePace
  gamePace = initGamePace;


}

int playerLoc(int player) {

  //returned value should be 0 to 1024 between (0 and 5v)
  int sensorValue;

  if (player == 1) {
    sensorValue = analogRead(p1Pin);
  } else {
    sensorValue = analogRead(p2Pin);
  }
  //convert potentiometer values to x value betwen 0 and 6
  int xval = map(sensorValue, 220, 780, 0, 6);
  xval = constrain (xval, 0, 6);
  return xval;
}

//display ball location
void displayBall() {
  field.setLed(fieldId, x, y, true); //init 2,4
}

//display player locations
void displayPlayers() {

  //find & set player position by reading potentiometer
  p1Right = playerLoc(1);
  p1Left = p1Right + 1;

  p2Left = playerLoc(2);
  p2Right = p2Left + 1;

  //display goalies
  field.setLed(0, 0, p1Left, true); //p1 goalie
  field.setLed(0, 0, p1Right, true); //p1 goalie

  field.setLed(1, 7, p2Left, true); //p2 goalie
  field.setLed(1, 7, p2Right, true); //p2 goalie

  //display strikers
  
}

//evaluate play based on ball & players' locations
void evaluatePlay() {

  //@ p1 goal line
  if ((x == 0) && (fieldId == 0)) { //player 2 goal
    //Serial.println("****** player 2 gooooal ********: ");
    digitalWrite(8, HIGH);
    delay(100);
    digitalWrite(8, LOW);
    delay(300);
    p2Score++;
    kickoff(1, p1Score, p2Score); //player 1 to kick off
  }

  if ((x == 1) && (fieldId == 0)) { //ball at the goal zone, check if goalie is right in front of the ball
    if (p1auto) { //for debugging: p1 always blocks
      rightDir = true;
    } else {
      if ((y == p1Left) or (y == p1Right)) {  //blocked
        rightDir = true;
      } else if (((y == (p1Right - 1)) && (upDir)) or ((y == p1Left + 1) && (!upDir))) { //TODO: combine 2 and 3 below
        rightDir = true;
        upDir = !upDir;
      } else {
        //Serial.print("goalie misses");
      }
    }
  }

  //ball in front of p2 striker
  

  //ball on p2 striker so deflect the ball
  

  if ((x == 6) && (fieldId == 1)) { //ball at the goal zone, check if goalie is right in front of the ball
    if (p2auto) { //for debugging: p2 always blocks
      rightDir = false;
    } else {
      if ((y == p2Left) or (y == p2Right)) {  //ball is right in front of the goalie
        //Serial.println("ball blocked");
        rightDir = false;  //change direction
      } else if (((y == (p2Right + 1)) && (!upDir)) or ((y == (p2Left - 1)) && (upDir))) { //TODO: combine 2 and 3 below
        //Serial.println("ball blocked - case #2 or #3");
        rightDir = false;  //change direction
        upDir = !upDir;
      } else {
        //Serial.print("goalie misses");
      }
    }
  }

  //@ p2 goal line
  if ((x == 7) && (fieldId == 1)) { //goal
    digitalWrite(8, HIGH);
    delay(100);
    digitalWrite(8, LOW);
    delay(300);
    p1Score++;
    kickoff(2, p1Score, p2Score); //player 2 to kick off
  }

}

//set the new location of ball & its directions
void moveBall() {

  //*************** check y coordinate
  //deal with ball hitting the side lines
  if (y == 7) { //you hit the right goal line, change direction
    //Serial.println("hit the right side line");
    digitalWrite(8, HIGH);
    delay(100);
    digitalWrite(8, LOW);
    delay(300);
    
    upDir = false;  //move from right to left
  }

  if (y == 0) { //ball hit the left goal line, change direction
    //Serial.println("hit the left side line");
    digitalWrite(8, HIGH);
    delay(100);
    digitalWrite(8, LOW);
    delay(300);
    upDir = true;
  }

  //change x-coordinate
  if (rightDir) { // ball moving from left to right
    if ((x == 7) && (fieldId == 0)) {
      //Serial.println("crossing half line");
      x = 0 ;
      fieldId = 1;
    } else {  //within the screeen boundary, keep increasing x
      //Serial.println("increase x");
      x++;
    }
  } else { //ball moving from right to left
    if ((x == 0) && (fieldId == 1)) {
      //Serial.println("crossing half line");
      x = 7;
      fieldId = 0;
    } else {
      //Serial.println("increase x");
      x--;
    }
  }

  //change y-coordinate
  if (upDir) {
    y++;
  } else {
    y--;
  }
}
