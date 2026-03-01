#include "filesystem_page.h"

//===================================================================================================================================================
//	Подключение стандартных библиотек
//===================================================================================================================================================

//===================================================================================================================================================
//	Подключение сторонних библиотек
//===================================================================================================================================================
#include <dpc/gui/dialogs/msg_box/MsgBox.h>

//===================================================================================================================================================
//	Подключение библиотек QT
//===================================================================================================================================================
#include <QStringList>
#include <QListWidgetItem>
#include <QApplication>
#include <QInputDialog>
#include <QFileDialog>
#include "qboxlayout.h"
#include "qlabel.h"

//===================================================================================================================================================
//	Подключение модулей проекта
//===================================================================================================================================================
//#include "gui_utilities.h"
#include "dpc/return_codes.h"
#include <journal/JSource.h>

#ifdef _THRADED_CHANNEL
using namespace Dc::SyBUS;
#else
using namespace Dpc::Sybus;
#endif // _THRADED_CHANNEL

//using namespace Dep;

//===================================================================================================================================================
//	Конструктор и деструктор класса
//===================================================================================================================================================
FilesystemPage::FilesystemPage(DcController* controller, QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Файловая система");
    m_connector = controller->channel();
	//	Свойства класса
	_path_edit = new QLineEdit(this);
	_path_edit->setReadOnly(true);
	_list_widget = new QListWidget(this);
	_current_path = QString();


    // QHBoxLayout* _info_layout = new QHBoxLayout;
    // _info_layout->setContentsMargins(6, 0, 6, 0);
    QHBoxLayout*_control_layout = new QHBoxLayout;
    QGridLayout* _main_layout = new QGridLayout;
    _main_layout->setContentsMargins(0, 6, 0, 0);

	//	Формирование кнопок
	QPushButton* new_folder_button = new QPushButton(QIcon(":/icons/32/folder_add.png"), "Создать папку", this);
	QPushButton* write_button = new QPushButton(QIcon(":/icons/32/import.png"), "Записать", this);
	QPushButton* download_button = new QPushButton(QIcon(":/icons/32/export.png"), "Скачать", this);
	QPushButton* delete_button = new QPushButton(QIcon(":/icons/32/delete.png"), "Удалить", this);
	_format_button = new QPushButton(QIcon(":/icons/32/formatting.png"), "Форматировать", this);
	_format_button->setEnabled(MinimalMode());

	_progress = new QProgressBar(this);

	connect(new_folder_button, &QPushButton::clicked, this, &FilesystemPage::NewCliked);
	connect(write_button, &QPushButton::clicked, this, &FilesystemPage::WriteClicked);
	connect(download_button, &QPushButton::clicked, this, &FilesystemPage::DownloadClicked);
	connect(delete_button, &QPushButton::clicked, this, &FilesystemPage::DeleteClicked);
	connect(_format_button, &QPushButton::clicked, this, &FilesystemPage::FormatClicked);

    _control_layout->addWidget(new_folder_button);
    _control_layout->addWidget(write_button);
    _control_layout->addWidget(download_button);
    _control_layout->addWidget(delete_button);
    _control_layout->addStretch();
    _control_layout->addWidget(_format_button);

	//	Добавление информации о носителе
	int error = GetFlashSize();
	int row = 0;
    _main_layout->setRowMinimumHeight(row, 10);
	if (error == RETURN_DATA) {
		row++;
		for (int i = 0; i < 4; i++) { 
			row++;
            _main_layout->addWidget(new QLabel(_flash_names[i]), row, 0);
            _main_layout->addWidget(new QLabel(QString::number(_flash_size[i], 'f', 0)), row, 1); }
	}

	row++;
    _main_layout->setRowMinimumHeight(row, 10);
	row++;

	//	Основная часть формы
	DirWidget()->setSelectionMode(QAbstractItemView::MultiSelection);
    _main_layout->addWidget(_path_edit, row++, 0, 1, 4);
    _main_layout->addWidget(_list_widget, row++, 0, 1, 4);
    _main_layout->addWidget(_progress, row++, 0, 1, 4);

	connect(DirWidget(), &QListWidget::itemDoubleClicked, this, &FilesystemPage::DirItemClicked);
    connect(Connector().get(), &Channel::progress, this, &FilesystemPage::setProgress);

	//	Загрузка списка файлов
	error = LoadFileList(_current_path);

	setProgress(-1);

    QGridLayout* widget_layout = new QGridLayout(this);
    // widget_layout->addLayout(_title_layout, row++, 0);
    // widget_layout->addLayout(_Info_Layout(), row++, 0);
    widget_layout->addLayout(_control_layout, row++, 0);
    // if (withScrollArea)
    //     widget_layout->addWidget(m_scrollArea, row++, 0);
    // else
        widget_layout->addLayout(_main_layout, row++, 0);
    widget_layout->setRowStretch(widget_layout->rowCount() - 1, 10);
	return;
}

FilesystemPage::~FilesystemPage()
{
    if (Connector()->connect(true)) {
        Connector()->fileSystemQuit();
        Connector()->disconnect();
    }

	disconnect(Connector().get(), &Channel::progress, this, &FilesystemPage::setProgress);


	_path_edit->deleteLater();
	_list_widget->deleteLater();
	_format_button->deleteLater();
	_progress->deleteLater();
}


//===================================================================================================================================================
//	Перегружаемые методы класса
//===================================================================================================================================================
void FilesystemPage::RunActivate()
{
	//if (Connector())  _download_button->setEnabled(true);
	FormatButton()->setEnabled(MinimalMode());
	UpdateFlashSize();
    //SetState(ReadyState);

	return;
}

void FilesystemPage::RunDeactivate()
{
}


//===================================================================================================================================================
//	Методы обработки сигналов формы
//===================================================================================================================================================
void FilesystemPage::NewCliked()
{
	if (CurrentPath().isEmpty()) return;

	QString text = QInputDialog::getText(this, tr("Добавление каталога"), tr("Имя каталога:"), QLineEdit::Normal, "NewFolder");
	if (text.isEmpty())	return;

	QString new_dir = CurrentPath();
	if (new_dir.back() != '/') new_dir.append('/');
	new_dir.append(text);

	//	Авторизация в системе
    if (!Connector()->connect(true)) {
        //gJournal.addErrorMessage("Не удалось подключиться к устройству.");
        //return;
    }
    //SetState(RunningState, "Создание новой папки...");
	Dpc::Sybus::FileSystemLoginScope fs(Connector());
	if (!fs.isValid) {
        //SetState(ErrorState, Connector()->getErrorMsg());
        Connector()->disconnect();
		return;	}
	if (!Connector()->mkdirectory(new_dir)) {
        //SetState(ErrorState, "Ошибка создание нового каталога.");
        Connector()->disconnect();
		return;	}
    Connector()->disconnect();
	LoadFileList(CurrentPath());

	UpdateFlashSize();
    //SetState(ReadyState);

	return;
}

void FilesystemPage::DownloadClicked()
{
	if (CurrentPath().isEmpty()) return;
	quint16 count = DirWidget()->selectedItems().count();
	if (count <= 0) return;

	//	Выбор каталога для выгрузки
	QString dir = QFileDialog::getExistingDirectory(this, "Выбрать папку", QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (dir.isEmpty()) return;

	//	Авторизация в системе
    if (!Connector()->connect(true)) {
        //SetState(ErrorState, Connector()->getErrorMsg());
        //gJournal.addErrorMessage("Не удалось подключиться к устройству.");
        //return;
	}

    //SetState(RunningState, "Загрузка файлов...");
	qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
	Dpc::Sybus::FileSystemLoginScope fs(Connector());
	if (!fs.isValid) {
		qApp->setOverrideCursor(QCursor(Qt::ArrowCursor));
        //SetState(ErrorState, Connector()->getErrorMsg());
        Connector()->disconnect();
		return; }

    //SetState(RunningState, "Выгрузка содержимого каталога ...");
    //qApp->setOverrideCursor(QCursor(Qt::WaitCursor));

	//	Скачивание файла
	QString path = PathEdit()->text();
	for (int i = 0; i < count; i++) {
		QListWidgetItem* item = DirWidget()->selectedItems().at(i);
		QString name = item->text();
		QString file_name = name;
		int position = file_name.lastIndexOf("/") + 1;
		file_name.remove(0, position);
		NodeType type = item->data(Qt::UserRole).value<NodeType>();
		if (type != NODE_FILE) continue;
		QString source = path /*+ "/"*/ + name;
		QString target = dir + "/" + file_name;
		bool res = Connector()->downloadFile(source, target); 
	}

	qApp->setOverrideCursor(QCursor(Qt::ArrowCursor));
    Connector()->disconnect();
	setProgress(-1);
    //SetState(ReadyState);

	return;
}

void FilesystemPage::WriteClicked()
{
	if (CurrentPath().isEmpty()) return;
	QStringList save_list = QFileDialog::getOpenFileNames(this, "Выбрать файлы", "");
	if (save_list.isEmpty()) return;

	//	Авторизация в системе
    if (!Connector()->connect(true)) {
        //gJournal.addErrorMessage("Не удалось подключиться к устройству.");
        //return;
    }
    //SetState(RunningState, "Загрузка файлов...");
	qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
	Dpc::Sybus::FileSystemLoginScope fs(Connector());
	if (!fs.isValid) {
		qApp->setOverrideCursor(QCursor(Qt::ArrowCursor));
        //SetState(ErrorState, Connector()->getErrorMsg());
        Connector()->disconnect();
        return;
    }

	int count = save_list.count();
	for (int i = 0; i < count; i++) {
		QString file_name = save_list.at(i);
		int position = file_name.lastIndexOf("/") + 1;
		file_name.remove(0, position);
		QString target_file = CurrentPath() + file_name;
		Connector()->uploadFile(save_list.at(i), target_file); }
	qApp->setOverrideCursor(QCursor(Qt::ArrowCursor));

	//	Обновление данных
    Connector()->disconnect();
	LoadFileList(CurrentPath());
	UpdateFlashSize();
    //SetState(ReadyState);
	setProgress(-1);

	return;
}

void FilesystemPage::FormatClicked()
{
	//	Выбор тома для форматирования
	bool ok;
	int volume = QInputDialog::getInt(this, "Выбор номера тома", "Номер тома для форматирования:", 0, 0, 1, 1, &ok);
	if (!ok) return;
	if (!Dpc::Gui::MsgBox::question(QString("Вы уверены что хотите продолжить? Все данные на томе %1 будут утеряны безвозвратно").arg(volume))) return;

	//	Запуск форматирования	
    if (!Connector()->connect(true)) {
        //gJournal.addErrorMessage("Не удалось подключиться к устройству.");
        //return;
	}
    //SetState(RunningState, QString("Форматирование тома %1 ...").arg(volume));
	qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
	if (!Connector()->formatDrive(volume)) {
        //SetState(ErrorState, Connector()->getErrorMsg());
		qApp->setOverrideCursor(QCursor(Qt::ArrowCursor));
        return;
    }
	qApp->setOverrideCursor(QCursor(Qt::ArrowCursor));

    Connector()->disconnect();
	LoadFileList(CurrentPath());
	UpdateFlashSize();
    //SetState(ReadyState);
	setProgress(-1);

	return;
}

void FilesystemPage::DeleteClicked()
{
	if (CurrentPath().isEmpty()) return;
	quint16 count = DirWidget()->selectedItems().count();
	if (count <= 0) return;

	//	Подтверждение удаления файлов
	if (!Dpc::Gui::MsgBox::question("Вы уверены что хотите удалить выделенные объекты?")) return;

	//	Авторизация в системе
    if (!Connector()->connect(true)) {
        //SetState(ErrorState, Connector()->getErrorMsg());
        //gJournal.addErrorMessage("Не удалось подключиться к устройству.");
        //return;
    }

    //SetState(RunningState, "Удаление каталога ...");
	qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
	Dpc::Sybus::FileSystemLoginScope fs(Connector());
	if (!fs.isValid) {
        qApp->setOverrideCursor(QCursor(Qt::ArrowCursor));
        //SetState(ErrorState, Connector()->getErrorMsg());
        Connector()->disconnect();
        return;
    }

	QString path = PathEdit()->text();
	for (int i = 0; i < count; i++) {
		QListWidgetItem* item = DirWidget()->selectedItems().at(i);
		QString name = item->text();
		if (name.contains("..")) continue;
		NodeType type = item->data(Qt::UserRole).value<NodeType>();
		QString file_name;
		if (type == NODE_FILE) file_name = path /*+ "/"*/ + name;
		if (type == NODE_DIR)  file_name = path /*+ "/"*/ + name /*+ "/"*/;
		QStringList file_list = Connector()->directoryFileList(file_name);
		if (!file_list.isEmpty())
		{
			qApp->setOverrideCursor(QCursor(Qt::ArrowCursor));
			Dpc::Gui::MsgBox::info(QString("Невозможно удалить папку \"%1\", содержащую файлы.").arg(name));
			qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
		}
		Connector()->deleteFile(file_name); 
	}
	qApp->setOverrideCursor(QCursor(Qt::ArrowCursor));
    Connector()->disconnect();

	LoadFileList(CurrentPath());
	UpdateFlashSize();
    //SetState(ReadyState);

	return;
}

void FilesystemPage::DirItemClicked(QListWidgetItem* item)
{
	if (item == nullptr) return;
	NodeType type = item->data(Qt::UserRole).value<NodeType>();
	if (type == NODE_FILE) return;

	QString dir;
	QString text = item->text();
	QString path = CurrentPath();

	if (type == NODE_DRIVE) {
		dir = QString("%1:/").arg(text);
	} else if (text == "..") {
		if (path.count() > 4) {
            if (path.back() == '/')
#if QT_VERSION < QT_VERSION_CHECK(6, 3, 0)
                path.remove(path.count()-1, 1);
#else
                path.removeLast();
#endif
			auto pos = path.lastIndexOf('/');
			dir = path.left(path.count('/') > 1 ? pos : pos + 1);}
	} else { dir = MakePath(text); }
	if (!dir.isEmpty() && dir.back() != '/')	dir.append('/');
	LoadFileList(dir);

	return;
}

void FilesystemPage::setProgress(int value)
{
	_progress->setVisible(value > -1);
	_progress->setValue(value);
}


//===================================================================================================================================================
//	Вспомогательные методы класса
//===================================================================================================================================================
bool FilesystemPage::MinimalMode()
{
	bool minimal = false;
    if (!Connector()->connect(true)) {
        //gJournal.addErrorMessage("Не удалось подключиться к устройству.");
        //return false;
    }
	quint16 index = 0x07;
    auto pack = Connector()->param(SP_SM_ENTITY_STATUS, index);
	if (pack) {
		quint8 data = pack->value<quint8>();
        //delete pack;
		if (data == 1)
			return minimal = true;
	}

    Connector()->disconnect();
	return minimal;
}

int FilesystemPage::GetFlashSize()
{
    if (!Connector()->connect(true)) {
        //gJournal.addErrorMessage("Не удалось подключиться к устройству.");
        //return RETURN_ERROR;
    }
    //SetState(RunningState, "Чтение информации о файловой системе.");

	auto name_pack = Connector()->names(SP_VOLUME_INFO);
    if (!name_pack)
	{
        //SetState(ErrorState, "Ошибка чтения информации о файловой системе.");
        Connector()->disconnect();
		return RETURN_ERROR; 
	}
    for (int i = 0; i < 4; i++) _flash_names[i] = name_pack->value<QString>(i + 1);

	quint16 index = 0;
    auto value_pack = Connector()->param(SP_VOLUME_INFO, index, 4);
	if (!value_pack) 
	{
        //SetState(ErrorState, "Ошибка чтения информации о файловой системе.");
        Connector()->disconnect();
		return RETURN_ERROR; 
	}
	for (int i = 0; i < 4; i++) 
		_flash_size[i] = value_pack->value<float>(i);

    //delete value_pack;
    Connector()->disconnect();
    //SetState(ReadyState);

	return RETURN_DATA;
}

void FilesystemPage::UpdateFlashSize()
{
	int error = GetFlashSize();
	if (error != RETURN_DATA) 
		return;
	for (int i = 0; i < 4; i++) {
        QLabel* widget = 0;
        if (layout()) {
            QLayoutItem* item = ((QGridLayout*)layout())->itemAtPosition(i + 2, 1);
            if (item)
                widget = (QLabel*)item->widget();
        }
		if (widget)
			widget->setText(QString::number(_flash_size[i], 'f', 0)); 
	}
	
	return;
}

QString FilesystemPage::MakePath(const QString& text)
{
	QString path = CurrentPath();
	if (path.back() != '/')	path.append('/');
	path.append(text);
	//if (path.back() != '/')	path.append('/');

	return path;
}


//===================================================================================================================================================
//	Основные методы класса
//===================================================================================================================================================
void FilesystemPage::AddItemList(const QString& name, NodeType type)
{
	QListWidgetItem* item = new QListWidgetItem(type == NODE_FILE ? QIcon(":/icons/32/edit_0.png") : QIcon(":/icons/32/folder.png"), name, DirWidget());
	item->setData(Qt::UserRole, QVariant::fromValue<NodeType>(type));
	DirWidget()->addItem(item);

	return;
}

int FilesystemPage::LoadFileList(const QString& dir)
{
	//	Авторизация в системе
    if (!Connector()->connect(true)) {
        //gJournal.addErrorMessage("Не удалось подключиться к устройству.");
        //return RETURN_ERROR;
    }

    //SetState(RunningState, "Чтение списка файлов.");
	qApp->setOverrideCursor(QCursor(Qt::WaitCursor));

	Dpc::Sybus::FileSystemLoginScope fs(Connector());
	if (!fs.isValid) {
        qApp->setOverrideCursor(QCursor(Qt::ArrowCursor));
        //SetState(ErrorState, Connector()->getErrorMsg());
        Connector()->disconnect();
        return RETURN_ERROR;
    }

	//	Заполнение списка файлов
	DirWidget()->clear();
	QStringList file_list = Connector()->directoryFileList(dir);
	file_list.sort();

	QStringList drives;
	QStringList files;
	QStringList dirs;

	if (dir.count('/')) dirs << "..";
	for (auto& item : file_list) {
		if (item.endsWith(":/")) { drives << item.chopped(2); }
		else if (item.endsWith('/')) {
            dirs << item.chopped(1);
        } else {
            files << item;
        }
	}

    for (auto& item : drives)
        AddItemList(item, NODE_DRIVE);
    for (auto& item : dirs)
        AddItemList(item, NODE_DIR);
    for (auto& item : files)
        AddItemList(item, NODE_FILE);

	SetCurrentPath(dir);
	qApp->setOverrideCursor(QCursor(Qt::ArrowCursor));
    //SetState(ReadyState);
    Connector()->disconnect();

	return RETURN_DATA;
}

Dpc::Sybus::ChannelPtr FilesystemPage::Connector()
{
    return m_connector;
}
