import Cocoa
import InputMethodKit

/// The input controller. One instance per text input session.
///
/// As the user types Latin letters we accumulate them in `buffer`, show the
/// live Myanmar conversion as marked (underlined) text, and commit it when the
/// user presses space / return / punctuation.
@objc(InputController)
class InputController: IMKInputController {

    private var buffer = ""

    // MARK: - Key handling

    override func handle(_ event: NSEvent!, client sender: Any!) -> Bool {
        guard let event = event, event.type == .keyDown else { return false }
        guard let client = sender as? IMKTextInput else { return false }

        let flags = event.modifierFlags
        // Let the system handle command/control shortcuts.
        if flags.contains(.command) || flags.contains(.control) {
            commit(client)
            return false
        }

        let keyCode = event.keyCode

        // Backspace (delete)
        if keyCode == 51 {
            if !buffer.isEmpty {
                buffer.removeLast()
                updateMarked(client)
                return true
            }
            return false
        }

        // Return / Enter / Tab / Escape -> commit then let the key act normally
        if keyCode == 36 || keyCode == 76 || keyCode == 48 || keyCode == 53 {
            if !buffer.isEmpty {
                commit(client)
                return keyCode == 53 // swallow only Escape
            }
            return false
        }

        let chars = event.characters ?? ""
        guard let ch = chars.first else { return false }

        // Romanization input characters: a-z, A-Z, '.', ':'
        if isRomanInput(ch) {
            buffer.append(ch)
            updateMarked(client)
            return true
        }

        // Anything else (space, punctuation, digits): commit buffer, then emit
        // the typed character ourselves so ordering is correct.
        if !buffer.isEmpty {
            let converted = Romanizer.convert(buffer)
            buffer = ""
            client.insertText(converted + chars,
                              replacementRange: NSRange(location: NSNotFound, length: 0))
            return true
        }
        return false
    }

    private func isRomanInput(_ ch: Character) -> Bool {
        if ch == "." || ch == ":" { return true }
        return ("a"..."z").contains(ch) || ("A"..."Z").contains(ch)
    }

    // MARK: - Composition display

    private func updateMarked(_ client: IMKTextInput) {
        if buffer.isEmpty {
            client.setMarkedText("",
                                 selectionRange: NSRange(location: 0, length: 0),
                                 replacementRange: NSRange(location: NSNotFound, length: 0))
            return
        }
        let converted = Romanizer.convert(buffer)
        let attr = NSAttributedString(
            string: converted,
            attributes: [
                .underlineStyle: NSUnderlineStyle.single.rawValue,
            ])
        client.setMarkedText(attr,
                             selectionRange: NSRange(location: converted.utf16.count, length: 0),
                             replacementRange: NSRange(location: NSNotFound, length: 0))
    }

    private func commit(_ client: IMKTextInput) {
        guard !buffer.isEmpty else { return }
        let converted = Romanizer.convert(buffer)
        buffer = ""
        client.insertText(converted,
                          replacementRange: NSRange(location: NSNotFound, length: 0))
    }

    // MARK: - Lifecycle

    override func commitComposition(_ sender: Any!) {
        if let client = sender as? IMKTextInput { commit(client) }
    }

    override func deactivateServer(_ sender: Any!) {
        if let client = sender as? IMKTextInput { commit(client) }
        super.deactivateServer(sender)
    }
}
