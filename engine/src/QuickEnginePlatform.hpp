#pragma once

#include "GenericEnginePlatform.hpp"

#include <QJsonObject>

class QQmlEngine;
class QQuickItem;
class QQuickWindow;
class QXmlStreamWriter;
class QuickEnginePlatform : public GenericEnginePlatform
{
    Q_OBJECT
public:
    explicit QuickEnginePlatform(QObject *parent);

    QQuickItem *findItemByObjectName(const QString &objectName, QQuickItem *parentItem = nullptr);
    QVariantList findItemsByClassName(const QString &className, QQuickItem *parentItem = nullptr);
    QVariantList findItemsByProperty(const QString &propertyName, const QVariant &propertyValue, QQuickItem *parentItem = nullptr);
    QVariantList findItemsByText(const QString &text, bool partial = true, QQuickItem *parentItem = nullptr);
    QVariantList findItemsByXpath(const QString &xpath, QQuickItem *parentItem = nullptr);
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
    void clearFocus();
    void clearComponentCache();
    void findElement(QTcpSocket *socket, const QString &strategy, const QString &selector, bool multiple = false, QQuickItem *item = nullptr);
    void grabScreenshot(QTcpSocket *socket, QQuickItem *item, bool fillBackground = false);
    void setProperty(QTcpSocket *socket, const QString &property, const QString &value, const QString &elementId);
    void recursiveDumpXml(QXmlStreamWriter *writer, QQuickItem *rootItem, int depth = 0);
    QJsonObject recursiveDumpTree(QQuickItem *rootItem, int depth = 0);
    QJsonObject dumpObject(QQuickItem *item, int depth = 0);

    QQmlEngine *getEngine(QQuickItem *item = nullptr);

    QQuickItem *m_rootQuickItem = nullptr;
    QQuickWindow *m_rootQuickWindow = nullptr;

    QHash<QString, QStringList> m_blacklistedProperties;

private slots:
    // IEnginePlatform interface
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

    // synthesized input events
    virtual void onKeyEvent(QKeyEvent *event) override;

    // findElement_%1 methods
    void findStrategy_id(QTcpSocket *socket, const QString &selector, bool multiple = false, QQuickItem *parentItem = nullptr);
    void findStrategy_objectName(QTcpSocket *socket, const QString &selector, bool multiple = false, QQuickItem *parentItem = nullptr);
    void findStrategy_classname(QTcpSocket *socket, const QString &selector, bool multiple = false, QQuickItem *parentItem = nullptr);
    void findStrategy_name(QTcpSocket *socket, const QString &selector, bool multiple = false, QQuickItem *parentItem = nullptr);
    void findStrategy_parent(QTcpSocket *socket, const QString &selector, bool multiple = false, QQuickItem *parentItem = nullptr);
    void findStrategy_xpath(QTcpSocket *socket, const QString &selector, bool multiple = false, QQuickItem *parentItem = nullptr);

    // execute_%1 methods
    void executeCommand_touch_pressAndHold(QTcpSocket *socket, double posx, double posy);
    void executeCommand_touch_mouseSwipe(QTcpSocket *socket, double posx, double posy, double stopx, double stopy);
    void executeCommand_touch_mouseDrag(QTcpSocket *socket, double posx, double posy, double stopx, double stopy);
    void executeCommand_app_method(QTcpSocket *socket, const QString &elementId, const QString &method, const QVariantList &params);
    void executeCommand_app_js(QTcpSocket *socket, const QString &elementId, const QString &jsCode);
    void executeCommand_app_setAttribute(QTcpSocket *socket, const QString &elementId, const QString &attribute, const QString &value);
    void executeCommand_app_dumpTree(QTcpSocket *socket);
};

