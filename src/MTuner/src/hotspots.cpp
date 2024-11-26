//--------------------------------------------------------------------------//
/// Copyright 2024 Milos Tosic. All Rights Reserved.                       ///
/// License: http://www.opensource.org/licenses/BSD-2-Clause               ///
//--------------------------------------------------------------------------//

#include <MTuner_pch.h>
#include <MTuner/src/hotspots.h>

static const uint32_t s_numRows = 32;

HotspotsWidget::HotspotsWidget(QWidget* _parent, Qt::WindowFlags _flags)
    : QWidget(_parent, _flags)
{
    ui.setupUi(this);

    m_usageMapping = NULL;
    m_peakUsageMapping = NULL;
    m_leaksMapping = NULL;

    m_usageTable = findChild<QTableWidget*>("tableUsage");
    m_peakUsageTable = findChild<QTableWidget*>("tablePeak");
    m_peakCountTable = findChild<QTableWidget*>("tablePeakCount");
    m_leaksTable = findChild<QTableWidget*>("tableLeaks");

    m_tableKeyWatch = new TableKeyWatcher();
    m_tableKeyWatch->installOn(m_usageTable);
    m_tableKeyWatch->installOn(m_peakUsageTable);
    m_tableKeyWatch->installOn(m_peakCountTable);
    m_tableKeyWatch->installOn(m_leaksTable);

    connect(m_tableKeyWatch, SIGNAL(updateStackTrace(QTableWidget*)), this, SLOT(updateStackTrace(QTableWidget*)));

    m_usageTable->horizontalHeader()->setHighlightSections(false);
    m_peakUsageTable->horizontalHeader()->setHighlightSections(false);
    m_peakCountTable->horizontalHeader()->setHighlightSections(false);
    m_leaksTable->horizontalHeader()->setHighlightSections(false);

    m_usageTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_peakUsageTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_peakCountTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_leaksTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    connect(m_usageTable, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(rowSelected(QTableWidgetItem*)));
    connect(m_peakUsageTable, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(rowSelected(QTableWidgetItem*)));
    connect(m_peakCountTable, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(rowSelected(QTableWidgetItem*)));
    connect(m_leaksTable, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(rowSelected(QTableWidgetItem*)));

    connect(m_usageTable, SIGNAL(itemActivated(QTableWidgetItem*)), this, SLOT(rowSelected(QTableWidgetItem*)));
    connect(m_peakUsageTable, SIGNAL(itemActivated(QTableWidgetItem*)), this, SLOT(rowSelected(QTableWidgetItem*)));
    connect(m_peakCountTable, SIGNAL(itemActivated(QTableWidgetItem*)), this, SLOT(rowSelected(QTableWidgetItem*)));
    connect(m_leaksTable, SIGNAL(itemActivated(QTableWidgetItem*)), this, SLOT(rowSelected(QTableWidgetItem*)));
}

void HotspotsWidget::changeEvent(QEvent* _event)
{
    QWidget::changeEvent(_event);
    if (_event->type() == QEvent::LanguageChange)
        ui.retranslateUi(this);
}

static QString s_typeName[rmem::LogMarkers::OpCount] = {"Alloc", "Alloc aligned", "Calloc", "Free", "Realloc", "Realloc aligned"};

static rtm::MemoryOperationGroup* getGroupFromMapping(GroupMapping* _group, int _index)
{
    uint32_t idx = _group->m_sortedIdx[_group->m_sortedIdx.size() - _index - 1];
    rtm::MemoryOperationGroup* group = _group->m_allGroups->operator[](idx);
    return group;
}

void HotspotsWidget::usageSortingDone(GroupMapping* _group)
{
    m_usageMapping = _group;

    int rows = m_usageTable->rowCount();
    for (int i = rows - 1; i >= 0; i--)
        m_usageTable->removeRow(i);

    QLocale locale;

    for (uint32_t i = 0; i < s_numRows; ++i)
    {
        if (i >= _group->m_sortedIdx.size())
            break;

        rtm::MemoryOperationGroup* group = getGroupFromMapping(_group, i);

        if (group->m_liveSize == 0)
            break;

        m_usageTable->insertRow(i);

        m_usageTable->setItem(i, 0, new QTableWidgetItem(s_typeName[group->m_operations[0]->m_operationType]));

        QString size;
        if (group->m_maxSize != group->m_minSize)
            size = locale.toString(group->m_minSize) + '-' + locale.toString(group->m_maxSize);
        else
            size = locale.toString(group->m_minSize);

        m_usageTable->setItem(i, 1, new QTableWidgetItem(size));
        m_usageTable->setItem(i,
                              2,
                              new QTableWidgetItem((group->m_operations[0]->m_alignment == 255)
                                                       ? QString("Default")
                                                       : QString::number(1 << group->m_operations[0]->m_alignment)));
        m_usageTable->setItem(i, 3, new QTableWidgetItem(locale.toString(group->m_liveCount)));
        m_usageTable->setItem(i, 4, new QTableWidgetItem(locale.toString((qlonglong)group->m_liveSize)));

        for (int j = 0; j < 5; ++j)
            m_usageTable->item(i, j)->setTextAlignment(Qt::AlignRight);
    }
}

void HotspotsWidget::peakUsageSortingDone(GroupMapping* _group)
{
    m_peakUsageMapping = _group;

    int rows = m_peakUsageTable->rowCount();
    for (int i = rows - 1; i >= 0; i--)
        m_peakUsageTable->removeRow(i);

    QLocale locale;

    for (uint32_t i = 0; i < s_numRows; ++i)
    {
        if (i >= _group->m_sortedIdx.size())
            break;

        rtm::MemoryOperationGroup* group = getGroupFromMapping(_group, i);

        if (group->m_peakSize == 0)
            break;

        m_peakUsageTable->insertRow(i);
        m_peakUsageTable->setItem(i, 0, new QTableWidgetItem(s_typeName[group->m_operations[0]->m_operationType]));

        QString size;
        if (group->m_maxSize != group->m_minSize)
            size = locale.toString(group->m_minSize) + '-' + locale.toString(group->m_maxSize);
        else
            size = locale.toString(group->m_minSize);

        m_peakUsageTable->setItem(i, 1, new QTableWidgetItem(size));
        m_peakUsageTable->setItem(i,
                                  2,
                                  new QTableWidgetItem(
                                      (group->m_operations[0]->m_alignment == 255)
                                          ? QString("Default")
                                          : QString::number(1 << group->m_operations[0]->m_alignment)));
        m_peakUsageTable->setItem(i, 3, new QTableWidgetItem(locale.toString(group->m_liveCountPeak)));
        m_peakUsageTable->setItem(i, 4, new QTableWidgetItem(locale.toString((qlonglong)group->m_peakSize)));

        for (int j = 0; j < 5; ++j)
            m_peakUsageTable->item(i, j)->setTextAlignment(Qt::AlignRight);
    }
}

void HotspotsWidget::peakCountSortingDone(GroupMapping* _group)
{
    m_peakCountUsageMapping = _group;

    int rows = m_peakCountTable->rowCount();
    for (int i = rows - 1; i >= 0; i--)
        m_peakCountTable->removeRow(i);

    QLocale locale;
    for (uint32_t i = 0; i < s_numRows; ++i)
    {
        if (i >= _group->m_sortedIdx.size())
            break;

        rtm::MemoryOperationGroup* group = getGroupFromMapping(_group, i);

        if (group->m_liveCountPeak == 0)
            break;

        m_peakCountTable->insertRow(i);
        m_peakCountTable->setItem(i, 0, new QTableWidgetItem(s_typeName[group->m_operations[0]->m_operationType]));

        QString size;
        if (group->m_maxSize != group->m_minSize)
            size = locale.toString(group->m_minSize) + '-' + locale.toString(group->m_maxSize);
        else
            size = locale.toString(group->m_minSize);
        m_peakCountTable->setItem(i, 1, new QTableWidgetItem(size));
        m_peakCountTable->setItem(i,
                                  2,
                                  new QTableWidgetItem(
                                      (group->m_operations[0]->m_alignment == 255)
                                          ? QString("Default")
                                          : QString::number(1 << group->m_operations[0]->m_alignment)));
        m_peakCountTable->setItem(i, 3, new QTableWidgetItem(locale.toString(group->m_liveCountPeak)));
        m_peakCountTable->setItem(i, 4, new QTableWidgetItem(locale.toString((qlonglong)group->m_peakSize)));

        for (int j = 0; j < 5; ++j)
            m_peakCountTable->item(i, j)->setTextAlignment(Qt::AlignRight);
    }
}

void HotspotsWidget::leaksSortingDone(GroupMapping* _group)
{
    m_leaksMapping = _group;

    int rows = m_leaksTable->rowCount();
    for (int i = rows - 1; i >= 0; i--)
        m_leaksTable->removeRow(i);

    QLocale locale;
    for (uint32_t i = 0; i < s_numRows; ++i)
    {
        if (i >= _group->m_sortedIdx.size())
            break;

        rtm::MemoryOperationGroup* group = getGroupFromMapping(_group, i);

        if (group->m_liveCount * group->m_operations[0]->m_allocSize == 0)
            break;

        m_leaksTable->insertRow(i);
        m_leaksTable->setItem(i, 0, new QTableWidgetItem(s_typeName[group->m_operations[0]->m_operationType]));

        QString size;
        if (group->m_maxSize != group->m_minSize)
            size = locale.toString(group->m_minSize) + '-' + locale.toString(group->m_maxSize);
        else
            size = locale.toString(group->m_minSize);

        m_leaksTable->setItem(i, 1, new QTableWidgetItem(size));
        m_leaksTable->setItem(i,
                              2,
                              new QTableWidgetItem((group->m_operations[0]->m_alignment == 255)
                                                       ? QString("Default")
                                                       : QString::number(1 << group->m_operations[0]->m_alignment)));
        m_leaksTable->setItem(i, 3, new QTableWidgetItem(locale.toString(group->m_liveCount)));
        m_leaksTable->setItem(i, 4, new QTableWidgetItem(locale.toString((qlonglong)group->m_liveSize)));

        for (int j = 0; j < 5; ++j)
            m_leaksTable->item(i, j)->setTextAlignment(Qt::AlignRight);
    }
}

void HotspotsWidget::rowSelected(QTableWidgetItem* _item)
{
    updateStackTrace(_item->tableWidget());
}

void HotspotsWidget::updateStackTrace(QTableWidget* _table)
{
    QTableWidget* w = _table;
    int row = w->currentItem()->row();
    rtm::MemoryOperationGroup* group = 0;

    if (w == m_usageTable)
        group = getGroupFromMapping(m_usageMapping, row);

    if (w == m_peakUsageTable)
        group = getGroupFromMapping(m_peakUsageMapping, row);

    if (w == m_peakCountTable)
        group = getGroupFromMapping(m_peakCountUsageMapping, row);

    if (w == m_leaksTable)
        group = getGroupFromMapping(m_leaksMapping, row);

    if (w != m_usageTable)
        m_usageTable->clearSelection();
    if (w != m_peakUsageTable)
        m_peakUsageTable->clearSelection();
    if (w != m_peakCountTable)
        m_peakCountTable->clearSelection();
    if (w != m_leaksTable)
        m_leaksTable->clearSelection();

    if (group)
    {
        emit setStackTrace(&(group->m_operations[0]->m_stackTrace), 1);

        size_t len = group->m_operations.size();
        uint64_t mn = group->m_operations[0]->m_operationTime;
        uint64_t mx = group->m_operations[len - 1]->m_operationTime;
        emit highlightRange(mn, mx);
    }
}
