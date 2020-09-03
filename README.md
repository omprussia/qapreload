# qapreload - Library for automated testing Aurora OS, Sailfish OS, Qt widgets and Qt QML application using Appium framework

Library implements common subset of commands to perform UI operations

Library must be injected into application. bridge is responsible for injecting while launching applications via Appium framework. For Aurora OS and Sailfish OS bridge is installed via rpm package and automatically listening for incoming communications from Appium driver on 8888 port. On other platforms you should build project and run bridge target manually.

## Aurora OS and Sailfish OS specific execute_script methods list

### app:pullDownTo

perform pulldown action on view. if view is not at beginning it will scroll to top, then pull down.

Usage:

`driver.execute_script("app:pullDownTo", "some text label")`

or

`driver.execute_script("app:pullDownTo", index)`

### app:pushUpTo

perform pushup action on view. if view is not at end it will scroll to end, then push up.

Usage:

`driver.execute_script("app:pushUpTo", "some text label")`

or

`driver.execute_script("app:pushUpTo", index)`

### app:clickContextMenuItem

perform context menu click on item

Usage:

`driver.execute_script("app:clickContextMenuItem", "ContextMenu_0x12345678", "some text label")`

or

`driver.execute_script("app:clickContextMenuItem", "ContextMenu_0x12345678", index)`

`"ContextMenu_0x12345678"` is element.id, you should find element before using this method

### app:waitForPageChange

synchronously wait for page change

Usage:

`driver.execute_script("app:waitForPageChange", 1000)`

1000 - is timeout to wait for change or continue anyway

### app:waitForPropertyChange

synchronously wait for element property value change

Usage:

`driver.execute_script("app:waitForPropertyChange", "ContextMenu_0x12345678", "opened", true, 10000)`

`"ContextMenu_0x12345678"` is element.id, you should find element before using this method

You can use None as property value to wait for any property change, or exact value to watch for.

10000 - is timeout to wait for change or continue anyway

### app:swipe

perform swipe action in selected direction

Usage:

`driver.execute_script("app:swipe", "left")`

Allowed directions are: left, right, up, down

### app:peek

perform peek action in selected direction

Usage:

`driver.execute_script("app:peek", "left")`

Allowed directions are: left, right, up, down

### app:goBack

perform back navigation action on current page

Usage:

`driver.execute_script("app:goBack")`

### app:goForward

perform back navigation action on current page

Usage:

`driver.execute_script("app:goForward")`

### app:enterCode

type code on KeyPad item. You can use `#` for confirm action and `*` for cancel action

Usage:

`driver.execute_script("app:enterCode", "12345#")`

### touch:pressAndHold

perform press and hold touch action on choosen coordinates

Usage:

`driver.execute_script("touch:pressAndHold", 20, 40)`

### touch:mouseSwipe

perform mouse swipe touch action from one point to another

Usage:

`driver.execute_script("touch:mouseSwipe", 20, 40, 60, 80)`

### touch:mouseDrag

perform mouse drag touch action from one point to another

Usage:

`driver.execute_script("touch:mouseDrag", 20, 40, 60, 80)`

### app:scrollToItem

perform scroll on view to some item

Usage:

`driver.execute_script("app:scrollToItem", "ContextMenu_0x12345678")`

`"ContextMenu_0x12345678"` is element.id, you should find element before using this method

### app:method

invoke meta method on element

Usage:

`driver.execute_script("app:method", "MyItem_0x12345678", "myFunction", ["some", "args", 15])`

`"MyItem_0x12345678"` is element.id, you should find element before using this method

### app:js

execute js code in element context

Usage:

`driver.execute_script("app:js", "MyItem_0x12345678", "function() { return "hello!"; }"`

`"MyItem_0x12345678"` is element.id, you should find element before using this method

### app:setAttribute

set attribute value in element

Usage:

`driver.execute_script("app:setAttribute", "MyItem_0x12345678", "attribute_name", "value")`

`"MyItem_0x12345678"` is element.id, you should find element before using this method

### app:saveScreenshot

Save a screenshot with the specified file name

Usage:

`driver.execute_script("app:saveScreenshot", "test.png")`

### app:dumpCurrentPage

dump current page items tree

Usage:

`driver.execute_script("app:dumpCurrentPage""`

### app:dumpTree

dump application items tree

Usage:

`driver.execute_script("app:dumpTree")`

### system:shell

executes script with root privileges. use with caution

Usage:

`driver.execute_script("system:shell", "ls", ["-la", "/"])`

