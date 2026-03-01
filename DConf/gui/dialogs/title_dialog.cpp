#include "title_dialog.h"

//===================================================================================================================================================
//	Подключение библиотек QT
//===================================================================================================================================================
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>


//===================================================================================================================================================
//	Конструктор и деструктор класса
//===================================================================================================================================================
TitleDialog::TitleDialog(QString name, QWidget* parent) : QDialog(parent)
{
	//	Свойства класса
	_name = name;

	//	Формирование вида окна диалога
	this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
	this->setWindowTitle(QString("Название алгоритма"));

	//	Создание полей редактирования
	_name_edit = new QLineEdit(this);
    _name_edit->setMinimumWidth(250);
    _name_edit->setMaximumWidth(250);
    _name_edit->setText(_name);
	QHBoxLayout* edit_layout = new QHBoxLayout();
    edit_layout->addWidget(_name_edit);

	//	Создание кнопок
	QPushButton* ok_button = new QPushButton(QString("OK"));
	ok_button->setDefault(true);
	ok_button->setMinimumSize(80, 24);
	QPushButton* exit_button = new QPushButton(QString("Отмена"));
	exit_button->setMinimumSize(80, 24);

	QHBoxLayout* button_layout = new QHBoxLayout();
	button_layout->addStretch();
	button_layout->addWidget(exit_button);
	button_layout->addWidget(ok_button);

	//	Вывод основной формы
	QVBoxLayout* main_layout = new QVBoxLayout(this);
	main_layout->addLayout(edit_layout);
	main_layout->addStretch();
	main_layout->addLayout(button_layout);
    this->setMinimumSize(270, 100);
    this->setMaximumSize(270, 100);

	//	Обработка сигналов
    connect(exit_button, &QPushButton::clicked, this, &TitleDialog::reject);
    connect(ok_button, &QPushButton::clicked, this, &TitleDialog::okClicked);
}

TitleDialog::~TitleDialog()
{
}


//===================================================================================================================================================
//	Обработка сигналов формы
//===================================================================================================================================================
void TitleDialog::okClicked()
{
	_name = _name_edit->text();
	this->accept();
}
