#include "orbittreeitem.h"

#include <QStringList>

#include "OrbitCore/Utils.h"

OrbitTreeItem::OrbitTreeItem(const QList<QVariant>& data,
                             OrbitTreeItem* parent) {
  m_parentItem = parent;
  m_itemData = data;
  m_IsVisible = true;
  m_MatchesFilter = false;
}

OrbitTreeItem::~OrbitTreeItem() { qDeleteAll(m_childItems); }

void OrbitTreeItem::appendChild(OrbitTreeItem* item) {
  m_childItems.append(item);
}

OrbitTreeItem* OrbitTreeItem::child(int row) { return m_childItems.value(row); }

int OrbitTreeItem::childCount() const { return m_childItems.count(); }

int OrbitTreeItem::columnCount() const { return m_itemData.count(); }

QVariant OrbitTreeItem::data(int column) const {
  return m_itemData.value(column);
}

OrbitTreeItem* OrbitTreeItem::parentItem() { return m_parentItem; }

void OrbitTreeItem::SetVisibleRecursive(bool a_Visible) {
  m_IsVisible = a_Visible;
  for (OrbitTreeItem* item : m_childItems) {
    item->SetVisibleRecursive(a_Visible);
  }
}

void OrbitTreeItem::SetMatchRecursive(bool a_Match) {
  m_MatchesFilter = a_Match;
  for (OrbitTreeItem* item : m_childItems) {
    item->SetMatchRecursive(a_Match);
  }
}

void OrbitTreeItem::SetParentsVisible(bool a_Visible) {
  if (m_parentItem) {
    m_parentItem->m_IsVisible = a_Visible;
    m_parentItem->SetParentsVisible(a_Visible);
  }
}

void OrbitTreeItem::Filter(const std::wstring& a_Filter) {
  SetVisibleRecursive(false);
  SetMatchRecursive(false);
  FilterRecursive(a_Filter);
}

void OrbitTreeItem::FilterRecursive(const std::wstring& a_Filter) {
  if (a_Filter != L"" && this->Contains(a_Filter)) {
    m_IsVisible = true;
    m_MatchesFilter = true;
    SetParentsVisible(true);
  }

  for (OrbitTreeItem* item : m_childItems) {
    item->FilterRecursive(a_Filter);
  }
}

bool OrbitTreeItem::Contains(const std::wstring& a_Filter) {
  for (QVariant& variant : m_itemData) {
    std::wstring str = variant.toString().toStdWString();
    if (::Contains(str, a_Filter)) {
      return true;
    }
  }

  return false;
}

int OrbitTreeItem::row() const {
  if (m_parentItem) {
    return m_parentItem->m_childItems.indexOf(const_cast<OrbitTreeItem*>(this));
  }

  return 0;
}