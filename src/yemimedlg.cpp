#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QFormLayout>
#include <QCompleter>
#include <QStringListModel>
#include <QTimer>
#include <QCloseEvent>
#include <QDebug>

#include "yemimedlg.h"
#include "yemimesettings.h"
#include "yefileutils.h"
#include "yesplitter.h"
//==============================================================================================================================

#define DEF_W 800
#define MIN_W 320
#define MIN_H 480

namespace MimeDlgMode {
	enum { None, SeleApp, EditMimes };
}

MimeDlg::MimeDlg(YeApplication *app, QWidget *parent)
	: QDialog(parent)
	, m_app(app)
	, m_mode(MimeDlgMode::None)
	, m_appWidth(MIN_W)
	, m_width(DEF_W)
	, m_height(MIN_H)
	, m_loaded(false)
	, m_busy(false)
	, m_mimeSettings(NULL)
{
	m_appWidget = createAppTree();
	m_mimeSettings = new MimeSettings(m_app);

	Splitter *splitter = new Splitter;
	splitter->setDirection(SplitterDirection::Left, 220);
	splitter->setClient(m_appWidget, m_mimeSettings);

	m_buttons = new QDialogButtonBox;
	m_buttons->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(m_buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(m_buttons, SIGNAL(rejected()), this, SLOT(reject()));

	QVBoxLayout *box = new QVBoxLayout(this);
	box->setContentsMargins(6, 6, 6, 6);
	box->setSpacing(6);
	box->addWidget(splitter);
	box->addWidget(m_buttons);

	setMinimumSize(MIN_W, MIN_H);

	connect(m_appTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
			this, SLOT(onAppItemDoubleClicked(QTreeWidgetItem*,int)));
	connect(&m_thread, SIGNAL(loadFinished()), this, SLOT(onLoadFinished()));
}

void MimeDlg::closeEvent(QCloseEvent *event)
{
	if (m_busy) {
		event->setAccepted(false);
		return;
	}

	if (m_mode == MimeDlgMode::EditMimes) m_width = width();
	m_height = height();
	m_appWidth = m_appWidget->width();

	if (m_width <= m_appWidth)
		m_width = m_appWidth + 160;

	event->accept();
}

void MimeDlg::accept()
{
	if (m_mode == MimeDlgMode::EditMimes) {
		if (m_mimeSettings->save()) this->done(1);
	} else {
		QDialog::accept();
	}
}
//==============================================================================================================================

bool MimeDlg::showDialog(QString &resultApp, const QString &mimeType)
{
	setWindowTitle(tr("Select application"));
	m_mode = MimeDlgMode::SeleApp;

	m_titleLabel->setText(tr("For mime-type: %1").arg(mimeType));
	m_edtLabel->setText(tr("Launcher: "));
	m_edtCommand->setVisible(true);
	m_mimeSettings->setVisible(false);
	resize(m_appWidth, m_height);

	bool ok = exec() == QDialog::Accepted;

	if (ok) {
		QString app = m_edtCommand->text();
		ok = !app.isEmpty();
		if (ok) resultApp = app + ".desktop";
	}

	return ok;
}

void MimeDlg::showDialog()
{
	setWindowTitle(tr("Edit mime-open items"));
	m_mode = MimeDlgMode::EditMimes;

	m_titleLabel->setText(tr("Available applications"));
	m_edtLabel->setText(tr("Double click to add application."));
	m_edtCommand->setVisible(false);
	m_mimeSettings->setVisible(true);
	resize(m_width, m_height);

	bool ok = exec() == QDialog::Accepted;

	if (ok) {
		//
	}
}

void MimeDlg::showEvent(QShowEvent *event)
{
	Q_UNUSED(event);

	QTimer::singleShot(200, this, SLOT(loadItems()));
}
//==============================================================================================================================

QWidget *MimeDlg::createAppTree()
{
	m_titleLabel = new QLabel;

	// Creates app list view
	m_appTree = new QTreeWidget;
	m_appTree->setIconSize(QSize(16, 16));
	m_appTree->setAlternatingRowColors(true);
	m_appTree->headerItem()->setText(0, tr("Application"));

	// Command bar
	m_edtLabel = new QLabel;
	m_edtCommand = new QLineEdit;
	m_edtCommand->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	QHBoxLayout *hbox = new QHBoxLayout;
	hbox->setContentsMargins(0, 0, 0, 0);
	hbox->setSpacing(4);
	hbox->addWidget(m_edtLabel);
	hbox->addWidget(m_edtCommand);

	QWidget *widget = new QWidget;
	QVBoxLayout *box = new QVBoxLayout(widget);
	box->setContentsMargins(0, 0, 0, 0);
	box->setSpacing(4);
	box->addWidget(m_titleLabel);
	box->addWidget(m_appTree);
	box->addLayout(hbox);

	// Synonyms for cathegory names
	m_catNames.clear();
	m_catNames.insert("Development", QStringList() << "Programming");
	m_catNames.insert("Games", QStringList() << "Game");
	m_catNames.insert("Graphics", QStringList());
	m_catNames.insert("Internet", QStringList() << "Network" << "WebBrowser");
	m_catNames.insert("Multimedia", QStringList() << "AudioVideo" << "Video");
	m_catNames.insert("Office", QStringList());
	m_catNames.insert("Other", QStringList());
	m_catNames.insert("Settings", QStringList() << "System");
	m_catNames.insert("Utilities", QStringList() << "Utility");

	// Load default icon
	m_defaultIcon = QIcon::fromTheme("application-x-executable");

	// Create default application cathegories
	m_categories.clear();
	createCategories();

	// Signals
	connect(m_appTree, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
			SLOT(updateCommand(QTreeWidgetItem *, QTreeWidgetItem *)));
	connect(m_appTree, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(initCategory(QTreeWidgetItem*)));

	return widget;
}

void MimeDlg::createCategories()
{
	foreach (QString name, m_catNames.keys()) {

		QIcon icon = QIcon::fromTheme("applications-" + name.toLower());		// Find icon

		if (icon.isNull()) {		// If icon not found, check synonyms
			foreach (QString synonym, m_catNames.value(name)) {
				icon = QIcon::fromTheme("applications-" + synonym.toLower());
				break;
			}
		}

		if (icon.isNull()) {		// If icon still not found, retrieve default icon
			icon = m_defaultIcon;
		}

		QTreeWidgetItem *category = new QTreeWidgetItem(m_appTree);		// Create category
		category->setText(0, name);
		category->setIcon(0, icon);
		category->setFlags(Qt::ItemIsEnabled);

		m_categories.insert(name, category);
	}
}
//==============================================================================================================================

QTreeWidgetItem *MimeDlg::findCategory(const DesktopFile &app)
{
	QTreeWidgetItem *category = m_categories.value("Other");	// Default categoty is 'Other'

	foreach (QString name, m_catNames.keys()) {					// Try to find more suitable category

		if (app.getCategories().contains(name)) {				// Try cathegory name
			category = m_categories.value(name);
			break;
		}

		bool found = false;
		foreach (QString synonym, m_catNames.value(name)) {		// Try synonyms
			if (app.getCategories().contains(synonym)) {
				found = true;
				break;
			}
		}
		if (found) {
			category = m_categories.value(name);
			break;
		}
	}

	return category;
}
//==============================================================================================================================

struct CatApps {
	QList<DesktopFile *> items;
};

static CatApps *getCatInfo(QTreeWidgetItem *category)
{
	QVariant data = category->data(0, Qt::UserRole);
	if (data.isNull()) return NULL;

	void *p = data.value<void *>();
	return reinterpret_cast<CatApps *>(p);
}

static void setCatInfo(QTreeWidgetItem *category, DesktopFile *app)
{
	CatApps *apps;
	if (category->childCount() < 1) {
		apps = new CatApps;
		QVariant data = QVariant::fromValue<void *>(apps);
		category->setData(0, Qt::UserRole, data);
		new QTreeWidgetItem(category);				// create first child for showing "+"
	} else {
		apps = getCatInfo(category);
	}
	apps->items.append(app);
}
//==============================================================================================================================

void MimeDlg::initCategory(QTreeWidgetItem *category)
{
	CatApps *apps = getCatInfo(category);
	if (apps == NULL) return;

	bool isNew = false;
	foreach (DesktopFile *app, apps->items) {
		QTreeWidgetItem *item;
		if (isNew) {
			item = new QTreeWidgetItem(category);	// Create item from current mime
		} else {
			isNew = true;
			item = category->child(0);				// first child
		}
		item->setIcon(0, FileUtils::searchAppIcon(*app, m_defaultIcon));
		item->setText(0, app->getName());
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		m_applications.insert(app->getPureFileName(), item);			// Register application
		delete app;
	}

	delete apps;
	category->setData(0, Qt::UserRole, QVariant());						// clear data
}

/*
 * This function run inside thread.
 */
void MimeDlg::loadCategories()
{
	QList<DesktopFile *> apps;
	FileUtils::getApplications(apps);					// Load applications and create category tree list

	foreach (DesktopFile *app, apps) {
		if (app->getName().compare("") == 0) {			// Check for name
			continue;
		}
		QTreeWidgetItem *category = findCategory(*app);	// Find category
		if (category != NULL) {
			setCatInfo(category, app);
		}
	}

//	QStringListModel *model = new QStringListModel;		// Create completer and its model for editation of command
//	model->setStringList(m_applications.keys());
//	QCompleter *completer = new QCompleter;
//	completer->setModel(model);
//	m_edtCommand->setCompleter(completer);

	m_loaded = true;
}

void MimeDlg::threadFunc(void *arg)
{
	MimeDlg *p = (MimeDlg *) arg;
	p->loadCategories();
}
//==============================================================================================================================

void MimeDlg::onLoadFinished()
{
	if (!m_mimeSettings->isLoaded() && m_mode == MimeDlgMode::EditMimes) {
		QTimer::singleShot(400, this, SLOT(loadItems()));
	}
}

void MimeDlg::loadItems()
{
	if (!m_loaded) {
		m_thread.startLoad(MimeDlg::threadFunc, (void *) this);
		return;
	}

	if (!m_mimeSettings->isLoaded() && m_mode == MimeDlgMode::EditMimes) {
	//	m_thread.startLoad(MimeSettings::threadFunc, (void *) m_mimeSettings);
		m_appTree->setEnabled(false);
		m_buttons->setEnabled(false);
		m_busy = true;
		m_mimeSettings->loadMimes();
		m_appTree->setEnabled(true);
		m_buttons->setEnabled(true);
		m_busy = false;
		return;
	}
}

//==============================================================================================================================

void MimeDlg::updateCommand(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
	Q_UNUSED(previous);

	m_edtCommand->setText(m_applications.key(current));
}

void MimeDlg::onAppItemDoubleClicked(QTreeWidgetItem *item, int col)
{
	Q_UNUSED(col);

	if (m_mode == MimeDlgMode::EditMimes) {
		m_mimeSettings->addDesktopItem(m_applications.key(item));
	}
}

//==============================================================================================================================
// class MimeThread
//==============================================================================================================================

MimeThread::MimeThread(QObject *parent)
	: QThread(parent)
{
}

void MimeThread::run()
{
	m_func(m_arg);
	emit loadFinished();
}

void MimeThread::startLoad(MimeThreadFunc func, void *arg)
{
	m_func = func;
	m_arg  = arg;
	start();
}
//==============================================================================================================================