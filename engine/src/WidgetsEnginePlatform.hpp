#pragma once

#include "GenericEnginePlatform.hpp"

#include <QJsonObject>

class QWidget;
class QXmlStreamWriter;
class WidgetsEnginePlatform : public GenericEnginePlatform
{
    Q_OBJECT
public:
    explicit WidgetsEnginePlatform(QObject *parent);

    QWidget *findItemByObjectName(const QString &objectName, QWidget *parentItem = nullptr);
    QVariantList findItemsByClassName(const QString &className, QWidget *parentItem = nullptr);
    QVariantList findItemsByProperty(const QString &propertyName, const QVariant &propertyValue, QWidget *parentItem = nullptr);
    QVariantList findItemsByText(const QString &text, bool partial = true, QWidget *parentItem = nullptr);
    QVariantList findItemsByXpath(const QString &xpath, QWidget *parentItem = nullptr);
    void findByProperty(QTcpSocket *socket, const QString &propertyName, const QVariant &propertyValue, bool multiple = false, QWidget *parentItem = nullptr);
    QVariantList filterVisibleItems(const QVariantList &items);

public slots:
    virtual void initialize() override;

private slots:
    void onFocusWindowChanged(QWindow *window);

    // IEnginePlatform interface
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

    // execute_%1 methods
    void executeCommand_app_dumpTree(QTcpSocket *socket);

    // findElement_%1 methods
    void findStrategy_id(QTcpSocket *socket, const QString &selector, bool multiple = false, QWidget *parentItem = nullptr);
    void findStrategy_objectName(QTcpSocket *socket, const QString &selector, bool multiple = false, QWidget *parentItem = nullptr);
    void findStrategy_classname(QTcpSocket *socket, const QString &selector, bool multiple = false, QWidget *parentItem = nullptr);
    void findStrategy_name(QTcpSocket *socket, const QString &selector, bool multiple = false, QWidget *parentItem = nullptr);
    void findStrategy_parent(QTcpSocket *socket, const QString &selector, bool multiple = false, QWidget *parentItem = nullptr);
    void findStrategy_xpath(QTcpSocket *socket, const QString &selector, bool multiple = false, QWidget *parentItem = nullptr);

protected:
    void recursiveDumpXml(QXmlStreamWriter *writer, QWidget *rootItem, int depth = 0);
    QJsonObject recursiveDumpTree(QWidget *rootItem, int depth = 0);
    QJsonObject dumpObject(QWidget *item, int depth = 0);
    QPoint getAbsPosition(QWidget *item);
    QString getText(QWidget *item);
    void grabScreenshot(QTcpSocket *socket, QWidget *item, bool fillBackground = false);
    void findElement(QTcpSocket *socket, const QString &strategy, const QString &selector, bool multiple = false, QWidget *item = nullptr);
    void clickItem(QWidget *item);
    void setProperty(QTcpSocket *socket, const QString &property, const QString &value, const QString &elementId);

private:
    QWidget *m_rootWidget = nullptr;
};

