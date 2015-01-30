#ifndef YE_TOOLBAR_H
#define YE_TOOLBAR_H

#include <QWidget>
#include <QBoxLayout>
#include <QSpacerItem>
#include <QMargins>
//==============================================================================================================================

class ToolItem;
class ToolIcon;
class ToolList;

class ToolBar : public QWidget
{
    Q_OBJECT
public:
	explicit ToolBar(Qt::Orientation orientation = Qt::Horizontal, QWidget *parent = 0);
	~ToolBar();

	void setupLayout();
	void setupTailLayout();

	void updateIconSize(int w, int h);
	void updateMinimumSize();
	void updateOrientation();
	void updateLayout();

	void addSeparator(int size = 8);
	void addItem(QWidget *item, int stretch, Qt::Alignment alignment);
	void addItem(QWidget *item, int stretch = 0);
	void insertItem(QWidget *item, int pos, int stretch = 0);
	void removeItem(QWidget *item);

	ToolList *addToolList();
	ToolIcon *addToolIcon();
	ToolIcon *addToolIcon(const QIcon &icon);
	ToolIcon *addToolIcon(const QIcon &icon, const QString &tips);
	ToolIcon *addToolIcon(const QIcon &icon, const QObject *receiver, const char *method);
	ToolIcon *addToolIcon(const QIcon &icon, const QString &tips, const QObject *receiver, const char *method);
	ToolIcon *addToolIcon(const QString &tips);
	ToolIcon *addToolIcon(const QString &tips, const QObject *receiver, const char *method);

	void addTailSeparator(int size = 8);
	void addTailItem(QWidget *item, int stretch, Qt::Alignment alignment);
	void addTailItem(QWidget *item, int stretch = 0);
	void insertTailItem(QWidget *item, int pos, int stretch = 0);
	void removeTailItem(QWidget *item);

	ToolIcon *addTailIcon();
	ToolIcon *addTailIcon(const QIcon &icon);
	ToolIcon *addTailIcon(const QIcon &icon, const QString &tips);
	ToolIcon *addTailIcon(const QIcon &icon, const QObject *receiver, const char *method);
	ToolIcon *addTailIcon(const QIcon &icon, const QString &tips, const QObject *receiver, const char *method);
	ToolIcon *addTailIcon(const QString &tips);
	ToolIcon *addTailIcon(const QString &tips, const QObject *receiver, const char *method);

	void setOrientation(Qt::Orientation orientation) { m_orientation = orientation; }
	void setIconSize(int w, int h);
	void setBasePads(int left, int top, int right, int bottom);
	void setItemPads(int left, int top, int right, int bottom);
	void setItemSpacing(int spacing);
	void setHasSpacer(bool flag) { m_hasSpacer = flag; }

	const QSize          &iconSize()     const { return m_iconSize; }
	const QMargins       &basePads()     const { return m_basePads; }
	const QMargins       &itemPads()     const { return m_itemPads; }
	QBoxLayout           *itemLayout()   const { return m_itemLayout; }
	QBoxLayout           *tailLayout()   const { return m_tailLayout; }
	bool                  isHorizontal() const { return m_orientation == Qt::Horizontal; }
	Qt::Orientation       orientation()  const { return m_orientation; }
	QBoxLayout::Direction direction()    const { return isHorizontal() ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom; }
	Qt::Alignment         alignment()    const { return isHorizontal() ? Qt::AlignLeft : Qt::AlignTop; }

protected:

private:
	void doAddSeparator(QBoxLayout *box, int size);
	void doInsertItem(QBoxLayout *box, QWidget *item, int pos, int stretch);
	void doRemoveItem(QBoxLayout *box, QWidget *item);

signals:

public slots:

private:
	Qt::Orientation m_orientation;
	QSize           m_iconSize;
	QMargins        m_basePads;
	QMargins        m_itemPads;
	int             m_itemSpacing;
	int             m_separatorSize;
	bool            m_hasSpacer;
	QSpacerItem    *m_baseSpacer;
	QBoxLayout     *m_baseLayout;
	QBoxLayout     *m_itemLayout;
	QBoxLayout     *m_tailLayout;
};

#endif // YE_TOOLBAR_H
