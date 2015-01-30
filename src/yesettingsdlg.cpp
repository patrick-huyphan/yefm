#include <QCloseEvent>
#include <QDirIterator>
#include <QDebug>

#include "yesettingsdlg.h"
#include "ui_yesettingsdlg.h"
#include "yeapplication.h"
#include "yemainwindow.h"
#include "yeapp.h"
#include "yeappdata.h"
#include "yeappresources.h"
//==============================================================================================================================

SettingsDlg::SettingsDlg(YeApplication *app, QWidget *parent)
	: QWidget(parent, Qt::Dialog | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint)
	, ui(new Ui::SettingsDlg)
	, m_app(app)
	, m_ready(false)
{
	ui->setupUi(this);
	ui->navList->setCurrentRow(0);

	connect(ui->navList, SIGNAL(currentRowChanged(int)), this, SLOT(onPageChanged(int)));
	connect(ui->iconThemeList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
			this, SLOT(onCurrentItemChanged(QListWidgetItem*,QListWidgetItem*)));
	connect(ui->rbIcon16, SIGNAL(toggled(bool)), this, SLOT(onIconSizeChanged(bool)));
	connect(ui->rbIcon22, SIGNAL(toggled(bool)), this, SLOT(onIconSizeChanged(bool)));
	connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	updateIconTheme();
	connect(app, SIGNAL(iconThemeChanged()), this, SLOT(onIconThemeChanged()));
}

SettingsDlg::~SettingsDlg()
{
	delete ui;
}
//==============================================================================================================================

void SettingsDlg::showDialog()
{
	AppData &d = R::data();

	m_savedIconTheme = d.iconTheme;
	m_savedIconSize  = d.iconSize;
	//--------------------------------------------------------------------------------------------------------------------------

	// window
	ui->chk2paneWinHeight->setChecked(d.expand2paneWinHeight);
	if (d.rightSide) ui->rbSideRight->setChecked(true); else ui->rbSideLeft->setChecked(true);
	//--------------------------------------------------------------------------------------------------------------------------

	// click & enter
	switch (d.clickEnter) {
		case ClickEnter::ClickIcon  : ui->rbEnterByIconClk->setChecked(true); break;
		case ClickEnter::SingleClick: ui->rbEnterByMonoClk->setChecked(true); break;
		case ClickEnter::DoubleClick:
		default                     : ui->rbEnterByDblClk ->setChecked(true);
	}
	ui->edHoverTime->setText(QString::number(d.hoverTime));
	ui->chkKeyStopHover->setChecked(d.keyStopHover);
	//--------------------------------------------------------------------------------------------------------------------------

	// icon theme
	initIconThemes();
	if (d.iconSize == 16) ui->rbIcon16->setChecked(true); else ui->rbIcon22->setChecked(true);
	//--------------------------------------------------------------------------------------------------------------------------

	// program files
	ui->edTerminal->setText(d.terminal);
	//--------------------------------------------------------------------------------------------------------------------------

	// time format
	ui->edStatDateFormat->setText(d.statDateFormat);
	ui->edStatTimeFormat->setText(d.statTimeFormat);

	ui->edFileDateFormat->setText(d.fileDateFormat);
	ui->edFileTimeFormat->setText(d.fileTimeFormat);

	ui->edPropDateFormat->setText(d.propDateFormat);
	ui->edPropTimeFormat->setText(d.propTimeFormat);
	//--------------------------------------------------------------------------------------------------------------------------

	this->show();
	m_ready = true;
}

void SettingsDlg::accept()
{
	AppData &d = R::data();

	// window
	bool expand2paneWinHeight = ui->chk2paneWinHeight->isChecked();
	bool rightSide = ui->rbSideRight->isChecked();
	//--------------------------------------------------------------------------------------------------------------------------

	// click & enter
	int clickEnter;
	int hoverTime = ui->edHoverTime->text().toInt();
	bool keyStopHover = ui->chkKeyStopHover->isChecked();
	if      (ui->rbEnterByIconClk->isChecked()) clickEnter = ClickEnter::ClickIcon;
	else if (ui->rbEnterByMonoClk->isChecked()) clickEnter = ClickEnter::SingleClick;
	else if (ui->rbEnterByDblClk->isChecked())  clickEnter = ClickEnter::DoubleClick;
	else                                        clickEnter = ClickEnter::DoubleClick;

	if (hoverTime < 200) hoverTime = 200;
	else if (hoverTime > 2000) hoverTime = 2000;
	ui->edHoverTime->setText(QString::number(hoverTime));
	//--------------------------------------------------------------------------------------------------------------------------

	// icon theme
	//	QString iconTheme = getSelectedIconTheme();
	//	int     iconSize  = ui->rbIcon16->isChecked() ? 16 : 22;
	//--------------------------------------------------------------------------------------------------------------------------

	// program files
	QString terminal = ui->edTerminal->text().trimmed();
	//--------------------------------------------------------------------------------------------------------------------------

	// date-time format
	QString statDateFormat = ui->edStatDateFormat->text().trimmed();
	QString statTimeFormat = ui->edStatTimeFormat->text().trimmed();
	QString fileDateFormat = ui->edFileDateFormat->text().trimmed();
	QString fileTimeFormat = ui->edFileTimeFormat->text().trimmed();
	QString propDateFormat = ui->edPropDateFormat->text().trimmed();
	QString propTimeFormat = ui->edPropTimeFormat->text().trimmed();
	//--------------------------------------------------------------------------------------------------------------------------

	bool isIconModified = d.iconTheme != m_savedIconTheme || d.iconSize != m_savedIconSize;
	bool isMiscModified = terminal != d.terminal || rightSide != d.rightSide || expand2paneWinHeight != d.expand2paneWinHeight;
	bool isPropModefied = keyStopHover != d.keyStopHover || hoverTime != d.hoverTime || clickEnter != d.clickEnter ||
			statDateFormat != d.statDateFormat || fileDateFormat != d.fileDateFormat || propDateFormat != d.propDateFormat ||
			statTimeFormat != d.statTimeFormat || fileTimeFormat != d.fileTimeFormat || propTimeFormat != d.propTimeFormat;


	bool isModified = isIconModified || isMiscModified || isPropModefied;

	if (isModified)
	{
		// window
		d.expand2paneWinHeight = expand2paneWinHeight;
		d.rightSide = rightSide;
		//--------------------------------------------------------------------------------------------------------------------------

		// click & enter
		d.clickEnter = clickEnter;
		d.hoverTime = hoverTime;
		d.keyStopHover = keyStopHover;
		//--------------------------------------------------------------------------------------------------------------------------

		// icon theme
		//d.iconTheme = iconTheme;
		//d.iconSize  = iconSize;
		//--------------------------------------------------------------------------------------------------------------------------

		// program files
		d.terminal  = terminal;
		//--------------------------------------------------------------------------------------------------------------------------

		// date-time format
		d.statDateFormat = statDateFormat;
		d.statTimeFormat = statTimeFormat;

		d.fileDateFormat = fileDateFormat;
		d.fileTimeFormat = fileTimeFormat;

		d.propDateFormat = propDateFormat;
		d.propTimeFormat = propTimeFormat;
		//--------------------------------------------------------------------------------------------------------------------------

		d.saveSettings();

		if (isIconModified) {
		//	R::app()->updateIconTheme();
		}

		if (isPropModefied) {
			R::app()->updateSettings();
		}
	}
	//--------------------------------------------------------------------------------------------------------------------------

	hide();
	m_ready = false;
}

void SettingsDlg::reject()
{
	undoIconTheme();
	hide();
	m_ready = false;
}

void SettingsDlg::closeEvent(QCloseEvent *event)
{
	undoIconTheme();
	event->accept();
	m_ready = false;
}

void SettingsDlg::undoIconTheme()
{
	AppData &d = R::data();
//	qDebug() << "SettingsDlg::undoIconTheme" << d.iconTheme << m_savedIconTheme;
	if (d.iconTheme != m_savedIconTheme || d.iconSize != m_savedIconSize) {
		d.iconTheme = m_savedIconTheme;
		d.iconSize  = m_savedIconSize;
		R::app()->updateIconTheme();
	}
}
//==============================================================================================================================

void SettingsDlg::updateIconTheme()
{
	ui->navList->item(0)->setIcon(R::icon("view-split-left-right", 22));
	ui->navList->item(1)->setIcon(R::icon("mouse", 22));
	ui->navList->item(2)->setIcon(R::icon("preferences-desktop-theme", 22));
	ui->navList->item(3)->setIcon(R::icon("utilities-terminal", 22));
	ui->navList->item(4)->setIcon(R::defaultIcon("clock", 22));

	ui->buttonBox->update();
}

void SettingsDlg::onIconThemeChanged()
{
	updateIconTheme();
}
//==============================================================================================================================

void SettingsDlg::onPageChanged(int index)
{
	ui->stack->setCurrentIndex(index);
}

void SettingsDlg::onIconSizeChanged(bool)
{
	if (!m_ready) return;

	R::data().iconSize = ui->rbIcon16->isChecked() ? 16 : 22;
	R::app()->updateIconTheme();
}

void SettingsDlg::onCurrentItemChanged(QListWidgetItem *curr, QListWidgetItem *)
{
	if (!m_ready) return;

	if (curr) {
		QList<int> types;
		QStringList paths;
		QString theme = curr->text();
		if (R::getValidIconTheme(types, paths, theme)) {
			R::data().iconTheme = curr->text();
			R::app()->updateIconTheme();
		}
	}
}

void SettingsDlg::initIconThemes()
{
	QDirIterator it("/usr/share/icons", QDir::Dirs | QDir::NoDotAndDotDot);
	ui->iconThemeList->clear();

	while (it.hasNext()) {
		it.next();
		QListWidgetItem *p = new QListWidgetItem(it.fileName());
		p->setSizeHint(QSize(18, 18));
		ui->iconThemeList->addItem(p);
		if (it.fileName() == m_savedIconTheme)
			ui->iconThemeList->setCurrentItem(p);
	}

	ui->iconThemeList->sortItems();
}

QString SettingsDlg::getSelectedIconTheme()
{
	QListWidgetItem *p = ui->iconThemeList->currentItem();
	return p ? p->text() : QString();
}
