// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#ifndef QAENGINE_HPP
#define QAENGINE_HPP

#include <QObject>

class QAEngineSocketClient;
class QTcpSocket;
class IEnginePlatform;
class QWindow;
class QAEngine : public QObject
{
    Q_OBJECT
public:
    static QAEngine *instance();
    static bool isLoaded();
    static void objectCreated(QObject *o);
    static void objectRemoved(QObject *o);
    IEnginePlatform* getPlatform(bool silent = false);

    virtual ~QAEngine();

    static QString processName();
    static bool metaInvoke(QTcpSocket *socket, QObject *object, const QString &methodName, const QVariantList &params, bool *implemented = nullptr);

    void addItem(QObject *o);
    void removeItem(QObject *o);

public slots:
    void initialize();

private slots:
    void onFocusWindowChanged(QWindow *window);
    void processCommand(QTcpSocket *socket, const QByteArray &cmd);
    bool processAppiumCommand(QTcpSocket *socket, const QString &action, const QVariantList &params);
    void onPlatformReady();

private:
    explicit QAEngine(QObject *parent = nullptr);
    QAEngineSocketClient *m_client = nullptr;
};

#endif // QAENGINE_HPP
