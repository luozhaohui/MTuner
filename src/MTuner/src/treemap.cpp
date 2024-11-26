//--------------------------------------------------------------------------//
/// Copyright 2024 Milos Tosic. All Rights Reserved.                       ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#include <MTuner_pch.h>
#include <MTuner/src/treemap.h>

TreeMapWidget::TreeMapWidget(QWidget* _parent, Qt::WindowFlags _flags)
    : QWidget(_parent, _flags)
{
    ui.setupUi(this);
    m_graphicsView = findChild<TreeMapView*>("graphicsView");
    m_context = nullptr;
    m_map = nullptr;

    QComboBox* cb = findChild<QComboBox*>("comboBoxType");
    connect(cb, SIGNAL(currentIndexChanged(int)), this, SLOT(treeMapTypeChanged(int)));

    m_scene = new QGraphicsScene(this);
    m_scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    m_scene->setBackgroundBrush(QColor(33, 80, 90, 235));

    m_graphicsView->setScene(m_scene);
    m_graphicsView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    connect(m_graphicsView,
            SIGNAL(setStackTrace(rtm::StackTrace**, int)),
            this,
            SIGNAL(setStackTrace(rtm::StackTrace**, int)));

    connect(m_graphicsView, SIGNAL(highlightTime(uint64_t)), this, SIGNAL(highlightTime(uint64_t)));
    connect(m_graphicsView, SIGNAL(highlightRange(uint64_t, uint64_t)), this, SIGNAL(highlightRange(uint64_t, uint64_t)));
}

void TreeMapWidget::changeEvent(QEvent* _event)
{
    QWidget::changeEvent(_event);
    if (_event->type() == QEvent::LanguageChange)
        ui.retranslateUi(this);
}

void TreeMapWidget::setContext(CaptureContext* _context)
{
    m_context = _context;
    m_graphicsView->setContext(_context);
    m_map = new TreeMapGraphicsItem(m_graphicsView, _context);
    m_scene->addItem(m_map);
}

void TreeMapWidget::setFilteringState(bool _state)
{
    RTM_UNUSED(_state);
    m_graphicsView->setMapType(m_graphicsView->getMapType());
}

void TreeMapWidget::treeMapTypeChanged(int _type)
{
    m_graphicsView->setMapType(_type);
}
