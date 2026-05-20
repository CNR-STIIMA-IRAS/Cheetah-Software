/*! @file GameController.cpp
 *  @brief Code to read the Logitech F310 Game Controller
 *  Creates a DriverCommand object to be sent to the robot controller
 *  Used in the development simulator and in the robot control mode
 *
 *  NOTE: Because QT is weird, the updateDriverCommand has to be called from a
 * QT event. Running it in another thread will cause it to not work. As a
 * result, this only works if called in the update method of a QTObject
 */

#include "GameController.h"

#include <QtCore/QObject>
#include <QtGamepad/QGamepad>
#include <QApplication>
#include <QEvent>
#include <QKeyEvent>

/*!
 * By default, the game controller selects the "first" joystick, printing a
 * warning if there are multiple joysticks On Linux, this is /dev/input/js0 If
 * no joystick is found, it will print an error message, and will return zero.
 * It is possible to change/add a joystick later with findNewController
 */
GameController::GameController(QObject *parent) : QObject(parent) {
  if (qApp) {
    qApp->installEventFilter(this);
  }

  findNewController();
}

static void printKeyboardTeleopHelpOnce() {
  static bool printed = false;

  if (printed) {
    return;
  }

  printed = true;

  printf("\n");
  printf("[Keyboard Teleop] No gamepad detected. Using keyboard fallback.\n");
  printf("[Keyboard Teleop] Click on the simulator window first, then use:\n");
  printf("[Keyboard Teleop]   W       : move forward\n");
  printf("[Keyboard Teleop]   S       : move backward\n");
  printf("[Keyboard Teleop]   A       : strafe left\n");
  printf("[Keyboard Teleop]   D       : strafe right\n");
  printf("[Keyboard Teleop]   Q       : turn left\n");
  printf("[Keyboard Teleop]   E       : turn right\n");
  printf("[Keyboard Teleop]   SPACE   : stop command\n");
  printf("\n");
  fflush(stdout);
}
/*!
 * Re-run the joystick finding code to select the "first" joystick. This can be
 * used to set up the joystick if the simulator is started without a joystick
 * plugged in
 */
void GameController::findNewController() {
  delete _qGamepad;
  _qGamepad = nullptr;  // in case this doesn't work!

  printf("[Gamepad] Searching for gamepads, please ignore \"Device discovery cannot open device\" errors\n");
  auto gamepadList = QGamepadManager::instance()->connectedGamepads();
  printf("[Gamepad] Done searching for gamepads.\n");
  if (gamepadList.empty()) {
    printf("[GameController] No physical gamepad connected. Using keyboard fallback.\n");
    printKeyboardTeleopHelpOnce();
  } else {
    if (gamepadList.size() > 1) {
      printf(
          "[ERROR: GameController] There are %d joysticks connected.  Using "
          "the first one.\n",
          gamepadList.size());
    } else {
      printf("[GameController] Found 1 joystick\n");
    }

    _qGamepad = new QGamepad(*gamepadList.begin());
  }
}

/*!
 * Overwrite a driverCommand with the current joystick state.  If there's no
 * joystick, sends zeros
 * TODO: what happens if the joystick is unplugged?
 */
void GameController::updateGamepadCommand(GamepadCommand &gamepadCommand) {
  if (_qGamepad) {
    gamepadCommand.leftBumper = _qGamepad->buttonL1();
    gamepadCommand.rightBumper = _qGamepad->buttonR1();
    gamepadCommand.leftTriggerButton = _qGamepad->buttonL2() != 0.;
    gamepadCommand.rightTriggerButton = _qGamepad->buttonR2() != 0.;
    gamepadCommand.back = _qGamepad->buttonSelect();
    gamepadCommand.start = _qGamepad->buttonStart();
    gamepadCommand.a = _qGamepad->buttonA();
    gamepadCommand.b = _qGamepad->buttonB();
    gamepadCommand.x = _qGamepad->buttonX();
    gamepadCommand.y = _qGamepad->buttonY();
    gamepadCommand.leftStickButton = _qGamepad->buttonL3();
    gamepadCommand.rightStickButton = _qGamepad->buttonR3();
    gamepadCommand.leftTriggerAnalog = (float)_qGamepad->buttonL2();
    gamepadCommand.rightTriggerAnalog = (float)_qGamepad->buttonR2();
    gamepadCommand.leftStickAnalog =
        Vec2<float>(_qGamepad->axisLeftX(), -_qGamepad->axisLeftY());
    gamepadCommand.rightStickAnalog =
        Vec2<float>(_qGamepad->axisRightX(), -_qGamepad->axisRightY());
  } else {
    gamepadCommand.zero();

    gamepadCommand.leftStickAnalog = Vec2<float>(_keyboardLX, _keyboardLY);
    gamepadCommand.rightStickAnalog = Vec2<float>(_keyboardRX, _keyboardRY);
  }

  // printf("%s\n", gamepadCommand.toString().c_str());
}

void GameController::updateKeyboardAxes() {
  constexpr float linear = 0.35f;
  constexpr float yaw = 0.35f;

  _keyboardLX = 0.f;
  _keyboardLY = 0.f;
  _keyboardRX = 0.f;
  _keyboardRY = 0.f;

  if (_keyW) _keyboardLY += linear;
  if (_keyS) _keyboardLY -= linear;

  if (_keyA) _keyboardLX -= linear;
  if (_keyD) _keyboardLX += linear;

  if (_keyQ) _keyboardRX -= yaw;
  if (_keyE) _keyboardRX += yaw;
}

bool GameController::eventFilter(QObject *obj, QEvent *event) {
  if (event->type() != QEvent::KeyPress &&
      event->type() != QEvent::KeyRelease) {
    return QObject::eventFilter(obj, event);
  }

  QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

  if (keyEvent->isAutoRepeat()) {
    return QObject::eventFilter(obj, event);
  }

  const bool pressed = event->type() == QEvent::KeyPress;

  switch (keyEvent->key()) {
    case Qt::Key_W:
      _keyW = pressed;
      break;

    case Qt::Key_S:
      _keyS = pressed;
      break;

    case Qt::Key_A:
      _keyA = pressed;
      break;

    case Qt::Key_D:
      _keyD = pressed;
      break;

    case Qt::Key_Q:
      _keyQ = pressed;
      break;

    case Qt::Key_E:
      _keyE = pressed;
      break;

    case Qt::Key_Space:
      _keyW = _keyS = _keyA = _keyD = _keyQ = _keyE = false;
      break;

    default:
      return QObject::eventFilter(obj, event);
  }

  updateKeyboardAxes();

  return QObject::eventFilter(obj, event);
}

GameController::~GameController() {
  if (qApp) {
    qApp->removeEventFilter(this);
  }

  delete _qGamepad;
}