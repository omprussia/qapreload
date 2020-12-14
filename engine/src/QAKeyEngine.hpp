// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#ifndef QAKEYENGINE_HPP
#define QAKEYENGINE_HPP

#include <QObject>

class QAPendingEvent;
class QKeyEvent;
class QAKeyEngine : public QObject
{
    Q_OBJECT
public:
    explicit QAKeyEngine(QObject *parent = nullptr);

    QAPendingEvent *pressEnter(int count);
    QAPendingEvent *pressBackspace(int count);
    QAPendingEvent *pressKeys(const QString &keys);

    void performChainActions(const QVariantList &actions);

signals:
    void triggered(QKeyEvent *event);

public slots:

private slots:
    void sendPress(const QChar &text, int key = 0);
    void sendRelease(const QChar &text, int key = 0);
};

#endif // QAKEYENGINE_HPP
