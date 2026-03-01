#include "add_controller.h"

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
#include <QGridLayout>
#include <QPushButton>
#include <QTextStream>

//===================================================================================================================================================
//	Подключение модулей проекта
//===================================================================================================================================================
#include <data_model/dc_data_manager.h>
#include "utils/cfg_path.h"
#include "config_deprotec.h"


//===================================================================================================================================================
//	Конструктор и деструктор класса
//===================================================================================================================================================
AddController::AddController(int32_t controller_index, QWidget* parent)
    : QDialog(parent)
    , _controllers_model{ new QStringListModel(this) }
    , _controllers_list{ new QListView(this) }
    , _controller{controller_index}
    , _is_C1{false}
    , _is_AC{false}
    , _is_deprotec{false}
{
    //	Создание формы
    this->setWindowTitle(QString("Выбор устройства"));
    this->setMinimumSize(QSize(400, 300));
    this->setMaximumSize(QSize(400, 300));
    this->setWindowIcon(QIcon(":/images/16/bind.png"));
    this->setModal(false);
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

	auto select_button = new QPushButton(this);
	select_button->setText(QString("Выбор"));
	select_button->setMinimumSize(QSize(75, 25));
	select_button->setMaximumSize(QSize(75, 25));
	select_button->setDefault(true);    
	auto cancel_button = new QPushButton(this);
	cancel_button->setText(QString("Отмена"));
	cancel_button->setMinimumSize(QSize(75, 25));
	cancel_button->setMaximumSize(QSize(75, 25));

    auto main_layout = new QGridLayout(this);
	main_layout->addWidget(_controllers_list, 0, 0, 1, 3);
	main_layout->addWidget(select_button, 1, 1);
    main_layout->addWidget(cancel_button, 1, 2);

	//	Заполнение списка контроллеров
    _controllers_model->setStringList(ControllersList());
    _controllers_list->setModel(_controllers_model);

	//	Обработка сигналов
	connect(cancel_button, &QPushButton::clicked, this, &AddController::reject);
	connect(select_button, &QPushButton::clicked, this, &AddController::SelectPressed);
	connect(_controllers_list, &QListView::doubleClicked, this, &AddController::SelectPressed);
}

AddController::~AddController()
{
}

//===================================================================================================================================================
//	Вспомогательные методы класса
//===================================================================================================================================================
QStringList AddController::ControllersList() const
{
    QStringList names_list;

	//	Проверка списка контроллеров
    QDir model_directory(cfg::path::templates());
	if (!model_directory.exists()) {
		Dpc::Gui::MsgBox::error(QString("Не найдена библиотека шаблонов."));
        return names_list; }

	QFileInfoList controllers_list = model_directory.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot);
	if (controllers_list.isEmpty()) {
		Dpc::Gui::MsgBox::error(QString("Библиотека шаблонов пуста."));
		return names_list; }

	//	Заполнение списка контроллеров
	for (int i = 0; i < controllers_list.size(); ++i) {
		QFileInfo fileInfo = controllers_list.at(i);
		if (!fileInfo.isDir()) continue;
		QString file_name = fileInfo.fileName();
		if (file_name.contains("DEPROTEC-LT")) file_name = "DEPROTEC";
		if (!names_list.contains(file_name)) names_list.append(file_name); }

	return names_list;
}

bool AddController::CreateController(const QString& config_path)
{
	//	Проверка имени контроллера и конфигурации
	if (_controller_name.isEmpty()) {
		Dpc::Gui::MsgBox::warning(QString("Не задано имя контроллера."));
		return false; }
	if (config_path.isEmpty()) {
		Dpc::Gui::MsgBox::warning(QString("Не определен шаблон конфигурации."));
		return false; }

	//	Удаление старого шаблона
	QString controller_path = cfg::path::controllerDir(_controller);
	QDir controller_dir(controller_path);
	if (controller_dir.exists()) {
        if (!cfg::path::remove(controller_path)) {
			Dpc::Gui::MsgBox::error(QString("Ошибка удаления старой папки в проекте! Проверьте, все ли файлы закрыты"));
			return false; }
	}

	//	Создание нового шаблона
    QDir().mkdir(controller_path);
    QString source = config_path + cfg::path::templateControllerFile();
    QString	target = controller_path + cfg::path::controllerFile();
	if (!QFile::copy(source, target)) {
		Dpc::Gui::MsgBox::error(QString("Ошибка создания устройства. Сбой в библиотеке шаблонов"));
		return false; }
		
	return true;
}

QMap<QString, QString> AddController::DeprotecMap() const
{
	//	Проверка списка конфигураций
    QFile list_file(cfg::path::getTemplateConfigurations());
	if (!list_file.exists()) {
		Dpc::Gui::MsgBox::error("Не найден список конфигураций.");
		return QMap<QString, QString>(); }
	if (list_file.isOpen()) {
		Dpc::Gui::MsgBox::error("Список конфигураций используется другой программой.");
		return QMap<QString, QString>(); }
	if (!list_file.open(QIODevice::ReadOnly)) {
		Dpc::Gui::MsgBox::error("Ошибка чтение списка конфигураций.");
		return QMap<QString, QString>(); }

	//	Чтение конфигураций из файла
	QMap<QString, QString> map;
	QTextStream input(&list_file);
	QString config_string = input.readAll();
	QStringList config_list = config_string.split("\n");
	for (int i = 0; i < config_list.count(); i++) {
		if (config_list.at(i).isEmpty()) continue;
		QStringList map_list = config_list.at(i).split(";");
		map[map_list.at(0).trimmed()] = map_list.at(1).trimmed(); }

	return map;
}


QString AddController::GetDeprotec(const QMap<QString, QString> &deprotec_map)
{
	//	Выбор модификации DeProtec
	ConfigDeprotec deprotec(deprotec_map);
	deprotec.exec();
	if (deprotec.result() != QDialog::Accepted) return QString();

	//	Параметры DeProtec
	_is_C1 = deprotec.IsC1();
	_controller_name = deprotec.DeprotecName();
	_is_deprotec = true;

	return deprotec.Path();
}


//===================================================================================================================================================
//	Слоты
//===================================================================================================================================================
void AddController::SelectPressed()
{
	//	Проверка выбранного устройства
    auto selectedIndex = _controllers_list->selectionModel()->selectedRows().value(0);
    if (!selectedIndex.isValid()) {
		Dpc::Gui::MsgBox::error(QString("Не выбрано значение из списка"));
        return;	}

    QString value = selectedIndex.data().toString();
	if (value.isEmpty()) {
        Dpc::Gui::MsgBox::error(QString("Выбрано пустое значение"));
		return;	}

	//	Определение типа контроллера
	_controller_name = value;
    QString config_path = cfg::path::templates() + _controller_name;
	_is_deprotec = false;

	//	Выбор типа Deprotec
	if (value.contains("DEPROTEC")) {
        this->hide();
		QMap<QString, QString> deprotec_map = DeprotecMap();
        if (deprotec_map.isEmpty())
            return;

		config_path = GetDeprotec(deprotec_map);
        if (config_path.isEmpty())
            return;

        config_path = cfg::path::templates() + config_path; }

	//	Создание контроллера
	CreateController(config_path) ? this->accept() : this->reject();

	return;
}
