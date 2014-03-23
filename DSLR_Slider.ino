#include <AFMotor.h>

// On my camera cable, yellow=tip, red=base, white=
// Begin: user-changeable values
int TAKE_PHOTO_PIN = 47;
int MOTOR_PORT_1 = 1;
int SLIDE_STOPPER_PIN_1 = 48;
int SLIDE_STOPPER_PIN_2 = 49;
int LED_PIN = 38;

// 128, 300 -> About 102 shots. Some stick.
// 255, 200 -> 46 shots. No sticking.
// 255, 100 -> 92 shots. No sticking.
// 255,  50 -> 178 shots. No sticking.
// 255,  25 -> 364 shots. No sticking.
// 230,  13 -> 1697 shots.
int MOTOR_SPEED_FOR_PHOTO = 255;          // The speed at which the motor travels between pictures.  1...255, 255 is fastest.  Non-linear.
int MOTOR_TRAVEL_TIME_FOR_PHOTO = 25;     // How long to run the motor between each pair of photos.  In milliseconds.
int MOTOR_SETTLE_TIME = 1000;             // How long to wait between stopping the motor and taking a picture, to avoid vibration, etc.  In milliseconds.
int CAMERA_SHUTTER_TIME = 250;            // How long to hold down the camera's shutter button.  In milliseconds.
int TIME_BETWEEN_PHOTOS = 30000;          // How long to wait between beginning to take one photo, and beginning to take the next photo (thus, this includes MOTOR_TRAVEL_TIME, MOTOR_SETTLE_TIME, and CAMERA_SHUTTER_TIME!).  In milliseconds.

int MOTOR_SPEED_FOR_RESET = 255;          // The speed at which the motor travels when it is resetting its position, prior to starting to take pictures. 1...255, 255 is fastest.  Non-linear.
int MOTOR_TRAVEL_TIME_FOR_RESET = 500;    // How long to run the motor (between testing the switch state) while resetting its position

int DELAY_AFTER_RESET = 2000;             // How long to wait after resetting the motor's position, before starting to take pictures.  In milliseconds.
// End: user-changeable values

int STATE_RESET_MOTOR = 1;
int STATE_DONE_RESETTING_POSITION = 2;
int STATE_TAKING_PICTURES = 3;
int STATE_DONE = 4;
int state = STATE_RESET_MOTOR;

int POST_PHOTO_DELAY_TIME;
AF_DCMotor motor(MOTOR_PORT_1, MOTOR12_1KHZ);
int motorDir = BACKWARD;
int photoNum = 0;

void setup() {
    pinMode(TAKE_PHOTO_PIN, OUTPUT);
    pinMode(SLIDE_STOPPER_PIN_1, INPUT);
    pinMode(SLIDE_STOPPER_PIN_2, INPUT);
    digitalWrite(TAKE_PHOTO_PIN, LOW);
    motor.setSpeed(MOTOR_SPEED_FOR_RESET);
    POST_PHOTO_DELAY_TIME = max(TIME_BETWEEN_PHOTOS - MOTOR_TRAVEL_TIME_FOR_PHOTO - MOTOR_SETTLE_TIME - CAMERA_SHUTTER_TIME, 0);
    Serial.begin(9600);
}

void loop() {
    printStopStates();
    if (state == STATE_RESET_MOTOR) {
        Serial.println("STATE_RESET_MOTOR");
        if (reachedBeginningOfSlide()) {
            state = STATE_DONE_RESETTING_POSITION;
        } else {
            advanceSlideForReset();
        }
    } else if (state == STATE_DONE_RESETTING_POSITION) {
        Serial.println("STATE_RESETTING_POSITION");
        blinkLed();
        delay(DELAY_AFTER_RESET);
        blinkLed();
        Serial.println("Done waiting.");
        motorDir = FORWARD;
        motor.setSpeed(MOTOR_SPEED_FOR_PHOTO);
        Serial.println("Changed motor parameters.");
        state = STATE_TAKING_PICTURES;
    } else if (state == STATE_TAKING_PICTURES) {
        Serial.println("STATE_TAKING_PICTURES");
        takePhoto();
        advanceSlideForPhoto();
        delay(POST_PHOTO_DELAY_TIME);
        if (reachedEndOfSlide()) {
            takePhoto();
            state = STATE_DONE;
            Serial.println("STATE_DONE");
        }
    } else {
        blinkLed();
    }
}

void advanceSlideForReset() {
    advanceSlide(MOTOR_TRAVEL_TIME_FOR_RESET, false);
}

void advanceSlideForPhoto() {
    advanceSlide(MOTOR_TRAVEL_TIME_FOR_PHOTO, true);
}

// Let the motor travel
void advanceSlide(int travelTime, boolean doDelay) {
    Serial.print("Moving for ");
    Serial.println(travelTime);
    motor.run(motorDir);
    delay(travelTime);
    motor.run(RELEASE);
    if (doDelay) {
        delay(MOTOR_SETTLE_TIME);
    }
}

// Take a picture
void takePhoto() {
    Serial.print("Smile for picture #");
    Serial.println(photoNum++);
    digitalWrite(TAKE_PHOTO_PIN, HIGH);
    delay(CAMERA_SHUTTER_TIME);
    digitalWrite(TAKE_PHOTO_PIN, LOW);
}

boolean reachedBeginningOfSlide() {
    return (HIGH == digitalRead(SLIDE_STOPPER_PIN_1));
}

boolean reachedEndOfSlide() {
    return (HIGH == digitalRead(SLIDE_STOPPER_PIN_2));
}

void blinkLed() {
    digitalWrite(LED_PIN, HIGH);
    delay(250);
    digitalWrite(LED_PIN, LOW);
    delay(250);
}

void printStopStates() {
    int stop1 = digitalRead(SLIDE_STOPPER_PIN_1);
    int stop2 = digitalRead(SLIDE_STOPPER_PIN_2);
    Serial.print("stop1=");
    Serial.print(stop1);
    Serial.print(", stop2=");
    Serial.print(stop2);
    Serial.print(", LOW=");
    Serial.print(LOW);
    Serial.print(", HIGH=");
    Serial.println(HIGH);
}


