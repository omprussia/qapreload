# qapreload

## execute_command custom methods list

### app:pullDownTo

perform pulldown action on view. if view is not at beginning it will scroll to top, then pull down.

Usage:

`driver.execute_command("app:pullDownTo", "some text label")`

or

`driver.execute_command("app:pullDownTo", index)`

### app:pushUpTo

perform pushup action on view. if view is not at end it will scroll to end, then push up.

Usage:

`driver.execute_command("app:pushUpTo", "some text label")`

or

`driver.execute_command("app:pushUpTo", index)`

### app:clickContextMenuItem

perform context menu click on item

Usage:

`driver.execute_command("app:clickContextMenuItem", "ContextMenu_0x12345678", "some text label")`

or

`driver.execute_command("app:clickContextMenuItem", "ContextMenu_0x12345678", index)`

`"ContextMenu_0x12345678"` is element.id, you should find element before using this method

### app:waitForPageChange

synchronously wait for page change

Usage:

`driver.execute_command("app:waitForPageChange")`

### app:swipe

perform swipe action in selected direction

Usage:

`driver.execute_command("app:swipe", "left")`

Allowed directions are: left, right, top, bottom

### app:peek

perform peek action in selected direction

Usage:

`driver.execute_command("app:peek", "left")`

Allowed directions are: left, right, top, bottom

### app:goBack

perform back navigation action on current page

Usage:

`driver.execute_command("app:goBack")`

### app:goForward

perform back navigation action on current page

Usage:

`driver.execute_command("app:goForward")`

### app:enterCode

type code on KeyPad item. You can use `#` for confirm action and `*` for cancel action

Usage:

`driver.execute_command("app:enterCode", "12345#")`

### touch:pressAndHold

perform press and hold touch action on choosen coordinates

Usage:

`driver.execute_command("touch:pressAndHold", 20, 40)`

### touch:mouseSwipe

perform mouse swipe touch action from one point to another

Usage:

`driver.execute_command("touch:mouseSwipe", 20, 40, 60, 80)`

### app:scrollToItem

perform scroll on view to some item

Usage:

`driver.execute_command("app:scrollToItem", "ContextMenu_0x12345678")`

`"ContextMenu_0x12345678"` is element.id, you should find element before using this method

### app:method

invoke meta method on element

Usage:

`driver.execute_command("app:method", "MyItem_0x12345678", "myFunction", ["some", "args", 15])`

`"MyItem_0x12345678"` is element.id, you should find element before using this method

### app:js

execute js code in element context

Usage:

`driver.execute_command("app:js", "MyItem_0x12345678", "function() { return "hello!"; }"`

`"MyItem_0x12345678"` is element.id, you should find element before using this method

### app:dumpCurrentPage

dump current page items tree

Usage:

`driver.execute_command("app:dumpCurrentPage""`

### app:dumpTree

dump application items tree

Usage:

`driver.execute_command("app:dumpTree")`

### system:shell

executes command with root privileges. use with caution

Usage:

`driver.execute_command("system:shell", "ls", ["-la", "/"])`

