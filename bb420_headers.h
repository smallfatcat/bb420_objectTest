#define BUT_DELAY   500
#define BUT_REPEAT  50

#define MODE_AUTO   0
#define MODE_MAN    1
#define MODE_SPEED  2
#define MODE_DELAY  3
#define MAX_MODES   3

// Timer Vars
#define UP  1
#define DOWN  0

class Motor {
  public:
    Motor();
    Motor(long initSpeed, int initDirection);
    bool          serialOut();
    bool          isActive() {return active;}
    int           getPos() {return position;}
    int           getSpeed() {return speed;}
    int           getDir() {return direction;}
    int           setSpeed(int newSpeed);
    int           setDirection(int newDirection);
    int           setPosition(long newPosition);
    bool          activate();
    bool          deactivate();
    volatile long pulseCount = 0;
    volatile int  loopCount = 0;
    int           targetLoopCount = 0;
    protected:
    int speed = 300;
    int  direction = 0;
    bool active = false;
    int position = 0;
};

class Button {
  public:
    Button();
    Button(int buttonPin);
    int state() {return s;}
    bool updateButton();
    bool reset = true;
    unsigned long pressedTime = 0;
    bool buttonInit(int buttonPin);
    bool debug();
  protected:
    int s, pin, lastState = HIGH;
    unsigned long debounceTimer = 0, debounceDelay = 50;
};

class Controller {
  public:
    Controller(int A, int B, int C, int D, int speed, int direction, unsigned long d);
    bool getButtonStates();
    bool logic();
    int nextMode();
    bool serialOut();
    String displayLCD0();
    String displayLCD1();
    Button butA;
    Button butB;
    Button butC;
    Button butD;
    Motor motor1;
    int mode;
    unsigned long delayTime;
    unsigned long delayNew;
    bool delayDirty = false;
    int speedNew;
    boolean speedDirty = false;
  protected:
    int state;
};
