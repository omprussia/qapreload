# qapreload - Library for automated testing Aurora OS, Sailfish OS, Qt widgets and Qt QML application using Appium framework

Library implements common subset of commands to perform UI operations

Library must be injected into application. bridge is responsible for injecting while launching applications via Appium framework. For Aurora OS and Sailfish OS bridge is installed via rpm package and automatically listening for incoming communications from Appium driver on 8888 port. On other platforms you should build project and run bridge target manually.

## Run tests

### Prerequisites

- Build and run `appium-aurora` docker container

### For Aurora OS and Sailfish OS targets

- Build and install `qapreload` rpm to device

### For Desktop targets

- Build and run `bridge` from this project

	Extra: if you running Linux-based distro, need to manually copy built `libqapreloadhook.so` and `libqaengine.so` to libdir (usually `/usr/lib` or `/usr/lib64`), otherwise bridge won't be able to automatically inject library to applciation.

Note: your application and `qapreload` should be built with same Qt versions

### Python example

To initialize Appium driver use following example:

```
#!/usr/bin/env python3

from appium import webdriver

def get_driver(device = '192.168.1.248', app = '/usr/bin/aurora-test-example', auto_launch = True):
    appium_socket = 'http://localhost:4723/wd/hub'
    data = {
        'platformName': 'Aurora',
        'appPackage': app,
        'deviceName': device,
        'autoLaunch': auto_launch,
    }
    return webdriver.Remote(appium_socket, data)

driver = get_driver()
```

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

### touch:pressAndHold (deprecated, use TouchAction instead)

perform press and hold touch action on choosen coordinates

Usage:

`driver.execute_script("touch:pressAndHold", 20, 40)`

### touch:mouseSwipe (deprecated, use TouchAction instead)

perform mouse swipe touch action from one point to another

Usage:

`driver.execute_script("touch:mouseSwipe", 20, 40, 60, 80)`

### touch:mouseDrag (deprecated, use TouchAction instead)

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

`driver.execute_script("app:dumpCurrentPage")`

### app:dumpTree

dump application items tree

Usage:

`driver.execute_script("app:dumpTree")`

### system:shell

executes script with root privileges. use with caution

Usage:

`driver.execute_script("system:shell", "ls", ["-la", "/"])`


## Qt Widgets specific execute_script methods list

### app:dumpInView

List elements in view

Usage:

`driver.execute_script("app:dumpInView", "MyItem_0x12345678")`

`"MyItem_0x12345678"` is element.id, you should find element before using this method

### app:posInView

Returns center coordinates of element item in view

Usage:

`driver.execute_script("app:posInView", "MyItem_0x12345678", "ElementName")`

`"MyItem_0x12345678"` is element.id, you should find element before using this method

### app:clickInView

Click center coordinates of element item in view

Usage:

`driver.execute_script("app:clickInView", "MyItem_0x12345678", "ElementName")`

`"MyItem_0x12345678"` is element.id, you should find element before using this method

### app:scrollInView

Scroll view to show element

Usage:

`driver.execute_script("app:scrollInView", "MyItem_0x12345678", "ElementName")`

`"MyItem_0x12345678"` is element.id, you should find element before using this method

### app:dumpInMenu

List all menu elements

Usage:

`driver.execute_script("app:dumpInMenu")`

### app:triggerInMenu

Trigger menu item

Usage:

`driver.execute_script("app:triggerInMenu", "MenuItemName")`

### app:dumpInComboBox

List elements in ComboBox

Usage:

`driver.execute_script("app:dumpInComboBox", "MyItem_0x12345678")`

`"MyItem_0x12345678"` is element.id, you should find element before using this method

### app:dumpInTabBar

List elements in TabBar

Usage:

`driver.execute_script("app:dumpInTabBar", "MyItem_0x12345678")`

`"MyItem_0x12345678"` is element.id, you should find element before using this method

### app:posInTabBar

Returns center coordinates of element item in TabBar

Usage:

`driver.execute_script("app:posInTabBar", "MyItem_0x12345678", "ElementName")`

or by index

`driver.execute_script("app:posInTabBar", "MyItem_0x12345678", 1`

`"MyItem_0x12345678"` is element.id, you should find element before using this method

### app:activateInTabBar

Activates TabBar element

Usage:

`driver.execute_script("app:activateInTabBar", "MyItem_0x12345678", "ElementName")`

or by index

`driver.execute_script("app:activateInTabBar", "MyItem_0x12345678", 1)`

`"MyItem_0x12345678"` is element.id, you should find element before using this method
