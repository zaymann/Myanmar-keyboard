import Cocoa
import InputMethodKit

// Entry point for the input method server.
// The connection name MUST match `InputMethodConnectionName` in Info.plist.

let kConnectionName = "MyanmarIME_1_Connection"

let bundleID = Bundle.main.bundleIdentifier ?? "com.zaymann.MyanmarIME"

guard let server = IMKServer(name: kConnectionName, bundleIdentifier: bundleID) else {
    NSLog("MyanmarIME: failed to create IMKServer")
    exit(1)
}
_ = server // keep a strong reference for the lifetime of the process

NSLog("MyanmarIME: server started (\(bundleID))")
NSApplication.shared.run()
