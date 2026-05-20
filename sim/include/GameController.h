/*! @file GameController.h
 *  @brief Code to read the Logitech F310 Game Controller
 *  Creates a DriverCommand object to be sent to the robot controller
 *  Used in the development simulator and in the robot control mode
 */

#ifndef PROJECT_GAMECONTROLLER_H
#define PROJECT_GAMECONTROLLER_H

#include "SimUtilities/GamepadCommand.h"

#include <QtCore/QObject>

class QGamepad;  // for an unknown reason, #including <QtGamepad/QGamepad> here
                 // makes compilation *very* slow

class GameController : public QObject {
  Q_OBJECT
 public:
  explicit GameController(QObject *parent = 0);
  void updateGamepadCommand(GamepadCommand &gamepadCommand);
  void findNewController();
  ~GameController();
 protected:
  bool eventFilter(QObject *obj, QEvent *event) override;

 private:
  void updateKeyboardAxes();

  QGamepad *_qGamepad = nullptr;

  bool _keyW = false;
  bool _keyS = false;
  bool _keyA = false;
  bool _keyD = false;
  bool _keyQ = false;
  bool _keyE = false;

  float _keyboardLX = 0.f;
  float _keyboardLY = 0.f;
  float _keyboardRX = 0.f;
  float _keyboardRY = 0.f;
};

#endif  // PROJECT_GAMECONTROLLER_H
