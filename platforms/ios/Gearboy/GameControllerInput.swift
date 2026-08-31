import GameController

final class GameControllerInput {
    enum Button: CaseIterable {
        case up
        case down
        case left
        case right
        case a
        case b
        case x
        case y
        case leftShoulder
        case rightShoulder
        case menu
        case options
    }

    var onButtonChanged: ((Button, Bool) -> Void)?

    private var controller: GCController?
    private var observers = [NSObjectProtocol]()
    private var pressedButtons = Set<Button>()
    private var dPadDirections = Set<Button>()
    private var thumbstickDirections = Set<Button>()

    deinit {
        stop()
    }

    func start() {
        guard observers.isEmpty else { return }

        let center = NotificationCenter.default
        observers.append(center.addObserver(
            forName: .GCControllerDidConnect,
            object: nil,
            queue: .main
        ) { [weak self] notification in
            guard let controller = notification.object as? GCController else { return }
            self?.attachIfNeeded(controller)
        })
        observers.append(center.addObserver(
            forName: .GCControllerDidDisconnect,
            object: nil,
            queue: .main
        ) { [weak self] notification in
            guard let self,
                  let disconnectedController = notification.object as? GCController,
                  disconnectedController === self.controller else { return }

            self.detachController()
            if let nextController = GCController.controllers().first(where: { $0 !== disconnectedController }) {
                self.attachController(nextController)
            }
        })

        if let controller = GCController.controllers().first {
            attachController(controller)
        }

        GCController.startWirelessControllerDiscovery(completionHandler: nil)
    }

    func stop() {
        GCController.stopWirelessControllerDiscovery()
        detachController()

        let center = NotificationCenter.default
        for observer in observers {
            center.removeObserver(observer)
        }
        observers.removeAll()
    }

    func releaseAllButtons() {
        dPadDirections.removeAll()
        thumbstickDirections.removeAll()

        let buttons = pressedButtons
        pressedButtons.removeAll()
        for button in buttons {
            onButtonChanged?(button, false)
        }
    }

    private func attachIfNeeded(_ controller: GCController) {
        guard self.controller == nil else { return }
        attachController(controller)
    }

    private func attachController(_ controller: GCController) {
        detachController()
        self.controller = controller
        controller.playerIndex = .index1

        if let gamepad = controller.extendedGamepad {
            bind(gamepad.buttonA, to: .a)
            bind(gamepad.buttonB, to: .b)
            bind(gamepad.buttonX, to: .x)
            bind(gamepad.buttonY, to: .y)
            bind(gamepad.leftShoulder, to: .leftShoulder)
            bind(gamepad.rightShoulder, to: .rightShoulder)
            bind(gamepad.buttonMenu, to: .menu)
            if let buttonOptions = gamepad.buttonOptions {
                bind(buttonOptions, to: .options)
            }

            gamepad.dpad.valueChangedHandler = { [weak self] _, x, y in
                self?.updateDirections(x: x, y: y, isThumbstick: false)
            }
            gamepad.leftThumbstick.valueChangedHandler = { [weak self] _, x, y in
                self?.updateDirections(x: x, y: y, isThumbstick: true)
            }
        } else if let gamepad = controller.microGamepad {
            gamepad.allowsRotation = false
            gamepad.reportsAbsoluteDpadValues = true
            bind(gamepad.buttonA, to: .a)
            bind(gamepad.buttonX, to: .b)
            bind(gamepad.buttonMenu, to: .menu)
            gamepad.dpad.valueChangedHandler = { [weak self] _, x, y in
                self?.updateDirections(x: x, y: y, isThumbstick: false)
            }
        }
    }

    private func detachController() {
        guard let controller else { return }
        clearHandlers(controller)
        releaseAllButtons()
        controller.playerIndex = .indexUnset
        self.controller = nil
    }

    private func clearHandlers(_ controller: GCController) {
        if let gamepad = controller.extendedGamepad {
            gamepad.buttonA.pressedChangedHandler = nil
            gamepad.buttonB.pressedChangedHandler = nil
            gamepad.buttonX.pressedChangedHandler = nil
            gamepad.buttonY.pressedChangedHandler = nil
            gamepad.leftShoulder.pressedChangedHandler = nil
            gamepad.rightShoulder.pressedChangedHandler = nil
            gamepad.buttonMenu.pressedChangedHandler = nil
            gamepad.buttonOptions?.pressedChangedHandler = nil
            gamepad.dpad.valueChangedHandler = nil
            gamepad.leftThumbstick.valueChangedHandler = nil
        }

        if let gamepad = controller.microGamepad {
            gamepad.buttonA.pressedChangedHandler = nil
            gamepad.buttonX.pressedChangedHandler = nil
            gamepad.buttonMenu.pressedChangedHandler = nil
            gamepad.dpad.valueChangedHandler = nil
        }
    }

    private func bind(_ input: GCControllerButtonInput, to button: Button) {
        input.pressedChangedHandler = { [weak self] _, _, pressed in
            self?.setButton(button, pressed: pressed)
        }
    }

    private func updateDirections(x: Float, y: Float, isThumbstick: Bool) {
        let threshold: Float = 0.35
        var directions = Set<Button>()

        if x < -threshold {
            directions.insert(.left)
        } else if x > threshold {
            directions.insert(.right)
        }

        if y < -threshold {
            directions.insert(.down)
        } else if y > threshold {
            directions.insert(.up)
        }

        if isThumbstick {
            thumbstickDirections = directions
        } else {
            dPadDirections = directions
        }

        let activeDirections = dPadDirections.union(thumbstickDirections)
        for button in [Button.up, .down, .left, .right] {
            setButton(button, pressed: activeDirections.contains(button))
        }
    }

    private func setButton(_ button: Button, pressed: Bool) {
        let wasPressed = pressedButtons.contains(button)
        guard wasPressed != pressed else { return }

        if pressed {
            pressedButtons.insert(button)
        } else {
            pressedButtons.remove(button)
        }
        onButtonChanged?(button, pressed)
    }
}
