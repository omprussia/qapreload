#include "SailfishEnginePlatform.hpp"

#include <QDebug>
#include <QQuickItem>
#include <QQuickWindow>

#include "QAMouseEngine.hpp"
#include "QAKeyEngine.hpp"

SailfishEnginePlatform::SailfishEnginePlatform(QObject *parent)
    : QuickEnginePlatform(parent)
{
    m_mouseEngine->setMode(QAMouseEngine::TouchEventMode);
}

void SailfishEnginePlatform::onChildrenChanged()
{
    if (m_rootQuickItem->childItems().isEmpty()) {
        return;
    }

    qDebug()
        << Q_FUNC_INFO
        << "Congratilations! New childrens appeared!";

    disconnect(m_rootQuickItem, &QQuickItem::childrenChanged,
               this, &SailfishEnginePlatform::onChildrenChanged);

    emit ready();
}

void SailfishEnginePlatform::initialize()
{
    qWarning()
        << Q_FUNC_INFO
        << m_rootQuickItem;

    QuickEnginePlatform::initialize();

    if (!m_rootQuickItem) {
        qWarning()
            << Q_FUNC_INFO
            << "No root item!";
        return;
    }

    qDebug()
        << Q_FUNC_INFO
        << m_rootQuickItem->window();

    if (m_rootQuickItem->childItems().isEmpty()) { // probably declarative cache
        qDebug()
            << Q_FUNC_INFO
            << "No childrens! Waiting for population...";
        connect(m_rootQuickItem, &QQuickItem::childrenChanged,
                this, &SailfishEnginePlatform::onChildrenChanged); // let's wait for loading
    }
}
