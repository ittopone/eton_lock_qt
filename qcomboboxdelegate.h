#ifndef QCOMBOBOXDELEGATE_H
#define QCOMBOBOXDELEGATE_H
#include <QWidget>
#include <QItemDelegate>
class QComboBoxDelegate : public QItemDelegate
{
private:
    QVector<QString> m_itemVector;

public:
    QComboBoxDelegate(QVector<QString>);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;

    void setModelData(QWidget *editor, QAbstractItemModel *model,
                       const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *editor,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const;
};

#endif // USERGROUPDELEGATE_H
