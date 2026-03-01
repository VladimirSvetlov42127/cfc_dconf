#include "dc_project_description.h"
#include "utils/cfg_path.h"

#include "dpc/iconvlite.h"

#include <qfile.h>
#include <qtextstream.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>

#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#else
#include <qtextcodec.h>
#endif

DcProjectDescription::DcProjectDescription() :
	m_name(""),
	m_object(""),
	m_author(""),
	m_description(""),
	m_timeCreate(""),
	m_timeUpdate(""),
	m_version(0),
	m_isNeedUpdate(false)
{

}

DcProjectDescription::DcProjectDescription(const QString & name, const QString & object, const QString & author,
	const QString & desc, const QString & timeCreate, const QString & timeUpdate) :
	m_name(name), 
	m_object(object), 
	m_author(author), 
	m_description(desc),
	m_timeCreate(timeCreate), 
	m_timeUpdate(timeUpdate),
	m_version(0),
	m_isNeedUpdate(false)
{

}

DcProjectDescription::~DcProjectDescription(){

}

void DcProjectDescription::clear(void)
{
	m_name = "";
	m_object = "";
	m_author = "";
	m_description = "";
	m_timeCreate = "";
	m_timeUpdate = "";
	m_isNeedUpdate = false;
}

bool DcProjectDescription::init(const QString& path){
	if (path.isEmpty())
		return false;

	QString content;
	QFile infofile(path);
	if (infofile.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QByteArray dataJson =  infofile.readAll();

		if (conv().isUtf8(dataJson))
		{
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
			auto toUtf8 = QStringDecoder(QStringDecoder::Utf8);
			content = toUtf8(dataJson);
#else
			auto toUtf8 = QTextCodec::codecForName("UTF-8");
			content = toUtf8->fromUnicode(dataJson);
#endif
		}
		else
		{
			content = conv().toUtf8(dataJson);//  QString::fromStdString(cp2utf(dataJson.data()));
		}
	}
	else
		return false;

	return fromJson(content);
}

bool DcProjectDescription::save(const QString& path) {
    // if (!isNeedUpdate())
    // 	return true;

	if (path.isEmpty()) {
		return false;
	}

	m_timeUpdate = QDateTime::currentDateTime().toString("(yyyy.MM.dd) hh:mm:ss");

	QString json = toJSon();

	QFile infofile(path);
	if (infofile.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QTextStream out(&infofile);
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
		out.setEncoding(QStringConverter::Utf8);
#else
		out.setCodec(QTextCodec::codecForName("UTF-8"));
#endif
		out << json;
		infofile.close();
		m_isNeedUpdate = false;

		//TODO error to log and screen
		return true;
	}
	m_isNeedUpdate = false;
	return false;
}

void DcProjectDescription::update() {
	m_isNeedUpdate = true;
}

// bool DcProjectDescription::isNeedUpdate() {
// 	return m_isNeedUpdate;
// }

void DcProjectDescription::setName(const QString &name) {
	m_name = name;
	update();
}

void DcProjectDescription::setObject(const QString &obj) {
	m_object = obj;
	update();
}

void DcProjectDescription::setAuthor(const QString &author) {
	m_author = author;
	update();
}

void DcProjectDescription::setDesc(const QString &desc) {
	m_description = desc;
	update();
}

void DcProjectDescription::setCreateTime(const QString &timeCreate) {
	m_timeCreate = timeCreate;
	update();
}

void DcProjectDescription::setUpdateTime(const QString &timeUpdate) {
	m_timeUpdate = timeUpdate;
}

void DcProjectDescription::setVersion(uint32_t ver) {
	m_version = ver;
	update();
}

QString DcProjectDescription::name() {
	return m_name;
}

QString DcProjectDescription::object() {
	return m_object;
}

QString DcProjectDescription::author() {
	return m_author;
}

QString DcProjectDescription::desc() {
	return m_description;
}

QString DcProjectDescription::timeCreate() {
	return m_timeCreate;
}

QString DcProjectDescription::timeUpdate() {
	return m_timeUpdate;
}

bool DcProjectDescription::fromJson(QString &jsonstr) {
	if (jsonstr.isEmpty())
		return true;

	auto json_doc = QJsonDocument::fromJson(jsonstr.toUtf8());

	QJsonObject obj = json_doc.object();

	QJsonValue value1 = obj.value(QString("name"));
	if (!value1.isNull())
		m_name = value1.toString();
	QJsonValue value2 = obj.value(QString("object"));
	if (!value2.isNull())
		m_object = value2.toString();
	QJsonValue value3 = obj.value(QString("author"));
	if (!value3.isNull())
		m_author = value3.toString();
	QJsonValue value4 = obj.value(QString("desc"));
	if (!value4.isNull())
		m_description = value4.toString();
	QJsonValue value5 = obj.value(QString("create_time"));
	if (!value5.isNull())
		m_timeCreate = value5.toString();
	QJsonValue value6 = obj.value(QString("update_time"));
	if (!value6.isNull())
		m_timeUpdate = value6.toString();
	m_version = obj.value("version").toString().toInt();

	return true;
}

QString DcProjectDescription::toJSon() {
	QString restmp = "{\"name\":\"%1\",\"object\":\"%2\",\"author\":\"%3\",\"desc\":\"%4\",\"create_time\":\"%5\",\"update_time\":\"%6\", \"version\":\"%7\"}";
	QString res = restmp.arg(m_name, m_object, m_author, m_description, m_timeCreate, m_timeUpdate, QString::number(m_version));
	return res;
}
