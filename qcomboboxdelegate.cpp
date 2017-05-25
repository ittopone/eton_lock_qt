#include "qcomboboxdelegate.h"
#include <QComboBox>

QComboBoxDelegate::QComboBoxDelegate(QVector<QString> itemVector)
{
    m_itemVector = itemVector;
}

QWidget * QComboBoxDelegate::createEditor(QWidget *parent,
                                           const QStyleOptionViewItem &option,
                                           const QModelIndex &index) const
{
    QComboBox *editor =  new  QComboBox(parent);
    for(int i=0;i<m_itemVector.size();i++)
    {
        editor->addItem(m_itemVector.at(i));
    }

    return  editor;
}

void  QComboBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QString text = index.model()->data(index, Qt::EditRole).toString();
    QComboBox *comboBox =  static_cast <QComboBox*>(editor);
    int  tindex = comboBox->findText(text);
    comboBox->setCurrentIndex(tindex);
}

void  QComboBoxDelegate::setModelData(QWidget *editor,
                                      QAbstractItemModel *model,
                                      const QModelIndex &index) const
{
    QComboBox *comboBox =  static_cast <QComboBox*>(editor);
    QString text = comboBox->currentText();
    model->setData(index, text, Qt::EditRole);
}

void  QComboBoxDelegate::updateEditorGeometry(QWidget *editor,
                                              const QStyleOptionViewItem &option,
                                              const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}

