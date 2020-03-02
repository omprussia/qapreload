#pragma once

#include "GenericEnginePlatform.hpp"

class QQuickItem;
class QQuickWindow;
class QuickEnginePlatform : public GenericEnginePlatform
{
    Q_OBJECT
public:
    explicit QuickEnginePlatform(QObject *parent);

    QQuickItem *findItemByObjectName(const QString &objectName, QQuickItem *parentItem = nullptr);
    QVariantList findItemsByClassName(const QString &getClassName, QQuickItem *parentItem = nullptr);
    QVariantList findItemsByProperty(const QString &propertyName, const QVariant &propertyValue, QQuickItem *parentItem = nullptr);
    QVariantList findItemsByText(const QString &text, bool partial = true, QQuickItem *parentItem = nullptr);
    void findByProperty(QTcpSocket *socket, const QString &propertyName, const QVariant &propertyValue, bool multiple = false, QQuickItem *parentItem = nullptr);
    QQuickItem *findParentFlickable(QQuickItem *rootItem = nullptr);
    QVariantList findNestedFlickable(QQuickItem *parentItem = nullptr);
    QVariantList filterVisibleItems(const QVariantList &items);
    QQuickItem *getApplicationWindow();

    QPointF getAbsPosition(QQuickItem *item);
    QString getText(QQuickItem *item);
    QVariant executeJS(const QString &jsCode, QQuickItem *item);

public slots:
    virtual void initialize() override;

protected:
    void clickItem(QQuickItem *item);
    void pressAndHoldItem(QQuickItem *item, int delay = 800);
    void waitForPropertyChange(QQuickItem *item, const QString &propertyName, const QVariant &value, int timeout = 10000);

    QQuickItem *getItem(const QString &elementId);

    QQuickItem *m_rootQuickItem = nullptr;
    QQuickWindow *m_rootQuickWindow = nullptr;

private:
    void findElement(QTcpSocket *socket, const QString &strategy, const QString &selector, bool multiple = false, QQuickItem *item = nullptr);
    void grabScreenshot(QTcpSocket *socket, QQuickItem *item, bool fillBackground = false);
    void setProperty(QTcpSocket *socket, const QString &property, const QString &value, const QString &elementId);

private slots:
    virtual void activateAppCommand(QTcpSocket *socket, const QString &appName) override;
    virtual void closeAppCommand(QTcpSocket *socket, const QString &appName) override;
    virtual void queryAppStateCommand(QTcpSocket *socket, const QString &appName) override;
    virtual void backgroundCommand(QTcpSocket *socket, double seconds) override;
//    virtual void getClipboardCommand(QTcpSocket *socket) override;
//    virtual void setClipboardCommand(QTcpSocket *socket, const QString &content) override;
    virtual void findElementCommand(QTcpSocket *socket, const QString &strategy, const QString &selector) override;
    virtual void findElementsCommand(QTcpSocket *socket, const QString &strategy, const QString &selector) override;
    virtual void findElementFromElementCommand(QTcpSocket *socket, const QString &strategy, const QString &selector, const QString &elementId) override;
    virtual void findElementsFromElementCommand(QTcpSocket *socket, const QString &strategy, const QString &selector, const QString &elementId) override;
    virtual void getLocationCommand(QTcpSocket *socket, const QString &elementId) override;
    virtual void getLocationInViewCommand(QTcpSocket *socket, const QString &elementId) override;
    virtual void getAttributeCommand(QTcpSocket *socket, const QString &attribute, const QString &elementId) override;
    virtual void getPropertyCommand(QTcpSocket *socket, const QString &attribute, const QString &elementId) override;
    virtual void getTextCommand(QTcpSocket *socket, const QString &elementId) override;
    virtual void getElementScreenshotCommand(QTcpSocket *socket, const QString &elementId) override;
    virtual void getScreenshotCommand(QTcpSocket *socket) override;
    virtual void elementEnabledCommand(QTcpSocket *socket, const QString &elementId) override;
    virtual void elementDisplayedCommand(QTcpSocket *socket, const QString &elementId) override;
    virtual void elementSelectedCommand(QTcpSocket *socket, const QString &elementId) override;
    virtual void getSizeCommand(QTcpSocket *socket, const QString &elementId) override;
    virtual void setValueImmediateCommand(QTcpSocket *socket, const QVariantList &value, const QString &elementId) override;
    virtual void replaceValueCommand(QTcpSocket *socket, const QVariantList &value, const QString &elementId) override;
    virtual void setValueCommand(QTcpSocket *socket, const QVariantList &value, const QString &elementId) override;
    virtual void clickCommand(QTcpSocket *socket, const QString &elementId) override;
    virtual void clearCommand(QTcpSocket *socket, const QString &elementId) override;
    virtual void submitCommand(QTcpSocket *socket, const QString &elementId) override;
    virtual void getPageSourceCommand(QTcpSocket *socket) override;

    // GenericEnginePlatform interface
    virtual void onKeyEvent(QKeyEvent *event) override;

    // execute_%1 methods
    void executeCommand_app_waitForPropertyChange(QTcpSocket *socket, const QString &elementId, const QString &propertyName, const QVariant &value, double timeout = 3000);
    void executeCommand_touch_pressAndHold(QTcpSocket *socket, double posx, double posy);
    void executeCommand_touch_mouseSwipe(QTcpSocket *socket, double posx, double posy, double stopx, double stopy);
    void executeCommand_touch_mouseDrag(QTcpSocket *socket, double posx, double posy, double stopx, double stopy);
    void executeCommand_app_method(QTcpSocket *socket, const QString &elementId, const QString &method, const QVariantList &params);
    void executeCommand_app_js(QTcpSocket *socket, const QString &elementId, const QString &jsCode);
    void executeCommand_app_setAttribute(QTcpSocket *socket, const QString &elementId, const QString &attribute, const QString &value);

    // own stuff
    void onPropertyChanged();
};

