#pragma once

#include <QObject>

class QAScreenRecorder : public QObject
{
    Q_OBJECT
public:
    explicit QAScreenRecorder(QObject *parent = nullptr);
    int state() const;
    QString lastFilename() const;
    bool setScale(double scale);
    bool start();
    bool stop();

signals:
    void stateChanged(int state);
    void recordingFinished(const QString &filename);

private slots:
    void onStateChanged(int state);
    void onRecordingFinished(const QString &filename);

private:
    int m_state = 0;
    QString m_lastFilename;
};
