#pragma once

#include <QList>
#include <QVariant>

class OrbitTreeItem {
 public:
  explicit OrbitTreeItem(const QList<QVariant>& data,
                         OrbitTreeItem* parentItem = nullptr);
  ~OrbitTreeItem();

  void appendChild(OrbitTreeItem* child);

  OrbitTreeItem* child(int row);
  int childCount() const;
  int columnCount() const;
  QVariant data(int column) const;
  int row() const;
  OrbitTreeItem* parentItem();
  bool IsVisible() const { return m_IsVisible; }
  bool MatchesFilter() const { return m_MatchesFilter; }
  void SetVisibleRecursive(bool a_Visible);
  void SetMatchRecursive(bool a_Match);
  void SetParentsVisible(bool a_Visible);
  void Filter(const std::string& a_Filter);
  void FilterRecursive(const std::string& a_Filter);
  bool Contains(const std::string& a_Filter);

 private:
  QList<OrbitTreeItem*> m_childItems;
  QList<QVariant> m_itemData;
  OrbitTreeItem* m_parentItem;
  bool m_IsVisible;
  bool m_MatchesFilter;
};
